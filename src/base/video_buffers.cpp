// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_buffers.h"

namespace mcil {

const void* ReadableBuffer::GetPlaneBuffer(const size_t plane) const {
  return nullptr;
}

struct timeval ReadableBuffer::GetTimeStamp() const {
  struct timeval timestamp;
  return timestamp;
}

WritableBufferRef& WritableBufferRef::operator=(WritableBufferRef&& other) {
  return *this;
}

struct timeval WritableBufferRef::GetTimeStamp() const {
  struct timeval timestamp;
  return timestamp;
}

}  //  namespace mcil
