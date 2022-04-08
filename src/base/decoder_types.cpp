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

#include "decoder_types.h"

#include <sstream>

namespace mcil {

// static
size_t VideoFrame::NumPlanes(VideoPixelFormat pixel_format) {
  switch (pixel_format) {
    case PIXEL_FMT_UYVY:
    case PIXEL_FMT_YUY2:
    case PIXEL_FMT_ARGB:
    case PIXEL_FMT_BGRA:
    case PIXEL_FMT_XRGB:
    case PIXEL_FMT_RGB24:
    case PIXEL_FMT_MJPEG:
    case PIXEL_FMT_Y16:
    case PIXEL_FMT_ABGR:
    case PIXEL_FMT_XBGR:
    case PIXEL_FMT_XR30:
    case PIXEL_FMT_XB30:
      return 1;
    case PIXEL_FMT_NV12:
    case PIXEL_FMT_NV21:
    case PIXEL_FMT_P016LE:
      return 2;
    case PIXEL_FMT_I420:
    case PIXEL_FMT_YV12:
    case PIXEL_FMT_I422:
    case PIXEL_FMT_I444:
    case PIXEL_FMT_YUV420P9:
    case PIXEL_FMT_YUV422P9:
    case PIXEL_FMT_YUV444P9:
    case PIXEL_FMT_YUV420P10:
    case PIXEL_FMT_YUV422P10:
    case PIXEL_FMT_YUV444P10:
    case PIXEL_FMT_YUV420P12:
    case PIXEL_FMT_YUV422P12:
    case PIXEL_FMT_YUV444P12:
      return 3;
    case PIXEL_FMT_I420A:
      return 4;
    default:
      break;
  }
  return 0;
}

ColorPlane::ColorPlane(int32_t st, size_t os, size_t sz)
  : stride(st), offset(os), size(sz) {}

ColorPlane::ColorPlane(const ColorPlane& other)
  : stride(other.stride), offset(other.offset), size(other.size) {}

ColorPlane& ColorPlane::operator=(const ColorPlane& other)  = default;

bool ColorPlane::operator==(const ColorPlane& rhs) const {
  return stride == rhs.stride && offset == rhs.offset && size == rhs.size;
}

bool ColorPlane::operator!=(const ColorPlane& rhs) const {
  return !(*this == rhs);
}

scoped_refptr<VideoFrame> VideoFrame::Create(const mcil::Size& size) {
  return new VideoFrame(size);
}

VideoFrame::VideoFrame(const mcil::Size& size)
 : coded_size(size.width, size.height) {}

VideoFrame::~VideoFrame() {}

struct timeval ReadableBuffer::GetTimeStamp() const {
  struct timeval timestamp;
  return timestamp;
}

WritableBufferRef& WritableBufferRef::operator=(WritableBufferRef&& other) {
  return *this;
}

bool WritableBufferRef::QueueBuffer(scoped_refptr<VideoFrame> video_frame) {
  return false;
}

struct timeval WritableBufferRef::GetTimeStamp() const {
  struct timeval timestamp;
  return timestamp;
}

}  //  namespace mcil
