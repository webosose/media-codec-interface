// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_frame.h"

namespace mcil {

namespace {

bool RequiresEvenSizeAllocation(VideoPixelFormat format) {
  switch (format) {
    case PIXEL_FORMAT_ARGB:
    case PIXEL_FORMAT_XRGB:
    case PIXEL_FORMAT_RGB24:
    case PIXEL_FORMAT_Y16:
    case PIXEL_FORMAT_ABGR:
    case PIXEL_FORMAT_XBGR:
    case PIXEL_FORMAT_XR30:
    case PIXEL_FORMAT_XB30:
    case PIXEL_FORMAT_BGRA:
    case PIXEL_FORMAT_RGBAF16:
      return false;
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
    case PIXEL_FORMAT_I420:
    case PIXEL_FORMAT_MJPEG:
    case PIXEL_FORMAT_YUY2:
    case PIXEL_FORMAT_YV12:
    case PIXEL_FORMAT_I422:
    case PIXEL_FORMAT_I444:
    case PIXEL_FORMAT_YUV420P9:
    case PIXEL_FORMAT_YUV422P9:
    case PIXEL_FORMAT_YUV444P9:
    case PIXEL_FORMAT_YUV420P10:
    case PIXEL_FORMAT_YUV422P10:
    case PIXEL_FORMAT_YUV444P10:
    case PIXEL_FORMAT_YUV420P12:
    case PIXEL_FORMAT_YUV422P12:
    case PIXEL_FORMAT_YUV444P12:
    case PIXEL_FORMAT_I420A:
    case PIXEL_FORMAT_UYVY:
    case PIXEL_FORMAT_P016LE:
      return true;
    case PIXEL_FORMAT_UNKNOWN:
      break;
  }
  return false;
}

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

// static
size_t VideoFrame::NumPlanes(VideoPixelFormat pixel_format) {
  switch (pixel_format) {
    case PIXEL_FORMAT_UYVY:
    case PIXEL_FORMAT_YUY2:
    case PIXEL_FORMAT_ARGB:
    case PIXEL_FORMAT_BGRA:
    case PIXEL_FORMAT_XRGB:
    case PIXEL_FORMAT_RGB24:
    case PIXEL_FORMAT_MJPEG:
    case PIXEL_FORMAT_Y16:
    case PIXEL_FORMAT_ABGR:
    case PIXEL_FORMAT_XBGR:
    case PIXEL_FORMAT_XR30:
    case PIXEL_FORMAT_XB30:
      return 1;
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
    case PIXEL_FORMAT_P016LE:
      return 2;
    case PIXEL_FORMAT_I420:
    case PIXEL_FORMAT_YV12:
    case PIXEL_FORMAT_I422:
    case PIXEL_FORMAT_I444:
    case PIXEL_FORMAT_YUV420P9:
    case PIXEL_FORMAT_YUV422P9:
    case PIXEL_FORMAT_YUV444P9:
    case PIXEL_FORMAT_YUV420P10:
    case PIXEL_FORMAT_YUV422P10:
    case PIXEL_FORMAT_YUV444P10:
    case PIXEL_FORMAT_YUV420P12:
    case PIXEL_FORMAT_YUV422P12:
    case PIXEL_FORMAT_YUV444P12:
      return 3;
    case PIXEL_FORMAT_I420A:
      return 4;
    default:
      break;
  }
  return 0;
}

// static
int VideoFrame::BytesPerElement(VideoPixelFormat format, size_t plane) {
  switch (format) {
    case PIXEL_FORMAT_RGBAF16:
      return 8;
    case PIXEL_FORMAT_ARGB:
    case PIXEL_FORMAT_BGRA:
    case PIXEL_FORMAT_XRGB:
    case PIXEL_FORMAT_ABGR:
    case PIXEL_FORMAT_XBGR:
    case PIXEL_FORMAT_XR30:
    case PIXEL_FORMAT_XB30:
      return 4;
    case PIXEL_FORMAT_RGB24:
      return 3;
    case PIXEL_FORMAT_Y16:
    case PIXEL_FORMAT_UYVY:
    case PIXEL_FORMAT_YUY2:
    case PIXEL_FORMAT_YUV420P9:
    case PIXEL_FORMAT_YUV422P9:
    case PIXEL_FORMAT_YUV444P9:
    case PIXEL_FORMAT_YUV420P10:
    case PIXEL_FORMAT_YUV422P10:
    case PIXEL_FORMAT_YUV444P10:
    case PIXEL_FORMAT_YUV420P12:
    case PIXEL_FORMAT_YUV422P12:
    case PIXEL_FORMAT_YUV444P12:
      return 2;
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21: {
      static const int bytes_per_element[] = {1, 2};
      if (plane < 2)
        return bytes_per_element[plane];

      return 0;
    }
    case PIXEL_FORMAT_P016LE: {
      static const int bytes_per_element[] = {1, 2};
      if (plane < 2)
        return bytes_per_element[plane] * 2;

      return 0;
    }
    case PIXEL_FORMAT_YV12:
    case PIXEL_FORMAT_I420:
    case PIXEL_FORMAT_I422:
    case PIXEL_FORMAT_I420A:
    case PIXEL_FORMAT_I444:
      return 1;
    default:
      break;
  }
  return 0;
}

// static
Size VideoFrame::SampleSize(VideoPixelFormat format, size_t plane) {
  switch (plane) {
    case kYPlane:  // and kARGBPlane:
    case kAPlane:
      return Size(1, 1);

    case kUPlane:  // and kUVPlane:
    case kVPlane:
      switch (format) {
        case PIXEL_FORMAT_I444:
        case PIXEL_FORMAT_YUV444P9:
        case PIXEL_FORMAT_YUV444P10:
        case PIXEL_FORMAT_YUV444P12:
        case PIXEL_FORMAT_Y16:
          return Size(1, 1);

        case PIXEL_FORMAT_I422:
        case PIXEL_FORMAT_YUV422P9:
        case PIXEL_FORMAT_YUV422P10:
        case PIXEL_FORMAT_YUV422P12:
          return Size(2, 1);

        case PIXEL_FORMAT_YV12:
        case PIXEL_FORMAT_I420:
        case PIXEL_FORMAT_I420A:
        case PIXEL_FORMAT_NV12:
        case PIXEL_FORMAT_NV21:
        case PIXEL_FORMAT_YUV420P9:
        case PIXEL_FORMAT_YUV420P10:
        case PIXEL_FORMAT_YUV420P12:
        case PIXEL_FORMAT_P016LE:
          return Size(2, 2);

        default:
          break;
      }
    default:
      break;
  }
  return Size();
}

// static
int VideoFrame::PlaneHorizontalBitsPerPixel(VideoPixelFormat format,
                                            size_t plane) {
  const int bits_per_element = 8 * BytesPerElement(format, plane);
  const int horiz_pixels_per_element = SampleSize(format, plane).width;
  return bits_per_element / horiz_pixels_per_element;
}

// static
int VideoFrame::PlaneBitsPerPixel(VideoPixelFormat format, size_t plane) {
  return PlaneHorizontalBitsPerPixel(format, plane) /
         SampleSize(format, plane).height;
}

// static
size_t VideoFrame::AllocationSize(VideoPixelFormat format,
                                  const Size& coded_size) {
  size_t total = 0;
  for (size_t i = 0; i < NumPlanes(format); ++i)
    total += PlaneSize(format, i, coded_size).GetArea();
  return total;
}

// static
Size VideoFrame::PlaneSize(VideoPixelFormat format,
                           size_t plane,
                           const Size& coded_size) {
  Size size = PlaneSizeInSamples(format, plane, coded_size);
  size.width = (size.width * BytesPerElement(format, plane));
  return size;
}

// static
Size VideoFrame::PlaneSizeInSamples(VideoPixelFormat format,
                                    size_t plane,
                                    const Size& coded_size) {
  int width = coded_size.width;
  int height = coded_size.height;

  if (RequiresEvenSizeAllocation(format)) {
    // Align to power of 2.
    width = (width + 2 - 1) & ~(2 - 1);
    height = (height + 2 - 1) & ~(2 - 1);
  }

  const Size subsample = SampleSize(format, plane);
  return Size(width / subsample.width, height / subsample.height);
}

// static
scoped_refptr<VideoFrame> VideoFrame::Create(const Size& size) {
  return new VideoFrame(size);
}

VideoFrame::VideoFrame(const Size& size)
 :format(PIXEL_FORMAT_UNKNOWN),
  coded_size(size.width, size.height),
  is_multi_planar(false) {
  memset(&timestamp, 0, sizeof(timestamp));
}

}  //  namespace mcil
