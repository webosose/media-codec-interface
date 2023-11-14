// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v4l2_buffers.h"

#include <memory>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "base/log.h"
#include "v4l2/v4l2_device.h"
#include "v4l2/v4l2_queue.h"

namespace mcil {

#if !defined(PLATFORM_EXTENSION)
std::unique_ptr<V4L2Buffer> V4L2Buffer::Create(scoped_refptr<V4L2Device> device,
                                               enum v4l2_buf_type buffer_type,
                                               enum v4l2_memory memory,
                                               const struct v4l2_format& format,
                                               size_t buffer_id) {
  std::unique_ptr<V4L2Buffer> buffer(
      new V4L2Buffer(device, buffer_type, memory, format, buffer_id));
  return buffer;
}
#endif

V4L2Buffer::V4L2Buffer(scoped_refptr<V4L2Device> device,
                       enum v4l2_buf_type buffer_type,
                       enum v4l2_memory memory,
                       const struct v4l2_format& format,
                       size_t buffer_id)
    : device_(std::move(device)), format_(format), buffer_type_(buffer_type) {
  memset(&v4l2_buffer_, 0, sizeof(v4l2_buffer_));
  memset(v4l2_planes_, 0, sizeof(v4l2_planes_));
  v4l2_buffer_.m.planes = v4l2_planes_;
  // Just in case we got more planes than we want.
  v4l2_buffer_.length =
      std::min(static_cast<size_t>(format.fmt.pix_mp.num_planes),
               ARRAY_SIZE(v4l2_planes_));
  v4l2_buffer_.index = buffer_id;
  v4l2_buffer_.type = buffer_type;
  v4l2_buffer_.memory = memory;
  plane_mappings_.resize(v4l2_buffer_.length);

  MCIL_DEBUG_PRINT(": %s index= %d, length=%d",
      (buffer_type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ? "input" : "output"),
      v4l2_buffer_.index, v4l2_buffer_.length);
}

V4L2Buffer::~V4L2Buffer() noexcept(false) {
  MCIL_DEBUG_PRINT(": %s index= %d",
      (buffer_type_ == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ? "input" : "output"),
      v4l2_buffer_.index);

  if (v4l2_buffer_.memory == V4L2_MEMORY_MMAP) {
    for (size_t i = 0; i < plane_mappings_.size(); i++)
      if (plane_mappings_[i] != nullptr)
        device_->Munmap(plane_mappings_[i], v4l2_buffer_.m.planes[i].length);
  }
}

size_t V4L2Buffer::BufferIndex() {
  return v4l2_buffer_.index;
}

scoped_refptr<VideoFrame> V4L2Buffer::GetVideoFrame() {
  if (!video_frame_)
    video_frame_ = CreateVideoFrame();

  return video_frame_;
}

void* V4L2Buffer::GetPlaneBuffer(const size_t plane) {
  if (plane >= plane_mappings_.size()) {
    MCIL_ERROR_PRINT(": Invalid plane");
    return nullptr;
  }

  void* p = plane_mappings_[plane];
  if (p)
    return p;

  if (v4l2_buffer_.memory != V4L2_MEMORY_MMAP) {
    MCIL_ERROR_PRINT(": Cannot create mapping on non-MMAP buffer");
    return nullptr;
  }

  p = device_->Mmap(nullptr, v4l2_buffer_.m.planes[plane].length,
                    PROT_READ | PROT_WRITE, MAP_SHARED,
                    v4l2_buffer_.m.planes[plane].m.mem_offset);
  if (p == MAP_FAILED) {
    MCIL_ERROR_PRINT(": mmap() failed:");
    return nullptr;
  }

  plane_mappings_[plane] = p;
  return p;
}

bool V4L2Buffer::Query() {
  if (device_->Ioctl(VIDIOC_QUERYBUF, &v4l2_buffer_)) {
    MCIL_ERROR_PRINT(": VIDIOC_QUERYBUF failed");
    return false;
  }

  return true;
}

int64_t V4L2Buffer::GetBufferPTS() {
  return timestamp_;
}

void V4L2Buffer::SetBufferPTS(int64_t timestamp) {
  timestamp_ = timestamp;
}

scoped_refptr<VideoFrame> V4L2Buffer::CreateVideoFrame() {
  scoped_refptr<VideoFrame> video_frame =
      V4L2Device::VideoFrameFromV4L2Format(format_);

  video_frame->dmabuf_fds = device_->GetDmabufsForV4L2Buffer(
      v4l2_buffer_.index, v4l2_buffer_.length,
      static_cast<enum v4l2_buf_type>(v4l2_buffer_.type));
  if (video_frame->dmabuf_fds.empty()) {
    MCIL_ERROR_PRINT(": Failed to get DMABUFs of V4L2 buffer");
    return nullptr;
  }

  while (video_frame->dmabuf_fds.size() != video_frame->color_planes.size()) {
    int32_t duped_fd = -1;
    if (video_frame->dmabuf_fds.back() != 0) {
      duped_fd = HANDLE_EINTR(dup(video_frame->dmabuf_fds.back()));
      if (duped_fd == -1) {
        MCIL_ERROR_PRINT(": Failed duplicating dmabuf fd");
        return nullptr;
      }
    }
    video_frame->dmabuf_fds.emplace_back(duped_fd);
  }

  for (size_t i = 0; i < video_frame->color_planes.size(); ++i) {
    MCIL_DEBUG_PRINT(", stride: %d, offset: %lu, size: %lu",
                     video_frame->color_planes[i].stride,
                     video_frame->color_planes[i].offset,
                     video_frame->color_planes[i].size);
  }
  return video_frame;
}

/* V4L2BuffersList */
void V4L2BuffersList::ReturnBuffer(size_t buffer_id) {
  std::lock_guard<std::mutex> lock(lock_);
  free_buffers_.emplace(buffer_id);
}

Optional<size_t> V4L2BuffersList::GetFreeBuffer() {
  std::lock_guard<std::mutex> lock(lock_);

  auto iter = free_buffers_.begin();
  if (iter == free_buffers_.end()) {
    return nullopt;
  }

  size_t buffer_id = *iter;
  free_buffers_.erase(iter);
  return buffer_id;
}

size_t V4L2BuffersList::GetSize() const {
  std::lock_guard<std::mutex> lock(lock_);
  return free_buffers_.size();
}

/* V4L2BufferRefBase */
V4L2BufferRefBase::V4L2BufferRefBase(const struct v4l2_buffer& v4l2_buffer,
                                     V4L2Queue* queue)
  : queue_(std::move(queue)), return_to_(queue_->free_buffers_) {
  memset(&v4l2_buffer_, 0, sizeof(v4l2_buffer_));
  memset(v4l2_planes_, 0, sizeof(v4l2_planes_));

  memcpy(&v4l2_buffer_, &v4l2_buffer, sizeof(v4l2_buffer_));
  memcpy(v4l2_planes_, v4l2_buffer.m.planes,
         sizeof(struct v4l2_plane) * v4l2_buffer.length);
  v4l2_buffer_.m.planes = v4l2_planes_;
}

V4L2BufferRefBase::~V4L2BufferRefBase() noexcept(false) {
  if (!queued_)
    return_to_->ReturnBuffer(BufferIndex());
}

void* V4L2BufferRefBase::GetPlaneBuffer(const size_t plane) {
  if (!queue_) {
    MCIL_DEBUG_PRINT(": Failed: Queue is not set yet.");
    return nullptr;
  }

  return queue_->buffers_[BufferIndex()]->GetPlaneBuffer(plane);
}

bool V4L2BufferRefBase::QueueBuffer(scoped_refptr<VideoFrame> video_frame) {
  if (!queue_) {
    MCIL_DEBUG_PRINT(": Failed: Queue is not set yet.");
    return false;
  }

  queued_ = queue_->QueueBuffer(&v4l2_buffer_, std::move(video_frame));
  return queued_;
}

scoped_refptr<VideoFrame> V4L2BufferRefBase::GetVideoFrame() {
  static const scoped_refptr<VideoFrame> null_videoframe;
  if (!queue_) {
    MCIL_DEBUG_PRINT(": Failed: Queue is not set yet.");
    return null_videoframe;
  }
  return queue_->buffers_[BufferIndex()]->GetVideoFrame();
}

int64_t V4L2BufferRefBase::GetBufferPTS() {
  return queue_->buffers_[BufferIndex()]->GetBufferPTS();
}

void V4L2BufferRefBase::SetBufferPTS(int64_t timestamp) {
  queue_->buffers_[BufferIndex()]->SetBufferPTS(timestamp);
}

/* V4L2ReadableBuffer */
V4L2ReadableBuffer::V4L2ReadableBuffer(const struct v4l2_buffer& v4l2_buffer,
                                       V4L2Queue* queue,
                                       scoped_refptr<VideoFrame> video_frame)
  : buffer_data_(
        std::make_unique<V4L2BufferRefBase>(v4l2_buffer, std::move(queue))),
    video_frame_(std::move(video_frame)) {
}

const void* V4L2ReadableBuffer::GetPlaneBuffer(const size_t plane) const {
  return buffer_data_->GetPlaneBuffer(plane);
}

bool V4L2ReadableBuffer::IsLast() const {
  return buffer_data_->v4l2_buffer_.flags & V4L2_BUF_FLAG_LAST;
}

size_t V4L2ReadableBuffer::GetBytesUsed(const size_t plane) const {
  if (plane >= PlanesCount()) {
    return 0;
  }
  return buffer_data_->v4l2_planes_[plane].bytesused;
}

struct timeval V4L2ReadableBuffer::GetTimeStamp() const {
  return buffer_data_->v4l2_buffer_.timestamp;
}

size_t V4L2ReadableBuffer::BufferIndex() const {
  return buffer_data_->v4l2_buffer_.index;
}

size_t V4L2ReadableBuffer::GetDataOffset(size_t plane) const {
  if (plane >= PlanesCount()) {
    return 0;
  }

  return buffer_data_->v4l2_planes_[plane].data_offset;
}

bool V4L2ReadableBuffer::IsKeyframe() const {
    return buffer_data_->v4l2_buffer_.flags & V4L2_BUF_FLAG_KEYFRAME;
}

size_t V4L2ReadableBuffer::PlanesCount() const {
  return buffer_data_->v4l2_buffer_.length;
}

void V4L2ReadableBuffer::SetFlags(uint32_t flags) {
  buffer_data_->v4l2_buffer_.flags = flags;
}

uint32_t V4L2ReadableBuffer::GetFlags() const {
  return buffer_data_->v4l2_buffer_.flags;
}

/* V4L2WritableBufferRef */
V4L2WritableBufferRef::V4L2WritableBufferRef(
    const struct v4l2_buffer& v4l2_buffer, V4L2Queue* queue)
  : buffer_data_(
        std::make_unique<V4L2BufferRefBase>(v4l2_buffer, queue)) {
}

V4L2WritableBufferRef::V4L2WritableBufferRef(V4L2WritableBufferRef&& other)
    : buffer_data_(std::move(other.buffer_data_)) {
}

V4L2WritableBufferRef& V4L2WritableBufferRef::operator=(
    V4L2WritableBufferRef&& other) {
  if (this == &other)
    return *this;

  buffer_data_ = std::move(other.buffer_data_);
  return *this;
}

void* V4L2WritableBufferRef::GetPlaneBuffer(const size_t plane) {
  return buffer_data_->GetPlaneBuffer(plane);
}

size_t V4L2WritableBufferRef::GetBufferSize(const size_t plane) const {
  return buffer_data_->v4l2_buffer_.m.planes[plane].length;
}

void V4L2WritableBufferRef::SetBufferSize(const size_t plane,
                                          const size_t length) {
  buffer_data_->v4l2_buffer_.m.planes[plane].length = length;
}

size_t V4L2WritableBufferRef::GetBytesUsed(const size_t plane) const {
  if (plane >= PlanesCount()) {
    return 0;
  }
  return buffer_data_->v4l2_buffer_.m.planes[plane].bytesused;
}

void V4L2WritableBufferRef::SetBytesUsed(const size_t plane,
                                         const size_t bytes_used) {
  if (bytes_used > GetBufferSize(plane)) {
    MCIL_DEBUG_PRINT(": Set bytes used(%lu), larger than plane size(%lu)",
                     bytes_used, GetBufferSize(plane));
    return;
  }

  buffer_data_->v4l2_buffer_.m.planes[plane].bytesused = bytes_used;
}

struct timeval V4L2WritableBufferRef::GetTimeStamp() const {
  return buffer_data_->v4l2_buffer_.timestamp;
}

void V4L2WritableBufferRef::SetTimeStamp(const struct timeval& timestamp) {
  buffer_data_->v4l2_buffer_.timestamp = timestamp;
}

size_t V4L2WritableBufferRef::BufferIndex() const {
  return buffer_data_->v4l2_buffer_.index;
}

scoped_refptr<VideoFrame> V4L2WritableBufferRef::GetVideoFrame() {
  return buffer_data_->GetVideoFrame();
}

bool V4L2WritableBufferRef::QueueMMap() && {
  V4L2WritableBufferRef self(std::move(*this));

  if (self.Memory() != V4L2_MEMORY_MMAP) {
    MCIL_ERROR_PRINT(" Called on invalid buffer type!");
    return false;
  }

  return std::move(self).DoQueue(nullptr);
}

bool V4L2WritableBufferRef::QueueUserPtr() && {
  V4L2WritableBufferRef self(std::move(*this));

  if (self.Memory() != V4L2_MEMORY_USERPTR) {
    MCIL_ERROR_PRINT(" Called on invalid buffer type!");
    return false;
  }

  return std::move(self).DoQueue(nullptr);
}

bool V4L2WritableBufferRef::QueueUserPtr(const std::vector<void*>& ptrs) && {
  V4L2WritableBufferRef self(std::move(*this));

  if (self.Memory() != V4L2_MEMORY_USERPTR) {
    MCIL_ERROR_PRINT(" Called on invalid buffer type!");
    return false;
  }

  if (ptrs.size() != self.PlanesCount()) {
    MCIL_ERROR_PRINT(" Provided [%ld] pointers while we require [%d]",
                     ptrs.size(), self.buffer_data_->v4l2_buffer_.length);
    return false;
  }

  for (size_t i = 0; i < ptrs.size(); i++) {
    self.buffer_data_->v4l2_buffer_.m.planes[i].m.userptr =
        reinterpret_cast<unsigned long>(ptrs[i]);
  }

  return std::move(self).DoQueue(nullptr);
}

size_t V4L2WritableBufferRef::PlanesCount() const {
  return buffer_data_->v4l2_buffer_.length;
}

enum v4l2_memory V4L2WritableBufferRef::Memory() const {
  return static_cast<enum v4l2_memory>(buffer_data_->v4l2_buffer_.memory);
}

void V4L2WritableBufferRef::SetFlags(uint32_t flags) {
  buffer_data_->v4l2_buffer_.flags = flags;
}

int64_t V4L2WritableBufferRef::GetBufferPTS() {
  return buffer_data_->GetBufferPTS();
}

void V4L2WritableBufferRef::SetBufferPTS(int64_t timestamp) {
  buffer_data_->SetBufferPTS(timestamp);
}

int32_t V4L2WritableBufferRef::GetBufferId() {
  return  buffer_data_->buffer_id_;
}

void V4L2WritableBufferRef::SetBufferId(int32_t buffer_id) {
  buffer_data_->buffer_id_ = buffer_id;
}

bool V4L2WritableBufferRef::DoQueue(scoped_refptr<VideoFrame> video_frame) && {
  bool queued = buffer_data_->QueueBuffer(std::move(video_frame));
  buffer_data_.reset();
  return queued;
}

}  //  namespace mcil
