// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines the V4L2Device interface which is used by the
// V4L2DecodeAccelerator class to delegate/pass the device specific
// handling of any of the functionalities.

#ifndef SRC_IMPL_V4L2_V4L2_QUEUE_H_
#define SRC_IMPL_V4L2_V4L2_QUEUE_H_

#include "v4l2/v4l2_utils.h"

namespace mcil {

class V4L2Buffer;
class V4L2BuffersList;
class V4L2Device;
class V4L2WritableBufferRef;

/* V4L2Queue */
class V4L2Queue : public RefCounted<V4L2Queue> {
 public:
  static scoped_refptr<V4L2Queue> Create(scoped_refptr<V4L2Device> dev,
                                         enum v4l2_buf_type type,
                                         V4L2BufferDestroyCb destroy_cb);

  virtual Optional<Rect> GetVisibleRect();
  virtual std::pair<Optional<struct v4l2_format>, int> GetFormat();
  virtual Optional<struct v4l2_format> SetFormat(uint32_t fourcc,
                                                 const Size& size,
                                                 size_t buffer_size);

  virtual bool StreamOn();
  virtual bool StreamOff();
  virtual bool IsStreaming() const;

  virtual size_t AllocateBuffers(size_t count, enum v4l2_memory memory);
  virtual bool DeallocateBuffers();

  virtual size_t AllocatedBuffersCount() const;
  virtual size_t FreeBuffersCount() const;
  virtual size_t QueuedBuffersCount() const;

  virtual Optional<V4L2WritableBufferRef> GetFreeBuffer();
  virtual V4L2WritableBufferRef* GetFreeBufferPtr();
  virtual std::pair<bool, ReadableBufferRef> DequeueBuffer();
  virtual bool QueueBuffer(struct v4l2_buffer* v4l2_buffer,
                           scoped_refptr<VideoFrame> video_frame);

 protected:
  friend class RefCounted<V4L2Queue>;
  friend class V4L2QueueFactory;
  friend class V4L2BufferRefBase;

  V4L2Queue(scoped_refptr<V4L2Device> device,
            enum v4l2_buf_type buffer_type,
            V4L2BufferDestroyCb destroy_cb);
  virtual ~V4L2Queue();

  enum v4l2_buf_type buffer_type_;
  scoped_refptr<V4L2Device> device_;

  scoped_refptr<V4L2BuffersList> free_buffers_;
  std::map<size_t, scoped_refptr<VideoFrame>> queued_buffers_;
  std::vector<std::unique_ptr<V4L2Buffer>> buffers_;

  size_t planes_count_ = 0;
  enum v4l2_memory memory_ = V4L2_MEMORY_MMAP;
  bool is_streaming_ = false;

  Optional<struct v4l2_format> current_format_;
}; /* V4L2Queue */

/* V4L2BufferRefFactory */
class V4L2BufferRefFactory {
 public:
  static V4L2WritableBufferRef CreateWritableRef(
      const struct v4l2_buffer& v4l2_buffer, V4L2Queue* queue);

  static V4L2WritableBufferRef* CreateWritableRefPtr(
      const struct v4l2_buffer& v4l2_buffer, V4L2Queue* queue);

  static ReadableBufferRef CreateReadableRef(
      const struct v4l2_buffer& v4l2_buffer,
      V4L2Queue* queue,
      scoped_refptr<VideoFrame> video_frame);
};

}  //  namespace mcil

#endif  // SRC_IMPL_V4L2_V4L2_QUEUE_H_
