// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fourcc.h"

#include "base/log.h"

namespace mcil {

Fourcc::Fourcc(Fourcc::Value fourcc) : value_(fourcc) {}
Fourcc::~Fourcc() = default;
Fourcc& Fourcc::operator=(const Fourcc& other) = default;

// static
Fourcc Fourcc::FromUint32(uint32_t fourcc) {
  switch (fourcc) {
    case AR24:
    case AB24:
    case XR24:
    case XB24:
    case RGB4:
    case YU12:
    case YV12:
    case YM12:
    case YM21:
    case YUYV:
    case NV12:
    case NV21:
    case NM12:
    case NM21:
    case YM16:
    case MT21:
    case MM21:
    case P010:
      return Fourcc(static_cast<Value>(fourcc));
    default:
      break;
  }
  MCIL_INFO_PRINT(": Unmapped fourcc: %d", fourcc);
  return Fourcc(NONE);
}

// static
Optional<Fourcc> Fourcc::FromVideoPixelFormat(
    VideoPixelFormat pixel_format, bool single_planar) {
  if (single_planar) {
    switch (pixel_format) {
      case PIXEL_FORMAT_ARGB:
        return Fourcc(AR24);
      case PIXEL_FORMAT_ABGR:
        return Fourcc(AB24);
      case PIXEL_FORMAT_XRGB:
        return Fourcc(XR24);
      case PIXEL_FORMAT_XBGR:
        return Fourcc(XB24);
      case PIXEL_FORMAT_BGRA:
        return Fourcc(RGB4);
      case PIXEL_FORMAT_I420:
        return Fourcc(YU12);
      case PIXEL_FORMAT_YV12:
        return Fourcc(YV12);
      case PIXEL_FORMAT_YUY2:
        return Fourcc(YUYV);
      case PIXEL_FORMAT_NV12:
        return Fourcc(NV12);
      case PIXEL_FORMAT_NV21:
        return Fourcc(NV21);
      case PIXEL_FORMAT_P016LE:
        return Fourcc(P010);
      default:
        break;
    }
  } else {
    switch (pixel_format) {
      case PIXEL_FORMAT_I420:
        return Fourcc(YM12);
      case PIXEL_FORMAT_YV12:
        return Fourcc(YM21);
      case PIXEL_FORMAT_NV12:
        return Fourcc(NM12);
      case PIXEL_FORMAT_I422:
        return Fourcc(YM16);
      case PIXEL_FORMAT_NV21:
        return Fourcc(NM21);
      default:
        break;
    }
  }

  MCIL_INFO_PRINT(": Unmapped %d for %s", pixel_format,
                  (single_planar ? "single-planar" : "multi-planar"));
  return Fourcc(NONE);
}

Optional<Fourcc> Fourcc::FromV4L2PixFmt(uint32_t v4l2_pix_fmt) {
  return FromUint32(v4l2_pix_fmt);
}

uint32_t Fourcc::ToV4L2PixFmt() const {
  return static_cast<uint32_t>(value_);
}

VideoPixelFormat Fourcc::ToVideoPixelFormat() const {
  switch (value_) {
    case AR24:
      return PIXEL_FORMAT_ARGB;
    case AB24:
      return PIXEL_FORMAT_ABGR;
    case XR24:
      return PIXEL_FORMAT_XRGB;
    case XB24:
      return PIXEL_FORMAT_XBGR;
    case RGB4:
      return PIXEL_FORMAT_BGRA;
    case YU12:
    case YM12:
      return PIXEL_FORMAT_I420;
    case YV12:
    case YM21:
      return PIXEL_FORMAT_YV12;
    case YUYV:
      return PIXEL_FORMAT_YUY2;
    case NV12:
    case NM12:
      return PIXEL_FORMAT_NV12;
    case NV21:
    case NM21:
      return PIXEL_FORMAT_NV21;
    case YM16:
      return PIXEL_FORMAT_I422;
    case MT21:
    case MM21:
      return PIXEL_FORMAT_NV12;
    case P010:
      return PIXEL_FORMAT_P016LE;
  }
  MCIL_INFO_PRINT(": Unmapped fourcc: %s", ToString().c_str());
  return PIXEL_FORMAT_UNKNOWN;
}

Fourcc Fourcc::ToSinglePlanar() const {
  switch (value_) {
    case AR24:
    case AB24:
    case XR24:
    case XB24:
    case RGB4:
    case YU12:
    case YV12:
    case YUYV:
    case NV12:
    case NV21:
    case P010:
      return Fourcc(value_);
    case YM12:
      return Fourcc(YU12);
    case YM21:
      return Fourcc(YV12);
    case NM12:
      return Fourcc(NV12);
    case NM21:
      return Fourcc(NV21);
    default:
      break;
  }
  return Fourcc(NONE);
}

bool operator!=(const Fourcc& lhs, const Fourcc& rhs) {
  return !(lhs == rhs);
}

bool Fourcc::IsMultiPlanar() const {
  switch (value_) {
    case AR24:
    case AB24:
    case XR24:
    case XB24:
    case RGB4:
    case YU12:
    case YV12:
    case YUYV:
    case NV12:
    case NV21:
    case P010:
      return false;
    case YM12:
    case YM21:
    case NM12:
    case NM21:
    case YM16:
    case MT21:
    case MM21:
      return true;
    default:
      break;
  }
  return false;
}

std::string Fourcc::ToString() const {
  return FourccToString(static_cast<uint32_t>(value_));
}

}  // namespace mcil
