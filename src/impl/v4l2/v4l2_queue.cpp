// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v4l2_queue.h"

#include "base/log.h"
#include "v4l2/v4l2_buffers.h"
#include "v4l2/v4l2_device.h"

namespace mcil {

namespace {

Rect V4L2RectToMCILRect(const v4l2_rect& rect) {
  return Rect(rect.left, rect.top, rect.width, rect.height);
}

struct v4l2_format BuildV4L2Format(const enum v4l2_buf_type buffer_type,
                                   uint32_t fourcc,
                                   const Size& size,
                                   size_t buffer_size) {
  struct v4l2_format format;
  memset(&format, 0, sizeof(format));
  format.type = buffer_type;
  format.fmt.pix_mp.pixelformat = fourcc;
  format.fmt.pix_mp.width = size.width;
  format.fmt.pix_mp.height = size.height;
  format.fmt.pix_mp.num_planes =
      static_cast<uint8_t>(V4L2Device::GetNumPlanesOfV4L2PixFmt(fourcc));
  format.fmt.pix_mp.plane_fmt[0].sizeimage = static_cast<uint32_t>(buffer_size);

  return format;
}

}  // namespace

#if !defined(PLATFORM_EXTENSION)
// static
scoped_refptr<V4L2Queue> V4L2Queue::Create(scoped_refptr<V4L2Device> dev,
                                           enum v4l2_buf_type type,
                                           V4L2BufferDestroyCb destroy_cb) {
    return new V4L2Queue(std::move(dev), type, std::move(destroy_cb));
}
#endif

/* V4L2BufferRefFactory */
V4L2WritableBufferRef V4L2BufferRefFactory::CreateWritableRef(
    const struct v4l2_buffer& buffer, V4L2Queue* queue) {
  return V4L2WritableBufferRef(buffer, queue);
}

V4L2WritableBufferRef* V4L2BufferRefFactory::CreateWritableRefPtr(
    const struct v4l2_buffer& buffer, V4L2Queue* queue) {
  return new V4L2WritableBufferRef(buffer, queue);
}

ReadableBufferRef V4L2BufferRefFactory::CreateReadableRef(
    const struct v4l2_buffer& buffer,
    V4L2Queue* queue,
    scoped_refptr<VideoFrame> video_frame) {
  V4L2ReadableBuffer* readable_buffer =
      new V4L2ReadableBuffer(buffer, queue, std::move(video_frame));
  return (ReadableBuffer*)readable_buffer;
}

/* V4L2Queue */
V4L2Queue::V4L2Queue(scoped_refptr<V4L2Device> device,
                     enum v4l2_buf_type buffer_type,
                     V4L2BufferDestroyCb destroy_cb)
  : buffer_type_(buffer_type),
    device_(std::move(device)) {
}

V4L2Queue::~V4L2Queue() noexcept(false){
}

Optional<Rect> V4L2Queue::GetVisibleRect() {
  enum v4l2_buf_type compose_type;
  switch (buffer_type_) {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
      compose_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      break;
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
      compose_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
      break;
    default:
      compose_type = buffer_type_;
      break;
  }

  struct v4l2_selection selection;
  memset(&selection, 0, sizeof(selection));
  selection.type = compose_type;
  selection.target = V4L2_SEL_TGT_COMPOSE;
  if (device_->Ioctl(VIDIOC_G_SELECTION, &selection) == 0) {
    return V4L2RectToMCILRect(selection.r);
  } else {
    MCIL_DEBUG_PRINT(": Fallback to VIDIOC_G_CROP");
  }

  struct v4l2_crop crop;
  memset(&crop, 0, sizeof(crop));
  crop.type = buffer_type_;
  if (device_->Ioctl(VIDIOC_G_CROP, &crop) == 0) {
    return V4L2RectToMCILRect(crop.c);
  }

  return nullopt;
}

std::pair<Optional<struct v4l2_format>, int32_t > V4L2Queue::GetFormat() {
  struct v4l2_format format;
  memset(&format, 0, sizeof(format));
  format.type = buffer_type_;

  if (device_->Ioctl(VIDIOC_G_FMT, &format) != 0) {
    MCIL_DEBUG_PRINT(": Failed to VIDIOC_G_FMT get format");
    return std::make_pair(nullopt, errno);
  }

  return std::make_pair(format, static_cast<int32_t>(0));
}

Optional<struct v4l2_format> V4L2Queue::SetFormat(uint32_t fourcc,
                                                  const Size& size,
                                                  size_t buffer_size) {
  struct v4l2_format format =
      BuildV4L2Format(buffer_type_, fourcc, size, buffer_size);
  if ((device_->Ioctl(VIDIOC_S_FMT, &format) != 0) ||
      (format.fmt.pix_mp.pixelformat != fourcc)) {
    MCIL_DEBUG_PRINT(": Failed to set format fourcc [0x%x]", fourcc);
    return nullopt;
  }

  current_format_ = format;
  return current_format_;
}

bool V4L2Queue::StreamOn() {
  if (streaming_state_)
    return true;

  int32_t arg = static_cast<int32_t>(buffer_type_);
  int32_t ret = device_->Ioctl(VIDIOC_STREAMON, &arg);
  if (ret != 0) {
    MCIL_ERROR_PRINT(": VIDIOC_STREAMON Failed");
    return false;
  }

  streaming_state_ = true;
  return true;
}

bool V4L2Queue::StreamOff() {
  int32_t arg = static_cast<int32_t>(buffer_type_);
  int32_t  ret = device_->Ioctl(VIDIOC_STREAMOFF, &arg);
  if (ret != 0) {
    MCIL_ERROR_PRINT(": VIDIOC_STREAMOFF Failed");
    return false;
  }

  for (const auto& it : queued_buffers_)
    free_buffers_->ReturnBuffer(it.first);

  queued_buffers_.clear();
  streaming_state_ = false;
  return true;
}

bool V4L2Queue::IsStreaming() const {
  return streaming_state_;
}

size_t V4L2Queue::AllocateBuffers(size_t count, enum v4l2_memory memory) {
  size_t buffer_count = count;
  if (buffer_count == 0) {
    MCIL_ERROR_PRINT(": Attempting to allocate 0 buffers");
    return 0;
  }

  if (IsStreaming()) {
    MCIL_ERROR_PRINT(": Cannot allocate buffers while streaming.");
    return 0;
  }

  if (buffers_.size() != 0) {
    MCIL_ERROR_PRINT(": Cannot allocate new buffers while others \
                     are still allocated.");
    return 0;
  }

  Optional<v4l2_format> format = GetFormat().first;
  if (!format) {
    MCIL_ERROR_PRINT(": Cannot get format");
    return 0;
  }

  planes_count_ = format->fmt.pix_mp.num_planes;
  struct v4l2_requestbuffers reqbufs;
  memset(&reqbufs, 0, sizeof(reqbufs));
  reqbufs.count = static_cast<uint32_t>(buffer_count);
  reqbufs.type = buffer_type_;
  reqbufs.memory = memory;

  int32_t ret = device_->Ioctl(VIDIOC_REQBUFS, &reqbufs);
  if (ret) {
    MCIL_ERROR_PRINT(": VIDIOC_REQBUFS Failed, %s", strerror(errno));
    return 0;
  }

  buffer_count = reqbufs.count;
  MCIL_DEBUG_PRINT(": queue[%d] got [%lu] buffers", buffer_type_, buffer_count);

  memory_ = memory;
  free_buffers_ = new V4L2BuffersList();

  // Now query all buffer information.
  for (size_t i = 0; i < buffer_count; i++) {
    auto buffer =
        V4L2Buffer::Create(device_, buffer_type_, memory_, *format, i);
    if (buffer && (buffer->Query() == false)) {
      DeallocateBuffers();
      return 0;
    }

    buffers_.emplace_back(std::move(buffer));
    free_buffers_->ReturnBuffer(i);
  }

  return buffers_.size();
}

bool V4L2Queue::DeallocateBuffers() {
  if (IsStreaming()) {
    MCIL_DEBUG_PRINT(": Cannot deallocate buffers while streaming.");
    return false;
  }

  if (buffers_.size() == 0)
    return true;

  buffers_.clear();
  free_buffers_ = nullptr;

  struct v4l2_requestbuffers reqbufs;
  memset(&reqbufs, 0, sizeof(reqbufs));
  reqbufs.count = 0;
  reqbufs.type = buffer_type_;
  reqbufs.memory = memory_;

  int32_t ret = device_->Ioctl(VIDIOC_REQBUFS, &reqbufs);
  if (ret) {
    MCIL_DEBUG_PRINT(": VIDIOC_REQBUFS errno(%d)", errno);
    return false;
  }
  return true;
}

size_t V4L2Queue::AllocatedBuffersCount() const {
  return buffers_.size();
}

size_t V4L2Queue::FreeBuffersCount() const {
  return free_buffers_ ? free_buffers_->GetSize() : 0;
}

size_t V4L2Queue::QueuedBuffersCount() const {
  return queued_buffers_.size();
}

Optional<V4L2WritableBufferRef> V4L2Queue::GetFreeBuffer() {
  // No buffers allocated at the moment?
  if (!free_buffers_) {
    MCIL_DEBUG_PRINT(": free_buffers_ not set");
    return nullopt;
  }

  auto buffer_id = free_buffers_->GetFreeBuffer();
  if (!buffer_id.has_value())
    return nullopt;

  return V4L2BufferRefFactory::CreateWritableRef(
      buffers_[buffer_id.value()]->get_v4l2_buffer(), this);
}

V4L2WritableBufferRef* V4L2Queue::GetFreeBufferPtr() {
  // No buffers allocated at the moment?
  if (!free_buffers_) {
    MCIL_DEBUG_PRINT(": free_buffers_ not set");
    return nullptr;
  }

  auto buffer_id = free_buffers_->GetFreeBuffer();
  if (!buffer_id.has_value())
    return nullptr;

  return V4L2BufferRefFactory::CreateWritableRefPtr(
      buffers_[buffer_id.value()]->get_v4l2_buffer(), this);
}

std::pair<bool, ReadableBufferRef> V4L2Queue::DequeueBuffer() {
  if (QueuedBuffersCount() == 0) {
    MCIL_DEBUG_PRINT(": No buffers queued yet");
    return std::make_pair(true, nullptr);
  }

  if (!IsStreaming()) {
    MCIL_DEBUG_PRINT(": Attempting to dequeue a buffer while not streaming.");
    return std::make_pair(true, nullptr);
  }

  struct v4l2_buffer read_v4l2_buf;
  memset(&read_v4l2_buf, 0, sizeof(read_v4l2_buf));

  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  memset(planes, 0, sizeof(planes));
  read_v4l2_buf.type = buffer_type_;
  read_v4l2_buf.memory = memory_;
  read_v4l2_buf.m.planes = planes;
  read_v4l2_buf.length = static_cast<uint32_t>(planes_count_);
  int32_t ret = device_->Ioctl(VIDIOC_DQBUF, &read_v4l2_buf);
  if (ret != 0) {
    switch (errno) {
      case EAGAIN:
      case EPIPE:
        return std::make_pair(true, nullptr);
      default: {
        MCIL_DEBUG_PRINT(
            ": %s VIDIOC_DQBUF failed errno(%d)",
            ((buffer_type_ == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) ? "input"
                                                                : "output"),
            errno);
        return std::make_pair(false, nullptr);
      }
    }
  }

  auto it = queued_buffers_.find(read_v4l2_buf.index);
  if (it == queued_buffers_.end())
    return std::make_pair(false, nullptr);

  scoped_refptr<VideoFrame> queued_frame = std::move(it->second);
  queued_buffers_.erase(it);

  return std::make_pair(true, V4L2BufferRefFactory::CreateReadableRef(
                                  read_v4l2_buf, this,
                                  std::move(queued_frame)));
}

bool V4L2Queue::QueueBuffer(struct v4l2_buffer* buffer,
                            scoped_refptr<VideoFrame> video_frame) {
  int32_t  ret = device_->Ioctl(VIDIOC_QBUF, buffer);
  if (ret) {
    if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
        buffer->m.planes->length < 2048)
      return true;
    return false;
  }

  auto inserted =
      queued_buffers_.emplace(buffer->index, std::move(video_frame));
  return true;
}

}  //  namespace mcil
