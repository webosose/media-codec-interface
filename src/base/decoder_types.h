// Copyright (c) 2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SRC_BASE_DECODER_TYPES_H_
#define SRC_BASE_DECODER_TYPES_H_

#include <chrono>

#include "codec_types.h"
#include "optional.h"
#include "ref_counted.h"

namespace mcil {

typedef std::chrono::_V2::system_clock::time_point 	ChronoTime;

// Same as the enum Error defined in upstream Chromium file
// media/video/video_decode_accelerator.h
enum DecoderError {
  ILLEGAL_STATE = 1,
  INVALID_ARGUMENT,
  UNREADABLE_INPUT,
  PLATFORM_FAILURE,
  ERROR_MAX = PLATFORM_FAILURE,
};

// Same as the enum State defined in upstream Chromium file
// media/gpu/v4l2/v4l2_video_decode_accelerator.h
enum DecoderState {
  kUninitialized = 0,
  kInitialized,
  kDecoding,
  kResetting,
  kChangingResolution,
  kAwaitingPictureBuffers,
  kDecoderError,
  kDestroying,
  kDecoderStateMax = kDestroying,
};

// Same as the enum OutputMode defined in upstream Chromium file
// media/video/video_decode_accelerator.h
enum OutputMode {
  OUTPUT_ALLOCATE,
  OUTPUT_IMPORT,
};

// Same as the enum BufferId defined in upstream Chromium file
// media/video/video_decode_accelerator.h
enum BufferId {
  kFlushBufferId = -2  // Buffer id for flush buffer, queued by FlushTask().
};

enum QueueType {
  INVALID_QUEUE,
  INPUT_QUEUE,
  OUTPUT_QUEUE,
};

enum PostTaskType {
  INVALID_DEVICE_POST_TASK,
  ENQUEUE_INPUT_BUFFER_TASK,
  ENQUEUE_OUTPUT_BUFFER_TASK,
  DEQUEUE_INPUT_BUFFER_TASK,
  DEQUEUE_OUTPUT_BUFFER_TASK,
  DECODER_DEVICE_READ_TASK,
  DECODER_DEVICE_WRITE_TASK,
  DISPLAY_DEVICE_READ_TASK,
  DISPLAY_DEVICE_WRITE_TASK,
  SCALER_DEVICE_READ_TASK,
};

/* Decoder configure data structure */
class DecoderConfig {
 public:
  DecoderConfig() = default;
  ~DecoderConfig() = default;

  uint32_t frameWidth;
  uint32_t frameHeight;
  VideoCodecType codecType;
  VideoCodecProfile profile;
  OutputMode outputMode;
};

class ColorPlane {
 public:
  ColorPlane() = default;
  ColorPlane(int32_t stride, size_t offset, size_t size);
  ColorPlane(const ColorPlane& other);
  ColorPlane& operator=(const ColorPlane& other);
  ~ColorPlane() = default;

  bool operator==(const ColorPlane& rhs) const;
  bool operator!=(const ColorPlane& rhs) const;

  int32_t stride = 0;
  size_t offset = 0;
  size_t size = 0;
};

class VideoFrame : public RefCounted<VideoFrame> {
 public:
  static size_t NumPlanes(VideoPixelFormat video_format);
  static  scoped_refptr<VideoFrame> Create(const mcil::Size& size);

  VideoPixelFormat format;
  Size size;
  size_t num_buffers;
  std::vector<ColorPlane> color_planes;
  std::vector<uint32_t> dmabuf_fds;

 private:
  VideoFrame(const mcil::Size& size);
  ~VideoFrame();
  friend class mcil::RefCounted<VideoFrame>;
};

class ReadableBuffer : public RefCounted<ReadableBuffer> {
 public:
  ReadableBuffer() = default;
  virtual ~ReadableBuffer() = default;

  virtual bool IsLast() const { return false; }
  virtual size_t GetBytesUsed(size_t plane) const { return 0; }
  virtual size_t BufferIndex() const { return 0; }
  virtual struct timeval GetTimeStamp() const;

 private:
  friend class mcil::RefCounted<ReadableBuffer>;
};

using ReadableBufferRef = scoped_refptr<ReadableBuffer>;

class WritableBufferRef {
 public:
  WritableBufferRef(WritableBufferRef&&) = default;
  WritableBufferRef() = default;
  WritableBufferRef& operator=(WritableBufferRef&& other);
  virtual ~WritableBufferRef() = default;

  virtual void* GetInputBuffer(const size_t size) { return nullptr; }
  virtual bool QueueBuffer(scoped_refptr<VideoFrame> video_frame);

  virtual size_t GetBufferSize(const size_t plane) const { return 0; }
  virtual void SetBufferSize(const size_t plane, const size_t length) {}

  virtual size_t GetBytesUsed(const size_t plane) const { return 0; }
  virtual void SetBytesUsed(const size_t plane, const size_t bytes_used) {}

  virtual struct timeval GetTimeStamp() const;
  virtual void SetTimeStamp(const struct timeval& timestamp) {}

  virtual size_t BufferIndex() const { return 0; }
  virtual scoped_refptr<VideoFrame> GetVideoFrame() { return nullptr; }
};

class OutputRecord {
 public:
  OutputRecord() = default;
  OutputRecord(OutputRecord&&) = default;
  ~OutputRecord() = default;

  void* egl_image = nullptr;
  int32_t picture_id = 0;
  uint32_t texture_id = 0;
  bool cleared = false;
};

}  // namespace mcil

#endif  // SRC_BASE_DECODER_TYPES_H_
