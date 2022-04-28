// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_BASE_VIDEO_BUFFERS_H_
#define SRC_BASE_VIDEO_BUFFERS_H_

#include "codec_types.h"
#include "video_frame.h"

namespace mcil {

class ReadableBuffer : public RefCounted<ReadableBuffer> {
 public:
  ReadableBuffer() = default;
  virtual ~ReadableBuffer() = default;

  virtual const void* GetPlaneBuffer(const size_t plane) const;
  virtual bool IsLast() const { return false; }
  virtual size_t GetBytesUsed(size_t plane) const { return 0; }
  virtual size_t BufferIndex() const { return 0; }
  virtual struct timeval GetTimeStamp() const;

 private:
  friend class RefCounted<ReadableBuffer>;
};

using ReadableBufferRef = scoped_refptr<ReadableBuffer>;

class WritableBufferRef {
 public:
  WritableBufferRef(WritableBufferRef&&) = default;
  WritableBufferRef() = default;
  WritableBufferRef& operator=(WritableBufferRef&& other);
  virtual ~WritableBufferRef() = default;

  virtual void* GetPlaneBuffer(const size_t plane) { return nullptr; }
  virtual size_t GetBufferSize(const size_t plane) const { return 0; }
  virtual void SetBufferSize(const size_t plane, const size_t length) {}

  virtual size_t GetBytesUsed(const size_t plane) const { return 0; }
  virtual void SetBytesUsed(const size_t plane, const size_t bytes_used) {}

  virtual struct timeval GetTimeStamp() const;
  virtual void SetTimeStamp(const struct timeval& timestamp) {}

  virtual size_t BufferIndex() const { return 0; }
  virtual scoped_refptr<VideoFrame> GetVideoFrame() { return nullptr; }
};

}  //  namespace mcil

#endif  // SRC_BASE_VIDEO_BUFFER_H_
