// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v4l2_video_encoder.h"

#include <iostream>
#include <map>
#include <memory>

#include "base/fourcc.h"
#include "base/log.h"
#include "base/video_encoder_client.h"
#include "v4l2/v4l2_device.h"
#include "v4l2/v4l2_queue.h"

namespace mcil {

#if !defined(PLATFORM_EXTENSION)
// static
scoped_refptr<VideoEncoder> V4L2VideoEncoder::Create() {
  return new V4L2VideoEncoder();
}

// static
SupportedProfiles V4L2VideoEncoder::GetSupportedProfiles() {
  scoped_refptr<V4L2Device> device = V4L2Device::Create(V4L2_ENCODER);

  if (!device)
    return SupportedProfiles();

  return device->GetSupportedEncodeProfiles();
}
#endif

V4L2VideoEncoder::InputFrameInfo::InputFrameInfo(
    scoped_refptr<VideoFrame> frame,
    bool force_keyframe)
    : frame(frame), force_keyframe(force_keyframe) {}

V4L2VideoEncoder::V4L2VideoEncoder()
 : VideoEncoder(),
   v4l2_device_(V4L2Device::Create(V4L2_ENCODER)),
   device_poll_thread_("V4L2EncoderDevicePollThread"),
   encoder_state_(kUninitialized),
   input_memory_type_(V4L2_MEMORY_MMAP),
   output_memory_type_(V4L2_MEMORY_MMAP) {
}

V4L2VideoEncoder::~V4L2VideoEncoder() {
}

bool V4L2VideoEncoder::Initialize(const EncoderConfig* config,
                                  VideoEncoderClient* client,
                                  EncoderClientConfig* client_config,
                                  int venc_port_index) {
  MCIL_DEBUG_PRINT(": profile[%d], resource index[%d] size[%dx%d]",
      config->profile, venc_port_index, config->width, config->height);

  client_ = client;
  if (!client_) {
    NOTIFY_ERROR(kInvalidArgumentError);
    MCIL_ERROR_PRINT(" Delegate not provided");
    return false;
  }

  Size input_visible_size(config->width, config->height);
  input_visible_rect_ = Rect(input_visible_size);

  output_format_fourcc_ =
      V4L2Device::VideoCodecProfileToV4L2PixFmt(config->profile);
  if (!output_format_fourcc_)
    return false;

  if (!v4l2_device_->Open(V4L2_ENCODER, output_format_fourcc_)) {
    MCIL_ERROR_PRINT(" Failed to open device for profile=%s, format=%s",
                     GetProfileName(config->profile).c_str(),
                     FourccToString(output_format_fourcc_).c_str());
    return false;
  }

  Size max_res, min_res;
  v4l2_device_->GetSupportedResolution(output_format_fourcc_, &min_res,
                                       &max_res);
  if (config->width < min_res.width || config->height < min_res.height ||
      config->width > max_res.width || config->height > max_res.height) {
    MCIL_ERROR_PRINT(" Unsupported resolution: [%dx%d], min[%dx%d], max[%dx%d]",
        config->width, config->height, min_res.width, min_res.height,
        max_res.width, max_res.height);
    return false;
  }

  struct v4l2_encoder_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.cmd = V4L2_ENC_CMD_STOP;
  is_flush_supported_ =
      (v4l2_device_->Ioctl(VIDIOC_TRY_ENCODER_CMD, &cmd) == 0);
  if (!is_flush_supported_) {
    MCIL_DEBUG_PRINT(" V4L2_ENC_CMD_STOP is not supported.");
  }

  struct v4l2_capability caps;
  memset(&caps, 0, sizeof(caps));
  uint32_t kCapsRequired = GetCapsRequired();
  IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_QUERYCAP, &caps);
  if ((caps.capabilities & kCapsRequired) != kCapsRequired) {
    MCIL_ERROR_PRINT(" caps check failed: %x", caps.capabilities);
    return false;
  }

  input_queue_ = v4l2_device_->GetQueue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
  output_queue_ = v4l2_device_->GetQueue(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
  if (!input_queue_ || !output_queue_) {
    MCIL_ERROR_PRINT(" Failed to get V4L2Queue.");
    NOTIFY_ERROR(kPlatformFailureError);
    return false;
  }

  encoder_config_.bitRate = config->bitRate;
  encoder_config_.width = config->width;
  encoder_config_.height = config->height;
  encoder_config_.frameRate = config->frameRate;
  encoder_config_.pixelFormat = config->pixelFormat;
  encoder_config_.outputBufferSize = config->outputBufferSize;
  encoder_config_.profile = config->profile;
  encoder_config_.h264OutputLevel = config->h264OutputLevel;
  encoder_config_.gopLength = config->gopLength;

  if (!SetFormats(config->pixelFormat, config->profile)) {
    MCIL_ERROR_PRINT(" Failed setting up formats.");
    return false;
  }

  InitInputMemoryType();
  InitOutputMemoryType();

  if (!InitControls(config))
    return false;

  if (!CreateOutputBuffers())
    return false;

  UpdateEncodingParams(encoder_config_.bitRate, encoder_config_.frameRate);

  if (client_config) {
    client_config->should_control_buffer_feed = false;
    client_config->output_buffer_byte_size = encoder_config_.outputBufferSize;
    client_config->should_inject_sps_and_pps = inject_sps_and_pps_;
    client_config->input_frame_size = input_frame_size_;
  }

  return true;
}

void V4L2VideoEncoder::Destroy() {
  StopDevicePoll();

  DestroyInputBuffers();
  DestroyOutputBuffers();

  v4l2_device_ = nullptr;
  start_time_ = ChronoTime();
}

bool V4L2VideoEncoder::IsFlushSupported() {
  return is_flush_supported_;
}

bool V4L2VideoEncoder::EncodeFrame(scoped_refptr<VideoFrame> frame,
                                   bool force_keyframe) {
  MCIL_DEBUG_PRINT(": force_keyframe[%d]", force_keyframe);

  if (encoder_state_ == kEncoderError) {
    MCIL_DEBUG_PRINT(" early out: kError state");
    return false;
  }

  if (frame && !input_buffer_created_ && !CreateInputBuffers())
    return false;

  encoder_input_queue_.emplace(std::move(frame), force_keyframe);
  EnqueueBuffers();

  return true;
}

bool V4L2VideoEncoder::FlushFrames() {
  if (encoder_state_ == kEncoderError) {
    MCIL_DEBUG_PRINT(" early out: kError state");
    return false;
  }

  if (!input_buffer_created_) {
    MCIL_DEBUG_PRINT(" Valid input frames are not queued yet.");
    return false;
  }

  encoder_input_queue_.emplace(nullptr, false);
  EnqueueBuffers();

  return true;
}

bool V4L2VideoEncoder::UpdateEncodingParams(
    uint32_t bitrate, uint32_t framerate) {
  MCIL_DEBUG_PRINT(": bitrate[%d], framerate[%d]", bitrate, framerate);

  if (bitrate == 0 || framerate == 0)
    return true;

  if (ShouldSetEncodingParams()) {
    if (current_bitrate_ != bitrate &&
        !v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                               V4L2_CID_MPEG_VIDEO_BITRATE, bitrate)) {
      MCIL_ERROR_PRINT(" Failed changing bitrate");
      NOTIFY_ERROR(kPlatformFailureError);
      return false;
    }

    if (current_framerate_ != framerate) {
      struct v4l2_streamparm parms;
      memset(&parms, 0, sizeof(parms));
      parms.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
      parms.parm.output.timeperframe.numerator = 1;
      parms.parm.output.timeperframe.denominator = framerate;
      IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_PARM, &parms);
    }
  }

  current_bitrate_ = bitrate;
  current_framerate_ = framerate;
  return true;
}

bool V4L2VideoEncoder::StartDevicePoll() {
  if (device_poll_thread_.IsRunning())
    return true;

  device_poll_thread_.Start();
  device_poll_thread_.PostTask(std::bind(&V4L2VideoEncoder::DevicePollTask,
                                         this, false));
  return true;
}

void V4L2VideoEncoder::RunEncodeBufferTask() {
  if (encoder_state_ == kEncoderError) {
    MCIL_DEBUG_PRINT(" early out: kError state");
    return;
  }

  DequeueBuffers();
  EnqueueBuffers();

  // Clear the interrupt fd.
  if (!v4l2_device_->ClearDevicePollInterrupt())
    return;

  bool poll_device = (input_queue_->QueuedBuffersCount() +
                          output_queue_->QueuedBuffersCount() >
                      0);
  device_poll_thread_.PostTask(std::bind(&V4L2VideoEncoder::DevicePollTask,
                                         this, poll_device));

  MCIL_DEBUG_PRINT(" [%ld] => DEVICE[%ld+%ld/%d->%ld+%ld/%ld] => OUT[%d]",
      encoder_input_queue_.size(), input_queue_->FreeBuffersCount(),
      input_queue_->QueuedBuffersCount(), kInputBufferCount,
      output_queue_->FreeBuffersCount(), output_queue_->QueuedBuffersCount(),
      output_queue_->AllocatedBuffersCount(), kOutputBufferCount);
}

void V4L2VideoEncoder::SendStartCommand(bool start) {
  struct v4l2_encoder_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.cmd = V4L2_ENC_CMD_START;
  IOCTL_OR_ERROR_RETURN(VIDIOC_ENCODER_CMD, &cmd);
}

void V4L2VideoEncoder::SetEncoderState(CodecState state) {
  if (encoder_state_ != state) {
    MCIL_DEBUG_PRINT(": encoder_state_ [%d -> %d]",
        static_cast<int>(encoder_state_), static_cast<int>(state));
    encoder_state_ = state;
  }
}

size_t V4L2VideoEncoder::GetFreeBuffersCount(QueueType queue_type) {
  if (queue_type == OUTPUT_QUEUE)
    return output_queue_->FreeBuffersCount();
  return input_queue_->FreeBuffersCount();
}

void V4L2VideoEncoder::EnqueueBuffers() {
  MCIL_DEBUG_PRINT(" free_input_buffers[%ld], input_queue[%ld]",
      input_queue_->FreeBuffersCount(), encoder_input_queue_.size());

  bool do_streamon = false;
  const size_t old_inputs_queued = input_queue_->QueuedBuffersCount();
  while (!encoder_input_queue_.empty() &&
         input_queue_->FreeBuffersCount() > 0) {
    if (encoder_input_queue_.front().frame == nullptr) {
      MCIL_DEBUG_PRINT(" All input frames needed to be flushed are enqueued.");
      encoder_input_queue_.pop();

      if (!input_queue_->IsStreaming()) {
        client_->NotifyFlushIfNeeded(true);
        return;
      }
      struct v4l2_encoder_cmd cmd;
      memset(&cmd, 0, sizeof(cmd));
      cmd.cmd = V4L2_ENC_CMD_STOP;
      if (v4l2_device_->Ioctl(VIDIOC_ENCODER_CMD, &cmd) != 0) {
        MCIL_ERROR_PRINT(" ioctl() failed: VIDIOC_ENCODER_CMD");
        NOTIFY_ERROR(kPlatformFailureError);
        client_->NotifyFlushIfNeeded(false);
        return;
      }
      client_->NotifyEncoderState(kFlushing);
      break;
    }

    Optional<V4L2WritableBufferRef> input = input_queue_->GetFreeBuffer();
    if (!input)
      return;

    if (!EnqueueInputBuffer(std::move(*input)))
      return;
  }

  if (old_inputs_queued == 0 && input_queue_->QueuedBuffersCount() != 0) {
    if (!v4l2_device_->SetDevicePollInterrupt())
      return;
    do_streamon = !input_queue_->IsStreaming();
  }

  if (!input_queue_->IsStreaming() && !do_streamon) {
    return;
  }

  const size_t old_outputs_queued = output_queue_->QueuedBuffersCount();
  while (auto output = output_queue_->GetFreeBuffer()) {
    if (!EnqueueOutputBuffer(std::move(*output)))
      return;
  }
  if (old_outputs_queued == 0 && output_queue_->QueuedBuffersCount() != 0) {
    if (!v4l2_device_->SetDevicePollInterrupt())
      return;
  }

  if (do_streamon && DoStreamOnInEnqueueBuffers()) {
    output_queue_->StreamOn();
    input_queue_->StreamOn();
  }
}

scoped_refptr<VideoFrame> V4L2VideoEncoder::GetDeviceInputFrame() {
  return device_input_frame_;
}

bool V4L2VideoEncoder::NegotiateInputFormat(VideoPixelFormat format,
                                            const Size& frame_size) {
  return SetInputFormat(format, frame_size).has_value();
}

void V4L2VideoEncoder::DequeueBuffers() {
  while (input_queue_->QueuedBuffersCount() > 0) {
    if (!DequeueInputBuffer())
      break;
  }

  bool buffer_dequeued = false;
  while (output_queue_->QueuedBuffersCount() > 0) {
    if (!DequeueOutputBuffer())
      break;
    buffer_dequeued = true;
  }

  if (buffer_dequeued)
    client_->PumpBitstreamBuffers();
}

uint32_t V4L2VideoEncoder::GetCapsRequired() {
  return V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING;
}

void V4L2VideoEncoder::InitInputMemoryType() {
  input_memory_type_ = V4L2_MEMORY_MMAP;
}

void V4L2VideoEncoder::InitOutputMemoryType() {
  output_memory_type_ = V4L2_MEMORY_MMAP;
}

bool V4L2VideoEncoder::SetFormats(VideoPixelFormat input_format,
                                  VideoCodecProfile output_profile) {
  MCIL_DEBUG_PRINT(": format[%d], profile[%d]", input_format, output_profile);

  if (input_queue_->IsStreaming() || output_queue_->IsStreaming()) {
    MCIL_DEBUG_PRINT(": Already streaming. Return.");
    return true;
  }

  if (!SetOutputFormat(output_profile))
    return false;

  Size input_size = Size(encoder_config_.width, encoder_config_.height);
  auto v4l2_format = SetInputFormat(input_format, input_size);
  if (!v4l2_format) {
    MCIL_ERROR_PRINT(": Failed to set Input format [%d], input[%dx%d]",
                     input_format, input_size.width, input_size.height);
    return false;
  }

  input_frame_size_ = V4L2Device::AllocatedSizeFromV4L2Format(*v4l2_format);
  MCIL_DEBUG_PRINT(" input_frame_size_[%dx%d]", input_frame_size_.width,
                   input_frame_size_.height);

  return true;
}

bool V4L2VideoEncoder::SetOutputFormat(VideoCodecProfile output_profile) {
  Optional<struct v4l2_format> format =
      output_queue_->SetFormat(output_format_fourcc_,
                               input_visible_rect_.size(),
                               encoder_config_.outputBufferSize);
  if (!format) {
    MCIL_ERROR_PRINT(": Failed to set Output format [%s], input[%dx%d], \
        output[%lu]", FourccToString(output_format_fourcc_).c_str(),
        input_visible_rect_.size().width, input_visible_rect_.size().height,
        encoder_config_.outputBufferSize);
    return false;
  }

  size_t adjusted_output_buffer_size =
      static_cast<size_t>(format->fmt.pix_mp.plane_fmt[0].sizeimage);
  encoder_config_.outputBufferSize = adjusted_output_buffer_size;

  return true;
}

Optional<struct v4l2_format> V4L2VideoEncoder::SetInputFormat(
    VideoPixelFormat pixel_format, const Size& frame_size) {
  MCIL_DEBUG_PRINT(" frame_size[%dx%d]", frame_size.width, frame_size.height);

  std::vector<uint32_t> pix_fmt_candidates;
  auto input_fourcc = Fourcc::FromVideoPixelFormat(pixel_format, false);
  if (!input_fourcc) {
    MCIL_ERROR_PRINT(", Invalid input format: %s",
                     VideoPixelFormatToString(pixel_format).c_str());
    return nullopt;
  }

  pix_fmt_candidates.push_back(input_fourcc->ToV4L2PixFmt());
  for (auto preferred_format :
       v4l2_device_->PreferredInputFormat(V4L2_ENCODER)) {
    pix_fmt_candidates.push_back(preferred_format);
  }

  for (const auto pix_fmt : pix_fmt_candidates) {
    MCIL_DEBUG_PRINT("Trying S_FMT with %s", FourccToString(pix_fmt).c_str());

    Optional<struct v4l2_format> format =
        input_queue_->SetFormat(pix_fmt, frame_size, 0);
    if (!format)
      continue;

    MCIL_DEBUG_PRINT("Success S_FMT with %s", FourccToString(pix_fmt).c_str());
    device_input_frame_ = V4L2Device::VideoFrameFromV4L2Format(*format);
    if (!device_input_frame_) {
      MCIL_ERROR_PRINT(" Invalid device_input_frame_");
      return nullopt;
    }

    if (!Rect(device_input_frame_->coded_size).Contains(Rect(frame_size))) {
      MCIL_ERROR_PRINT(" Input size[%dx%d], exceeds encoder size[%dx%d]",
                       frame_size.width, frame_size.height,
                       device_input_frame_->coded_size.width,
                       device_input_frame_->coded_size.height);
      return nullopt;
    }

    if (ShouldApplyCrop() && !ApplyCrop()) {
      MCIL_ERROR_PRINT(" Failed to Apply Corp");
      return nullopt;
    }

    return format;
  }

  return nullopt;
}

bool V4L2VideoEncoder::ApplyCrop() {
  struct v4l2_rect visible_rect;
  visible_rect.left = input_visible_rect_.x;
  visible_rect.top = input_visible_rect_.y;
  visible_rect.width = input_visible_rect_.width;
  visible_rect.height = input_visible_rect_.height;

  struct v4l2_selection selection_arg;
  memset(&selection_arg, 0, sizeof(selection_arg));
  selection_arg.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  selection_arg.target = V4L2_SEL_TGT_CROP;
  selection_arg.r = visible_rect;

  if (v4l2_device_->Ioctl(VIDIOC_S_SELECTION, &selection_arg) == 0) {
    MCIL_DEBUG_PRINT(" VIDIOC_S_SELECTION is supported");
    visible_rect = selection_arg.r;
  } else {
    MCIL_DEBUG_PRINT(" Fallback to VIDIOC_S/G_CROP");
    struct v4l2_crop crop;
    memset(&crop, 0, sizeof(crop));
    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    crop.c = visible_rect;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_CROP, &crop);
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_G_CROP, &crop);
    visible_rect = crop.c;
  }

  const Rect adjusted_visible_rect(visible_rect.left, visible_rect.top,
                                   visible_rect.width, visible_rect.height);
  if (input_visible_rect_ != adjusted_visible_rect) {
    MCIL_ERROR_PRINT(" input_visible_rect_[%dx%d], adjusted_visible_rect[%dx%d",
                     input_visible_rect_.width, input_visible_rect_.height,
                     adjusted_visible_rect.width, adjusted_visible_rect.height);
    return false;
  }
  return true;
}

bool V4L2VideoEncoder::InitControls(const EncoderConfig* config) {
  switch (output_format_fourcc_) {
    case V4L2_PIX_FMT_H264:
      if (!InitControlsH264(config)) {
        return false;
      }
      break;
    case V4L2_PIX_FMT_VP8:
      InitControlsVP8(config);
      break;
    default:
      MCIL_ERROR_PRINT("Unsupported codec %s",
                       FourccToString(output_format_fourcc_).c_str());
      return false;
  }

  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE, 1);
  v4l2_device_->SetGOPLength(config->gopLength);

  return true;
}

bool V4L2VideoEncoder::InitControlsH264(const EncoderConfig* config) {
  inject_sps_and_pps_ = ShouldInjectSpsPps();

  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG, V4L2_CID_MPEG_VIDEO_B_FRAMES, 0);

  int32_t profile_value =
      V4L2Device::VideoCodecProfileToV4L2H264Profile(config->profile);
  if (profile_value < 0) {
    MCIL_ERROR_PRINT(" Invalid H264 profile value");
    NOTIFY_ERROR(kInvalidArgumentError);
    return false;
  }

  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_H264_PROFILE, profile_value);

  uint8_t h264_level = client_->GetH264LevelLimit(config);
  int32_t level_value = V4L2Device::H264LevelIdcToV4L2H264Level(h264_level);
  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_H264_LEVEL, level_value);
  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_HEADER_MODE,
                        V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME);

  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG, V4L2_CID_MPEG_VIDEO_H264_I_PERIOD,
                        0);

  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE,
                        V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_ENABLED);

  if (config->profile == H264PROFILE_MAIN ||
      config->profile == H264PROFILE_HIGH) {
    v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                          V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE,
                          V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC);
  }

  if (config->profile == H264PROFILE_HIGH) {
    v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                          V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM, true);
  }

  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_H264_MAX_QP, 42);
  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_H264_MIN_QP, 24);

  return true;
}

void V4L2VideoEncoder::InitControlsVP8(const EncoderConfig* config) {
  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_VPX_MIN_QP, 4);
  v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                        V4L2_CID_MPEG_VIDEO_VPX_MAX_QP, 117);
}

void V4L2VideoEncoder::NotifyErrorState(EncoderError error_code) {
  MCIL_ERROR_PRINT(" error_code[%d]", error_code);
  client_->NotifyEncoderError(error_code);
}

bool V4L2VideoEncoder::CreateInputBuffers() {
  if (input_queue_->AllocateBuffers(kInputBufferCount, input_memory_type_) <
      kInputBufferCount) {
    MCIL_ERROR_PRINT(" Failed to allocate V4L2 input buffers.");
    return false;
  }

  size_t allocated = input_queue_->AllocatedBuffersCount();
  client_->CreateInputBuffers(allocated);
  input_buffer_created_ = true;

  MCIL_DEBUG_PRINT(" allocated[%ld]", allocated);
  return true;
}

bool V4L2VideoEncoder::CreateOutputBuffers() {
  if (output_queue_->AllocateBuffers(kOutputBufferCount, output_memory_type_) <
      kOutputBufferCount) {
    MCIL_ERROR_PRINT(" Failed to allocate V4L2 output buffers.");
    return false;
  }

  size_t allocated = output_queue_->AllocatedBuffersCount();
  MCIL_DEBUG_PRINT(" allocated[%ld]", allocated);
  return true;
}

void V4L2VideoEncoder::DestroyInputBuffers() {
  if (!input_queue_ || input_queue_->AllocatedBuffersCount() == 0)
    return;

  input_queue_->DeallocateBuffers();
  client_->DestroyInputBuffers();
}

void V4L2VideoEncoder::DestroyOutputBuffers() {
  if (!output_queue_ || output_queue_->AllocatedBuffersCount() == 0)
    return;

  output_queue_->DeallocateBuffers();
}

bool V4L2VideoEncoder::EnqueueInputBuffer(V4L2WritableBufferRef buffer) {
  InputFrameInfo frame_info = encoder_input_queue_.front();
  if (frame_info.force_keyframe) {
    if (!v4l2_device_->SetCtrl(V4L2_CTRL_CLASS_MPEG,
                               V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME, 0)) {
      MCIL_ERROR_PRINT(" Failed requesting keyframe");
      NOTIFY_ERROR(kPlatformFailureError);
      return false;
    }
  }

  scoped_refptr<VideoFrame> frame = std::move(frame_info.frame);

  size_t buffer_index = buffer.BufferIndex();
  buffer.SetTimeStamp(frame->timestamp);

  size_t num_planes = V4L2Device::GetNumPlanesOfV4L2PixFmt(
      Fourcc::FromVideoPixelFormat(device_input_frame_->format,
                                   !device_input_frame_->is_multi_planar)
          ->ToV4L2PixFmt());

  for (size_t i = 0; i < num_planes; ++i) {
    size_t bytesused = 0;
    if (num_planes == 1) {
      bytesused = VideoFrame::AllocationSize(
          frame->format, device_input_frame_->coded_size);
    } else {
      bytesused = static_cast<size_t>(
          VideoFrame::PlaneSize(frame->format, i,
                                device_input_frame_->coded_size)
              .GetArea());
    }

    switch (buffer.Memory()) {
      case V4L2_MEMORY_MMAP: {
        size_t plane_size = buffer.GetBufferSize(i);
        size_t bytes_used = buffer.GetBytesUsed(i);
        void* mapping = buffer.GetPlaneBuffer(i);
        memcpy(reinterpret_cast<uint8_t*>(mapping) + bytes_used,
               frame->data[i], plane_size);
        break;
      }
      case V4L2_MEMORY_USERPTR:
        buffer.SetBufferSize(i, device_input_frame_->color_planes[i].size);
        break;
      default:
        return false;
    }

    buffer.SetBytesUsed(i, bytesused);
  }

  switch (buffer.Memory()) {
    case V4L2_MEMORY_MMAP: {
      std::move(buffer).QueueMMap();
      break;
    }
    case V4L2_MEMORY_USERPTR: {
      std::vector<void*> user_ptrs;
      for (size_t i = 0; i < buffer.PlanesCount(); ++i)
        user_ptrs.push_back(buffer.GetPlaneBuffer(i));
      std::move(buffer).QueueUserPtr(std::move(user_ptrs));
      break;
    }
    default:
      return false;
  }

  client_->EnqueueInputBuffer(buffer_index);
  encoder_input_queue_.pop();

  return true;
}

bool V4L2VideoEncoder::DequeueInputBuffer() {
  MCIL_DEBUG_PRINT(" inputs queued: %ld", input_queue_->QueuedBuffersCount());

  auto ret = input_queue_->DequeueBuffer();
  if (!ret.first) {
    NOTIFY_ERROR(kPlatformFailureError);
    return false;
  }

  if (!ret.second) {
    // We're just out of buffers to dequeue.
    return false;
  }

  size_t buffer_index = ret.second->BufferIndex();
  client_->DequeueInputBuffer(buffer_index);
  return true;
}

bool V4L2VideoEncoder::EnqueueOutputBuffer(V4L2WritableBufferRef buffer) {
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
    MCIL_DEBUG_PRINT(": Error in Enqueue output buffer");
    return false;
  }

  return true;
}

bool V4L2VideoEncoder::DequeueOutputBuffer() {
  auto ret = output_queue_->DequeueBuffer();
  if (!ret.first) {
    NOTIFY_ERROR(kPlatformFailureError);
    return false;
  }

  if (!ret.second) {
    // We're just out of buffers to dequeue.
    return false;
  }

  if (start_time_ == ChronoTime())
    start_time_ = std::chrono::system_clock::now();

  frames_per_sec_++;
  std::chrono::duration<double> time_past =
      std::chrono::system_clock::now() - start_time_;
  if (time_past >= std::chrono::seconds(1)) {
    current_secs_++;
    MCIL_INFO_PRINT(": Encoder @ %d secs => %d fps",
                    current_secs_, frames_per_sec_);
    start_time_ = std::chrono::system_clock::now();
    frames_per_sec_ = 0;
  }

  client_->BitstreamBufferReady(std::move(ret.second));
  return true;
}

bool V4L2VideoEncoder::StopDevicePoll() {
  if (!device_poll_thread_.IsRunning())
    return true;

  if (!v4l2_device_->SetDevicePollInterrupt())
    return false;

  device_poll_thread_.Stop();

  if (!v4l2_device_->ClearDevicePollInterrupt())
    return false;

  // Tegra driver cannot call Streamoff() when the stream is off, so we check
  // IsStreaming() first.
  if (input_queue_ && input_queue_->IsStreaming() && !input_queue_->StreamOff())
    return false;

  if (output_queue_ && output_queue_->IsStreaming() &&
      !output_queue_->StreamOff())
    return false;

  // Reset all our accounting info.
  while (!encoder_input_queue_.empty())
    encoder_input_queue_.pop();

  client_->StopDevicePoll();

  MCIL_DEBUG_PRINT(" device poll thread stopped");
  return true;
}

void V4L2VideoEncoder::DevicePollTask(bool poll_device) {
  bool event_pending;
  if (!v4l2_device_->Poll(poll_device, &event_pending)) {
    NOTIFY_ERROR(kPlatformFailureError);
    return;
  }

  client_->NotifyEncodeBufferTask();
}

}  // namespace mcil
