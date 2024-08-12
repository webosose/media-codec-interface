// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_BASE_VIDEO_FRAME_H_
#define SRC_BASE_VIDEO_FRAME_H_

#include "codec_types.h"

namespace mcil {

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
  enum {
    kYPlane = 0,
    kARGBPlane = kYPlane,
    kUPlane = 1,
    kUVPlane = kUPlane,
    kVPlane = 2,
    kAPlane = 3,
    kMaxPlanes = 4,
  };

  static size_t NumPlanes(VideoPixelFormat video_format);
  static int32_t BytesPerElement(VideoPixelFormat format, size_t plane);
  static Size SampleSize(VideoPixelFormat format, size_t plane);
  static int32_t PlaneHorizontalBitsPerPixel(VideoPixelFormat format,
                                         size_t plane);
  static int32_t PlaneBitsPerPixel(VideoPixelFormat format, size_t plane);
  static size_t AllocationSize(VideoPixelFormat format,
                               const Size& coded_size);
  static Size PlaneSize(VideoPixelFormat format,
                        size_t plane,
                        const Size& coded_size);
  static Size PlaneSizeInSamples(VideoPixelFormat format,
                                 size_t plane,
                                 const Size& coded_size);

  static  scoped_refptr<VideoFrame> Create(const Size& size);

  VideoPixelFormat format;
  Size coded_size;

  std::vector<ColorPlane> color_planes;
  std::vector<int32_t> dmabuf_fds;

  struct timeval timestamp;
  bool is_multi_planar;

  const uint8_t* data[kMaxPlanes] = {};

 private:
  friend class RefCounted<VideoFrame>;

  VideoFrame(const Size& coded_size);
  ~VideoFrame() = default;
};

}  //  namespace mcil

#endif  // SRC_BASE_VIDEO_FRAME_H_
