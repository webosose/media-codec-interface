// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines the V4L2Device interface which is used by the
// V4L2DecodeAccelerator class to delegate/pass the device specific
// handling of any of the functionalities.

#ifndef SRC_IMPL_V4L2_V4L2_BUFFERS_H_
#define SRC_IMPL_V4L2_V4L2_BUFFERS_H_

#include "v4l2/v4l2_utils.h"

namespace mcil {

class V4L2Device;
class V4L2Queue;

/* V4L2Buffer */
class V4L2Buffer {
 public:
  static std::unique_ptr<V4L2Buffer> Create(scoped_refptr<V4L2Device> device,
                                            enum v4l2_buf_type buffer_type,
                                            enum v4l2_memory memory,
                                            const struct v4l2_format& format,
                                            size_t buffer_id);
  virtual ~V4L2Buffer() noexcept(false);

  virtual size_t BufferIndex();
  virtual scoped_refptr<VideoFrame> GetVideoFrame();
  virtual void* GetPlaneBuffer(const size_t plane);
  virtual const struct v4l2_buffer& v4l2_buffer() const { return v4l2_buffer_; }
  virtual bool Query();

  virtual int64_t GetBufferPTS();
  virtual void SetBufferPTS(int64_t buffer_pts);

 protected:
  V4L2Buffer(scoped_refptr<V4L2Device> device,
             enum v4l2_buf_type buffer_type,
             enum v4l2_memory memory,
             const struct v4l2_format& format,
             size_t buffer_id);

  virtual scoped_refptr<VideoFrame> CreateVideoFrame();

  scoped_refptr<V4L2Device> device_;
  std::vector<void*> plane_mappings_;

  int64_t timestamp_ = 0;

  struct v4l2_buffer v4l2_buffer_;
  enum v4l2_buf_type buffer_type_;

  struct v4l2_plane v4l2_planes_[VIDEO_MAX_PLANES];

  struct v4l2_format format_;
  scoped_refptr<VideoFrame> video_frame_;
}; /* V4L2Buffer */

/* V4L2BuffersList */
class V4L2BuffersList : public RefCounted<V4L2BuffersList> {
 public:
  V4L2BuffersList() = default;

  void ReturnBuffer(size_t buffer_id);
  Optional<size_t> GetFreeBuffer();
  size_t GetSize() const;

 private:
  friend class RefCounted<V4L2BuffersList>;
  ~V4L2BuffersList() = default;

  mutable std::mutex lock_;
  std::set<size_t> free_buffers_ GUARDED_BY(lock_);
}; /* V4L2BuffersList */

/* V4L2BufferRefBase */
class V4L2BufferRefBase {
 public:
  V4L2BufferRefBase(const struct v4l2_buffer& v4l2_buffer,
                    V4L2Queue* queue);
  ~V4L2BufferRefBase() noexcept(false);

  void* GetPlaneBuffer(const size_t plane);
  bool QueueBuffer(scoped_refptr<VideoFrame> video_frame);
  scoped_refptr<VideoFrame> GetVideoFrame();

  int64_t GetBufferPTS();
  void SetBufferPTS(int64_t buffer_pts);

  struct v4l2_buffer v4l2_buffer_;
  struct v4l2_plane v4l2_planes_[VIDEO_MAX_PLANES];

  int32_t buffer_id_ = -1;

 private:
  friend class V4L2ReadableBuffer;
  friend class V4L2WritableBuffer;

  size_t BufferIndex() const { return v4l2_buffer_.index; }

  V4L2Queue* queue_;

  scoped_refptr<V4L2BuffersList> return_to_;
  bool queued_ = false;
}; /* V4L2BufferRefBase */

/* V4L2ReadableBuffer */
class V4L2ReadableBuffer : public ReadableBuffer {
 public:
  const void* GetPlaneBuffer(const size_t plane) const override;
  bool IsLast() const override;
  size_t GetBytesUsed(size_t plane) const override;
  struct timeval GetTimeStamp() const override;
  size_t BufferIndex() const override;

  size_t GetDataOffset(size_t plane) const override;
  bool IsKeyframe() const override;

  size_t PlanesCount() const override;
  void SetFlags(uint32_t flags) override;
  uint32_t GetFlags() const override;

 private:
  V4L2ReadableBuffer(const struct v4l2_buffer& v4l2_buffer,
                     V4L2Queue* queue,
                     scoped_refptr<VideoFrame> video_frame);
  ~V4L2ReadableBuffer() = default;

  friend class V4L2BufferRefFactory;

  std::unique_ptr<V4L2BufferRefBase> buffer_data_;
  scoped_refptr<VideoFrame> video_frame_;
}; /* V4L2ReadableBuffer */

/* V4L2WritableBufferRef */
class V4L2WritableBufferRef : public WritableBufferRef {
 public:
  V4L2WritableBufferRef(V4L2WritableBufferRef&& other);
  V4L2WritableBufferRef() = delete;
  V4L2WritableBufferRef& operator=(V4L2WritableBufferRef&& other);
  ~V4L2WritableBufferRef() override {}

  /* WritableBufferRef override */
  void* GetPlaneBuffer(const size_t plane) override;
  size_t GetBufferSize(const size_t plane) const override;
  void SetBufferSize(const size_t plane, const size_t length) override;
  size_t GetBytesUsed(const size_t plane) const override;
  void SetBytesUsed(const size_t plane, const size_t bytes_used) override;
  struct timeval GetTimeStamp() const override;
  void SetTimeStamp(const struct timeval& timestamp) override;
  size_t BufferIndex() const override;
  scoped_refptr<VideoFrame> GetVideoFrame() override;

  bool QueueMMap() &&;
  bool QueueUserPtr() &&;
  bool QueueUserPtr(const std::vector<void*>& ptrs) &&;
  size_t PlanesCount() const;
  enum v4l2_memory Memory() const;
  void SetFlags(uint32_t flags);

  int32_t GetBufferId();
  void SetBufferId(int32_t buffer_id);

  int64_t GetBufferPTS();
  void SetBufferPTS(int64_t buffer_pts);

 private:
  V4L2WritableBufferRef(const struct v4l2_buffer& v4l2_buffer,
                        V4L2Queue* queue);

  bool DoQueue(scoped_refptr<VideoFrame> video_frame) &&;

  friend class V4L2BufferRefFactory;

  std::unique_ptr<V4L2BufferRefBase> buffer_data_;
}; /* V4L2WritableBufferRef */

}  //  namespace mcil

#endif  // SRC_IMPL_V4L2_V4L2_BUFFERS_H_
