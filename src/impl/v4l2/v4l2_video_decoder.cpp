// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v4l2_video_decoder.h"

#include <iostream>
#include <map>
#include <memory>

#include "base/fourcc.h"
#include "base/log.h"
#include "base/video_decoder_client.h"
#include "v4l2/v4l2_device.h"
#include "v4l2/v4l2_queue.h"

namespace mcil {

#if !defined(PLATFORM_EXTENSION)
// static
scoped_refptr<VideoDecoder> V4L2VideoDecoder::Create() {
  return new V4L2VideoDecoder();
}

// static
SupportedProfiles V4L2VideoDecoder::GetSupportedProfiles() {
  scoped_refptr<V4L2Device> device = V4L2Device::Create(V4L2_DECODER);

  if (!device)
    return SupportedProfiles();

  return device->GetSupportedDecodeProfiles();
}
#endif

V4L2VideoDecoder::V4L2VideoDecoder()
 : VideoDecoder(),
   device_(V4L2Device::Create(V4L2_DECODER)),
   output_mode_(OUTPUT_ALLOCATE),
   device_poll_thread_("V4L2DecoderDevicePollThread"),
   decoder_state_(kUninitialized) {
}

V4L2VideoDecoder::~V4L2VideoDecoder() {
}

bool V4L2VideoDecoder::Initialize(const DecoderConfig* config,
                                  VideoDecoderClient* client,
                                  DecoderClientConfig* client_config,
                                  int32_t vdec_port_index) {
  MCIL_DEBUG_PRINT(": resource index received: %d", vdec_port_index);

  client_ = client;
  if (client_ == nullptr) {
    NOTIFY_ERROR(INVALID_ARGUMENT);
    MCIL_ERROR_PRINT(" Delegate not provided");
    return false;
  }

  if (vdec_port_index < 0) {
    MCIL_ERROR_PRINT(": Resource not aquired: %d", vdec_port_index);
    return false;
  }

  if (!CheckConfig(config))
    return false;

  if (!SubscribeEvents())
    return false;

  if (!AllocateInputBuffers()) {
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }

  decoder_cmd_supported_ = IsDecoderCmdSupported();

  if (client_config != nullptr) {
    if (egl_image_format_fourcc_) {
      client_config->output_pixel_format =
          egl_image_format_fourcc_->ToVideoPixelFormat();
    }
    // Controlling the buffer feed is needed since we dont have
    // feed and release based control because decode and capture
    // devices are different in other platforms.
    client_config->should_control_buffer_feed = false;
  }

  return StartDevicePoll();
}

void V4L2VideoDecoder::Destroy() {
  StopDevicePoll();

  StopOutputStream();
  StopInputStream();

  current_input_buffer_.reset();

  DestroyInputBuffers();
  DestroyOutputBuffers();

  input_queue_ = nullptr;
  output_queue_ = nullptr;

  device_ = nullptr;

  start_time_ = ChronoTime();
}

bool V4L2VideoDecoder::ResetInputBuffer() {
  current_input_buffer_.reset();
  return true;
}

bool V4L2VideoDecoder::ResetDecodingBuffers(bool* reset_pending) {
  if (!(StopDevicePoll() && StopOutputStream()))
    return false;

  if (DequeueResolutionChangeEvent() != 0) {
    if (reset_pending != nullptr) {
      *reset_pending = true;
    }
    StartResolutionChange();
    return false;
  }

  if (!StopInputStream())
    return false;

  return true;
}

bool V4L2VideoDecoder::CanNotifyResetDone() {
  if (device_poll_thread_.IsRunning())
    return true;

  return StartDevicePoll();
}

bool V4L2VideoDecoder::DecodeBuffer(const void* buffer,
                                    size_t buffer_size,
                                    const int32_t buffer_id,
                                    int64_t buffer_pts) {
  MCIL_DEBUG_PRINT(": buffer[%p], size[%lu], id[%d], pts[%ld]",
                   buffer, buffer_size, buffer_id, buffer_pts);

    // Flush if we're too big
  if (current_input_buffer_) {
    size_t plane_size = current_input_buffer_->GetBufferSize(0);
    size_t bytes_used = current_input_buffer_->GetBytesUsed(0);
    if ((bytes_used + buffer_size) > plane_size) {
      if (!FlushInputBuffers()) {
        return false;
      }
    }
  }

  // Try to get an available input buffer.
  if (!current_input_buffer_) {
    if (input_queue_->FreeBuffersCount() == 0)
      DequeueBuffers();

    current_input_buffer_ = input_queue_->GetFreeBuffer();
    if (!current_input_buffer_) {
      MCIL_DEBUG_PRINT(": stalled for input buffers");
      return false;
    }

    struct timeval timestamp = { .tv_sec = buffer_id };
    current_input_buffer_->SetTimeStamp(timestamp);
    current_input_buffer_->SetBufferId(buffer_id);
  }

  if (buffer_size == 0) {
    MCIL_DEBUG_PRINT(": buffer_size is zero. buffer_id= %d", buffer_id);
    return true;
  }

  // Copy in to the buffer.
  size_t plane_size = current_input_buffer_->GetBufferSize(0);
  size_t bytes_used = current_input_buffer_->GetBytesUsed(0);

  if (buffer_size > (plane_size - bytes_used)) {
    MCIL_ERROR_PRINT(": over-size frame, erroring");
    NOTIFY_ERROR(UNREADABLE_INPUT);
    return false;
  }

  void* input_buffer = current_input_buffer_->GetPlaneBuffer(0);
  if (input_buffer == nullptr) {
    MCIL_ERROR_PRINT(": Error allocating input buffer");
    return false;
  }

  if (buffer_size > 0) {
    memcpy(reinterpret_cast<uint8_t*>(input_buffer) + bytes_used,
           buffer, buffer_size);
    current_input_buffer_->SetBytesUsed(0, bytes_used + buffer_size);
  }

  return true;
}

bool V4L2VideoDecoder::FlushInputBuffers() {
  if (!current_input_buffer_) {
    MCIL_DEBUG_PRINT(": current_input_buffer_: NULL");
    return true;
  }

  const int32_t input_id = current_input_buffer_->GetBufferId();
  if ((input_id >= 0) &&
      (current_input_buffer_->GetBytesUsed(0) == 0)) {
    current_input_buffer_.reset();
    return true;
  }

  // Queue it.
  MCIL_DEBUG_PRINT(": Queuing buffer input_id=%d", input_id);
  input_ready_queue_.push(std::move(*current_input_buffer_));
  current_input_buffer_.reset();

  EnqueueBuffers();

  return (decoder_state_ != kDecoderError);
}

bool V4L2VideoDecoder::DidFlushBuffersDone() {
  if (current_input_buffer_) {
    MCIL_DEBUG_PRINT(": Current input buffer != -1");
    return false;
  }

  if ((input_ready_queue_.size() + input_queue_->QueuedBuffersCount()) != 0) {
    MCIL_DEBUG_PRINT(": Some input buffers are not dequeued.");
    return false;
  }

  if (flush_awaiting_last_output_buffer_) {
    MCIL_DEBUG_PRINT(": Waiting for last output buffer.");
    return false;
  }

  if (!(StopDevicePoll() && StopOutputStream() && StopInputStream()))
    return false;

  return StartDevicePoll();
}

void V4L2VideoDecoder::EnqueueBuffers() {
  if (client_->IsDestroyPending() || (decoder_state_ == kChangingResolution)) {
    MCIL_DEBUG_PRINT(": state[%d]", static_cast<int32_t>(decoder_state_));
    return;
  }

  const int32_t old_inputs_queued =
      static_cast<int32_t>(input_queue_->QueuedBuffersCount());
  while (!input_ready_queue_.empty()) {
    bool flush_handled = false;
    int32_t input_id = input_ready_queue_.front().GetBufferId();
    if (input_id == kFlushBufferId) {
      if (input_queue_->QueuedBuffersCount() > 0)
        break;

      if (coded_size_.IsEmpty() || (input_queue_->IsStreaming() == false)) {
        MCIL_DEBUG_PRINT(": Nothing to flush. Notify flush done directly");
        client_->NotifyFlushDone();
        flush_handled = true;
      } else if (decoder_cmd_supported_) {
        if (!SendDecoderCmdStop())
          return;
        flush_handled = true;
      }
    }
    if (flush_handled) {
      input_ready_queue_.pop();
    } else {
      auto buffer = std::move(input_ready_queue_.front());
      input_ready_queue_.pop();
      if (!EnqueueInputBuffer(std::move(buffer)))
        return;
    }
  }

  if ((old_inputs_queued == 0) && (input_queue_->QueuedBuffersCount() != 0)) {
    if (!device_->SetDevicePollInterrupt()) {
      MCIL_ERROR_PRINT(": SetDevicePollInterrupt failed");
      NOTIFY_ERROR(PLATFORM_FAILURE);
      return;
    }

    if (!input_queue_->StreamOn()) {
      MCIL_ERROR_PRINT(": Failed Stream on input queue");
      NOTIFY_ERROR(PLATFORM_FAILURE);
      return;
    }
  }

  if (!input_queue_->IsStreaming())
    return;

  const int32_t old_outputs_queued =
      static_cast<int32_t>(output_queue_->QueuedBuffersCount());

  client_->CheckGLFences();
  while (auto buffer_opt = output_queue_->GetFreeBuffer()) {
    if (!EnqueueOutputBuffer(std::move(*buffer_opt)))
      return;
  }

  if ((old_outputs_queued == 0) && (output_queue_->QueuedBuffersCount() != 0)) {
    if (!device_->SetDevicePollInterrupt()) {
      MCIL_ERROR_PRINT(": SetDevicePollInterrupt failed");
      NOTIFY_ERROR(PLATFORM_FAILURE);
      return;
    }

    if (!output_queue_->StreamOn()) {
      MCIL_ERROR_PRINT(": Failed Stream on output queue");
      NOTIFY_ERROR(PLATFORM_FAILURE);
      return;
    }
  }
}

void V4L2VideoDecoder::DequeueBuffers() {
  while (input_queue_->QueuedBuffersCount() > 0) {
    if (!DequeueInputBuffer())
      break;
  }

  while (output_queue_->QueuedBuffersCount() > 0) {
    if (!DequeueOutputBuffer())
      break;
  }

  client_->NotifyFlushDoneIfNeeded();
}

void V4L2VideoDecoder::ReusePictureBuffer(int32_t pic_buffer_id) {
  MCIL_DEBUG_PRINT(": pic_buffer_id: %d", pic_buffer_id);
}

void V4L2VideoDecoder::RunDecodeBufferTask(bool event_pending, bool) {
  MCIL_DEBUG_PRINT(": event_pending[%d]", event_pending);

  bool resolution_change_pending = false;
  if (event_pending) {
    resolution_change_pending =
        static_cast<bool>(DequeueResolutionChangeEvent());
  }

  if ((resolution_change_pending == false) && coded_size_.IsEmpty()) {
    bool again;
    struct v4l2_format format;
    Size visible_size;
    if (GetFormatInfo(&format, &visible_size, &again) && (again == false)) {
      resolution_change_pending = true;
      DequeueResolutionChangeEvent();
    }
  }

  DequeueBuffers();
  EnqueueBuffers();

  // Clear the interrupt fd.
  if (!device_->ClearDevicePollInterrupt()) {
    MCIL_ERROR_PRINT(": Failed Clear the interrupt fd");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  bool poll_device = false;
  if ((input_queue_->QueuedBuffersCount() +
       output_queue_->QueuedBuffersCount()) > 0) {
    poll_device = true;
  }

  // Queue the DevicePollTask() now.
  device_poll_thread_.PostTask(std::bind(&V4L2VideoDecoder::DevicePollTask,
                                         this, poll_device));

  client_->ScheduleDecodeBufferTaskIfNeeded();
  if (resolution_change_pending)
    StartResolutionChange();
}

void V4L2VideoDecoder::SetDecoderState(CodecState state) {
  if (decoder_state_ != state) {
    MCIL_DEBUG_PRINT(": decoder_state_ [%d -> %d]",
        static_cast<int32_t>(decoder_state_), static_cast<int32_t>(state));
    decoder_state_ = state;
  }
}

bool V4L2VideoDecoder::GetCurrentInputBufferId(int32_t* buffer_id) {
  if (current_input_buffer_) {
    *buffer_id = current_input_buffer_->GetBufferId();
    return true;
  }
  return false;
}

size_t V4L2VideoDecoder::GetFreeBuffersCount(QueueType queue_type) {
  if (queue_type == OUTPUT_QUEUE)
    return output_queue_->FreeBuffersCount();
  return input_queue_->FreeBuffersCount();
}

bool V4L2VideoDecoder::AllocateOutputBuffers(
    uint32_t buffer_count,
    std::vector<WritableBufferRef*>& output_buffers) {
  uint32_t req_buffer_count =
      output_dpb_size_ + static_cast<uint32_t>(kDpbOutputBufferExtraCount);
  MCIL_DEBUG_PRINT(": request output buffers \
      ( Got: %d, requested: %d)", buffer_count, req_buffer_count);
  if (buffer_count < req_buffer_count) {
    MCIL_ERROR_PRINT(": Failed to provide requested output buffers \
        ( Got: %d, requested: %d)", buffer_count, req_buffer_count);
    NOTIFY_ERROR(INVALID_ARGUMENT);
    return false;
  }

  if (output_queue_->AllocateBuffers(buffer_count, V4L2_MEMORY_MMAP) == 0) {
    MCIL_ERROR_PRINT(": Failed to request buffers!");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }

  size_t allocated_count = output_queue_->AllocatedBuffersCount();
  if (allocated_count != buffer_count) {
    MCIL_ERROR_PRINT("Could not allocate output buffer, \
        requested [%u], allocated [%lu]", buffer_count, allocated_count);
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }

  size_t i = 0;
  while (auto buffer = output_queue_->GetFreeBufferPtr()) {
    if(i < buffer_count) {
      output_buffers[i] = (WritableBufferRef*)buffer;
      i++;
    } else {
      MCIL_ERROR_PRINT("Index i outside the bounds of output_buffers");
      NOTIFY_ERROR(PLATFORM_FAILURE);
      return false;
    }
  }

  MCIL_DEBUG_PRINT(": allocated [%lu]", output_buffers.size());
  return true;
}

bool V4L2VideoDecoder::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  if (device_) {
    auto fourcc = Fourcc::FromVideoPixelFormat(pixel_format);
    if (!fourcc)
      return false;
    return device_->CanCreateEGLImageFrom(*fourcc);
  }
  return false;
}

void V4L2VideoDecoder::OnEGLImagesCreationCompleted() {
}

#if defined (ENABLE_REACQUIRE)
void V4L2VideoDecoder::SetResolutionChangeCb(ResolutionChangeCb cb) {
  resolution_change_cb_ = std::move(cb);
}
#endif

void V4L2VideoDecoder::DevicePollTask(bool poll_device) {
  bool event_pending = false;
  if (!device_->Poll(poll_device, &event_pending)) {
    MCIL_ERROR_PRINT(": Failed during poll");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  // All processing should happen on DecodeBufferTask(), since we shouldn't
  // touch decoder state from this thread.
  client_->NotifyDecodeBufferTask(event_pending, false);
}

bool V4L2VideoDecoder::IsDecoderCmdSupported() {
  struct v4l2_decoder_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.cmd = V4L2_DEC_CMD_STOP;
  if (device_->Ioctl(VIDIOC_TRY_DECODER_CMD, &cmd) != 0) {
    MCIL_DEBUG_PRINT(": V4L2_DEC_CMD_STOP is not supported");
    return false;
  }

  return true;
}

bool V4L2VideoDecoder::SendDecoderCmdStop() {
  struct v4l2_decoder_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.cmd = V4L2_DEC_CMD_STOP;
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_DECODER_CMD, &cmd);

  flush_awaiting_last_output_buffer_ = true;

  return true;
}

bool V4L2VideoDecoder::SendDecoderCmdStart() {
  flush_awaiting_last_output_buffer_ = false;

  struct v4l2_decoder_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.cmd = V4L2_DEC_CMD_START;
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_DECODER_CMD, &cmd);

  return true;
}

bool V4L2VideoDecoder::CheckConfig(const DecoderConfig* config) {
  MCIL_DEBUG_PRINT(": profile[%d]", config->profile);

  decoder_config_.frameWidth = config->frameWidth;
  decoder_config_.frameHeight = config->frameHeight;
  decoder_config_.profile = config->profile;
  decoder_config_.outputMode = config->outputMode;

  input_format_fourcc_ =
      V4L2Device::VideoCodecProfileToV4L2PixFmt(config->profile);

  if (!input_format_fourcc_ ||
      (device_->Open(V4L2_DECODER, input_format_fourcc_) == false)) {
    MCIL_ERROR_PRINT(": Failed to open input device [%s]",
                     FourccToString(input_format_fourcc_).c_str());
    return false;
  }

  // Capabilities check.
  struct v4l2_capability caps;
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_QUERYCAP, &caps);

  const __u32 kCapsRequired = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING;
  if ((caps.capabilities & kCapsRequired) != kCapsRequired) {
    MCIL_ERROR_PRINT(": Check failed input_caps: 0x%x", caps.capabilities);
    return false;
  }

  output_mode_ = config->outputMode;

  input_queue_ = device_->GetQueue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
  if (!input_queue_) {
    MCIL_ERROR_PRINT(": Failed to create input_queue_!");
    return false;
  }

  output_queue_ = device_->GetQueue(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
  if (!output_queue_) {
    MCIL_ERROR_PRINT(": Failed to create output_queue_!");
    return false;
  }

  return SetupFormats();
}

bool V4L2VideoDecoder::SetupFormats() {
  size_t input_size;
  Size max_resolution;
  Size min_resolution;
  device_->GetSupportedResolution(
      input_format_fourcc_, &min_resolution, &max_resolution);
  if ((max_resolution.width > 1920) && (max_resolution.height > 1088))
    input_size = kInputBufferMaxSizeFor4k;
  else
    input_size = kInputBufferMaxSizeFor1080p;

  struct v4l2_fmtdesc fmtdesc;
  memset(&fmtdesc, 0, sizeof(fmtdesc));
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
  bool is_format_supported = false;
  while (device_->Ioctl(VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
    if (fmtdesc.pixelformat == input_format_fourcc_) {
      is_format_supported = true;
      break;
    }
    ++fmtdesc.index;
  }

  if (!is_format_supported) {
    MCIL_ERROR_PRINT(": Input fourcc [%s] not supported by device",
                     FourccToString(input_format_fourcc_).c_str());
    return false;
  }

  struct v4l2_format format;
  memset(&format, 0, sizeof(format));
  format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
  format.fmt.pix_mp.pixelformat = input_format_fourcc_;
  format.fmt.pix_mp.plane_fmt[0].sizeimage = static_cast<uint32_t>(input_size);
  format.fmt.pix_mp.num_planes = 1;
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_FMT, &format);

  // We have to set up the format for output, because the driver may not allow
  // changing it once we start streaming; whether it can support our chosen
  // output format or not may depend on the input format.
  memset(&fmtdesc, 0, sizeof(fmtdesc));
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  while (device_->Ioctl(VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
    auto fourcc = Fourcc::FromV4L2PixFmt(fmtdesc.pixelformat);
    if (fourcc && device_->CanCreateEGLImageFrom(*fourcc)) {
      output_format_fourcc_ = *fourcc;
      break;
    }
    ++fmtdesc.index;
  }

  if (!output_format_fourcc_) {
    MCIL_ERROR_PRINT(": Output fourcc [%s] not supported by device",
                     output_format_fourcc_->ToString().c_str());
    return false;
  } else {
    egl_image_format_fourcc_ = output_format_fourcc_;
  }

  memset(&format, 0, sizeof(format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  format.fmt.pix_mp.pixelformat = output_format_fourcc_->ToV4L2PixFmt();
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_FMT, &format);

  MCIL_DEBUG_PRINT(": Success format [%s]",
                   output_format_fourcc_->ToString().c_str());
  return true;
}

bool V4L2VideoDecoder::SubscribeEvents() {
  // Subscribe to the resolution change event.
  struct v4l2_event_subscription sub_events;
  memset(&sub_events, 0, sizeof(struct v4l2_event_subscription));
  sub_events.type = V4L2_EVENT_SOURCE_CHANGE;
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_SUBSCRIBE_EVENT, &sub_events);
  return true;
}

bool V4L2VideoDecoder::UnsubscribeEvents() {
  // Unsubscribe to the resolution change event.
  struct v4l2_event_subscription sub_events;
  memset(&sub_events, 0, sizeof(struct v4l2_event_subscription));
  sub_events.type = V4L2_EVENT_SOURCE_CHANGE;
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_UNSUBSCRIBE_EVENT, &sub_events);
  return true;
}

int32_t V4L2VideoDecoder::DequeueResolutionChangeEvent() {
  while (Optional<struct v4l2_event> event = device_->DequeueEvent()) {
    if (event->type == V4L2_EVENT_SOURCE_CHANGE) {
      if ((event->u.src_change.changes & V4L2_EVENT_SRC_CH_RESOLUTION) != 0) {
        MCIL_DEBUG_PRINT(": got resolution change event");
        return 1;
      }
    } else {
      MCIL_DEBUG_PRINT(": got an event (%d) we haven't subscribed to.",
                       event->type);
    }
  }
  return 0;
}

bool V4L2VideoDecoder::AllocateInputBuffers() {
  if (input_queue_->AllocateBuffers(kInputBufferCount,
                                    V4L2_MEMORY_MMAP) == 0) {
    MCIL_ERROR_PRINT(": Failed allocating input buffers");
    return false;
  }

  MCIL_DEBUG_PRINT(": allocated[%lu]", input_queue_->AllocatedBuffersCount());
  return true;
}

bool V4L2VideoDecoder::CreateOutputBuffers() {
  auto ctrl = device_->GetCtrl(V4L2_CID_MIN_BUFFERS_FOR_CAPTURE);
  if (!ctrl)
    return false;
  output_dpb_size_ = ctrl->value;

  uint32_t buffer_count =
      output_dpb_size_ + static_cast<uint32_t>(kDpbOutputBufferExtraCount);
  MCIL_DEBUG_PRINT(": buffer_count[%d], coded_size[%dx%d]",
                   buffer_count, coded_size_.width, coded_size_.height);

  VideoPixelFormat pixel_format = (output_mode_ == OUTPUT_IMPORT) ?
      egl_image_format_fourcc_->ToVideoPixelFormat() : PIXEL_FORMAT_UNKNOWN;
  return client_->CreateOutputBuffers(
      pixel_format, buffer_count, device_->GetTextureTarget());
}

bool V4L2VideoDecoder::DestroyOutputBuffers() {
  if (!output_queue_)
    return true;

  if (!client_->DestroyOutputBuffers())
    return true;

  bool success = true;
  if (!output_queue_->DeallocateBuffers()) {
    MCIL_ERROR_PRINT(": Failed deallocating output buffers");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    success = false;
  }

  return success;
}

void V4L2VideoDecoder::DestroyInputBuffers() {
  if (!input_queue_)
    return;

  input_queue_->DeallocateBuffers();
}

bool V4L2VideoDecoder::GetFormatInfo(struct v4l2_format* format,
                                     Size* visible_size, bool* again) {
  *again = false;
  auto ret = output_queue_->GetFormat();
  switch (ret.second) {
    case 0:
      *format = *ret.first;
      break;
    case EINVAL:
      // EINVAL means we haven't seen sufficient stream to decode the format.
      *again = true;
      return true;
    default:
      return false;
  }

  // Make sure we are still getting the format we set on initialization.
  if (format->fmt.pix_mp.pixelformat != output_format_fourcc_->ToV4L2PixFmt()) {
    MCIL_DEBUG_PRINT(": Unexpected format from G_FMT on output");
    return false;
  }

  Size coded_size(format->fmt.pix_mp.width, format->fmt.pix_mp.height);
  if (visible_size != nullptr)
    *visible_size = GetVisibleSize(coded_size);

  return true;
}

Size V4L2VideoDecoder::GetVisibleSize(const Size& coded_size) {
  auto ret = output_queue_->GetVisibleRect();
  if (!ret) {
    return coded_size;
  }

  Rect rect = std::move(*ret);
  if (!Rect(coded_size).Contains(rect)) {
    MCIL_DEBUG_PRINT(": visible rect is not inside coded size");
    return coded_size;
  }

  if (rect.IsEmpty()) {
    MCIL_DEBUG_PRINT(": visible size is empty");
    return coded_size;
  }

  // Chrome assume picture frame is coded at (0, 0).
  if (!((rect.x == 0) && (rect.y == 0))) {
    MCIL_DEBUG_PRINT(": Unexpected: top-left is not !(0,0)");
    return coded_size;
  }

  return Size(rect.width, rect.height);
}

bool V4L2VideoDecoder::CreateBuffersForFormat(
    const struct v4l2_format& format, const Size& visible_size) {
  coded_size_.width = format.fmt.pix_mp.width;
  coded_size_.height = format.fmt.pix_mp.height;

  visible_size_ = visible_size;
  MCIL_DEBUG_PRINT(": resolution[%dx%d], visible_size[%dx%d \
      decoder output planes count: [%d], EGLImage plane count[%d]",
      coded_size_.width, coded_size_.height, visible_size_.width,
      visible_size_.height, format.fmt.pix_mp.num_planes,
      format.fmt.pix_mp.num_planes);

  client_->CreateBuffersForFormat(coded_size_, visible_size_);
  return CreateOutputBuffers();
}

void V4L2VideoDecoder::NotifyErrorState(DecoderError error_code) {
  if(client_ != nullptr) {
      client_->NotifyDecoderError(error_code);
  }
}

bool V4L2VideoDecoder::EnqueueInputBuffer(V4L2WritableBufferRef buffer) {
  size_t buffer_index = buffer.BufferIndex();
  int32_t buffer_id = buffer.GetBufferId();
  size_t bytes_used = buffer.GetBytesUsed(0);

  bool ret = false;
  switch (buffer.Memory()) {
    case V4L2_MEMORY_MMAP:
      ret = std::move(buffer).QueueMMap();
      break;
    case V4L2_MEMORY_USERPTR: {
      std::vector<void*> user_ptrs;
      for (size_t i = 0; i < buffer.PlanesCount(); ++i)
        user_ptrs.push_back(buffer.GetPlaneBuffer(i));
      ret = std::move(buffer).QueueUserPtr(user_ptrs);
      break;
    }
    default:
      return false;
  }

  if (!ret) {
    MCIL_ERROR_PRINT(": Error in Enqueue input buffer");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }

  MCIL_DEBUG_PRINT(": buffer index[%ld] id[%d], size[%ld]",
                   buffer_index, buffer_id, bytes_used);
  return true;
}

bool V4L2VideoDecoder::DequeueInputBuffer() {
  auto ret = input_queue_->DequeueBuffer();
  if (ret.first == false) {
    MCIL_ERROR_PRINT(": Error in Enqueue input buffer");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  } else if (!ret.second) {
    MCIL_DEBUG_PRINT(": Dequeue input buffer. Waiting");
    return false;
  }
  return true;
}

bool V4L2VideoDecoder::EnqueueOutputBuffer(V4L2WritableBufferRef buffer) {
  size_t buffer_index = buffer.BufferIndex();
  bool ret = false;
  switch (buffer.Memory()) {
    case V4L2_MEMORY_MMAP:
      ret = std::move(buffer).QueueMMap();
      break;
    case V4L2_MEMORY_USERPTR: {
      std::vector<void*> user_ptrs;
      for (size_t i = 0; i < buffer.PlanesCount(); ++i)
        user_ptrs.push_back(buffer.GetPlaneBuffer(i));
      ret = std::move(buffer).QueueUserPtr(user_ptrs);
      break;
    }
    default:
      return false;
  }

  if (!ret) {
    MCIL_ERROR_PRINT(": Error in Enqueue output buffer");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }

  enqueued_output_buffers_++;
  MCIL_DEBUG_PRINT(": buffer index[%ld], enqueued[%d]",
                   buffer_index, enqueued_output_buffers_.load());
  return true;
}

bool V4L2VideoDecoder::DequeueOutputBuffer() {
  auto ret = output_queue_->DequeueBuffer();
  if (ret.first == false) {
    MCIL_ERROR_PRINT(": Error in Dequeue output buffer");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }
  if (!ret.second) {
    MCIL_DEBUG_PRINT(": Dequeue output buffer. Waiting");
    return false;
  }

  ReadableBufferRef buffer(std::move(ret.second));
  if (buffer->GetBytesUsed(0) > 0) {
    size_t index = buffer->BufferIndex();
    int32_t buffer_id = static_cast<int32_t>(buffer->GetTimeStamp().tv_sec);
    MCIL_DEBUG_PRINT(": Send buffer: index[%ld], id[%d]", index, buffer_id);
    client_->SendBufferToClient(index, buffer_id, buffer);
  }

  if (start_time_ == ChronoTime())
    start_time_ = std::chrono::system_clock::now();

  frames_per_sec_++;
  std::chrono::duration<double> time_past =
      std::chrono::system_clock::now() - start_time_;
  if (time_past >= std::chrono::seconds(1)) {
    current_secs_++;
    MCIL_INFO_PRINT(": Decoder @ %d secs => %d fps",
                    current_secs_, frames_per_sec_);
    start_time_ = std::chrono::system_clock::now();
    frames_per_sec_ = 0;
  }

  if (buffer->IsLast()) {
    MCIL_DEBUG_PRINT(": Got last output buffer. Waiting last buffer[%d]",
                     flush_awaiting_last_output_buffer_);
    if (flush_awaiting_last_output_buffer_) {
       if (!SendDecoderCmdStart())
         return false;
    }
  }

  return true;
}

bool V4L2VideoDecoder::StartDevicePoll() {
  if (device_poll_thread_.IsRunning())
    return true;

  client_->OnStartDevicePoll();

  device_poll_thread_.Start();
  device_poll_thread_.PostTask(std::bind(&V4L2VideoDecoder::DevicePollTask,
                                         this, false));

  return true;
}

bool V4L2VideoDecoder::StopDevicePoll() {
  if (!device_poll_thread_.IsRunning())
    return true;

  if (!device_->SetDevicePollInterrupt()) {
    MCIL_DEBUG_PRINT(": SetDevicePollInterrupt(): failed");
    return false;
  }

  device_poll_thread_.Stop();
  client_->OnStopDevicePoll();

  if (!device_->ClearDevicePollInterrupt()) {
    MCIL_DEBUG_PRINT(": ClearDevicePollInterrupt(): failed");
    return false;
  }

  return true;
}

bool V4L2VideoDecoder::StopInputStream() {
  if ((input_queue_ == nullptr) || (input_queue_->IsStreaming() == false)) {
    return true;
  }

  if (!input_queue_->StreamOff()) {
    MCIL_ERROR_PRINT(": Failed streaming off input queue");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }

  // Reset accounting info for input.
  while (!input_ready_queue_.empty())
    input_ready_queue_.pop();

  return true;
}

bool V4L2VideoDecoder::StopOutputStream() {
  if ((output_queue_ == nullptr) || (output_queue_->IsStreaming() == false)) {
    return true;
  }

  if (!output_queue_->StreamOff()) {
    MCIL_ERROR_PRINT(": Failed streaming off output queue");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return false;
  }

  // Output stream is stopped. No need to wait for the buffer anymore.
  flush_awaiting_last_output_buffer_ = false;

  return true;
}

void V4L2VideoDecoder::StartResolutionChange() {
  if (!(StopDevicePoll() && StopOutputStream()))
    return;

  client_->StartResolutionChange();

  if (!DestroyOutputBuffers()) {
    MCIL_ERROR_PRINT(": Failed destroying output buffers");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  FinishResolutionChange();
}

void V4L2VideoDecoder::FinishResolutionChange() {
  if (decoder_state_ == kDecoderError) {
    MCIL_DEBUG_PRINT(": early out: ERROR stat");
    return;
  }

  struct v4l2_format format;
  bool again;
  Size visible_size;
  bool ret = GetFormatInfo(&format, &visible_size, &again);
  if ((ret == false) || again) {
    MCIL_ERROR_PRINT(": Couldn't get format info after resolution change");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  bool ignore_resolution_change = coded_size_.IsEmpty();
  if (!CreateBuffersForFormat(format, visible_size)) {
    MCIL_ERROR_PRINT(": Couldn't reallocate buffers after resolution change");
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  #if defined (ENABLE_REACQUIRE)
  if (resolution_change_cb_ && !ignore_resolution_change)
    resolution_change_cb_(coded_size_.width, coded_size_.height);
  #endif

  StartDevicePoll();
}

}  // namespace mcil
