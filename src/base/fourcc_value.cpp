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

#include "fourcc_value.h"

#include "base/log.h"

namespace mcil {

FourccValue::FourccValue(FourccValue::Value fourcc) : value_(fourcc) {}
FourccValue::~FourccValue() = default;
FourccValue& FourccValue::operator=(const FourccValue& other) = default;

// static
FourccValue FourccValue::FromUint32(uint32_t fourcc) {
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
      return FourccValue(static_cast<Value>(fourcc));
    default:
      break;
  }
  MCIL_INFO_PRINT("%d %s : Unmapped fourcc: %d", __LINE__, __FUNCTION__, fourcc);
  return FourccValue(NONE);
}

// static
FourccValue FourccValue::FromMCILPixelFormat(
    VideoPixelFormat pixel_format, bool single_planar) {
  if (single_planar) {
    switch (pixel_format) {
      case PIXEL_FMT_ARGB:
        return FourccValue(AR24);
      case PIXEL_FMT_ABGR:
        return FourccValue(AB24);
      case PIXEL_FMT_XRGB:
        return FourccValue(XR24);
      case PIXEL_FMT_XBGR:
        return FourccValue(XB24);
      case PIXEL_FMT_BGRA:
        return FourccValue(RGB4);
      case PIXEL_FMT_I420:
        return FourccValue(YU12);
      case PIXEL_FMT_YV12:
        return FourccValue(YV12);
      case PIXEL_FMT_YUY2:
        return FourccValue(YUYV);
      case PIXEL_FMT_NV12:
        return FourccValue(NV12);
      case PIXEL_FMT_NV21:
        return FourccValue(NV21);
      case PIXEL_FMT_P016LE:
        return FourccValue(P010);
      default:
        break;
    }
  } else {
    switch (pixel_format) {
      case PIXEL_FMT_I420:
        return FourccValue(YM12);
      case PIXEL_FMT_YV12:
        return FourccValue(YM21);
      case PIXEL_FMT_NV12:
        return FourccValue(NM12);
      case PIXEL_FMT_I422:
        return FourccValue(YM16);
      case PIXEL_FMT_NV21:
        return FourccValue(NM21);
      default:
        break;
    }
  }

  MCIL_INFO_PRINT("%d %s : Unmapped %d for %s",
      __LINE__, __FUNCTION__, pixel_format,
      (single_planar ? "single-planar" : "multi-planar"));
  return FourccValue(NONE);
}

Optional<FourccValue> FourccValue::FromV4L2PixFmt(uint32_t v4l2_pix_fmt) {
  return FromUint32(v4l2_pix_fmt);
}

uint32_t FourccValue::ToV4L2PixFmt() const {
  return static_cast<uint32_t>(value_);
}

VideoPixelFormat FourccValue::ToMCILPixelFormat() const {
  switch (value_) {
    case AR24:
      return PIXEL_FMT_ARGB;
    case AB24:
      return PIXEL_FMT_ABGR;
    case XR24:
      return PIXEL_FMT_XRGB;
    case XB24:
      return PIXEL_FMT_XBGR;
    case RGB4:
      return PIXEL_FMT_BGRA;
    case YU12:
    case YM12:
      return PIXEL_FMT_I420;
    case YV12:
    case YM21:
      return PIXEL_FMT_YV12;
    case YUYV:
      return PIXEL_FMT_YUY2;
    case NV12:
    case NM12:
      return PIXEL_FMT_NV12;
    case NV21:
    case NM21:
      return PIXEL_FMT_NV21;
    case YM16:
      return PIXEL_FMT_I422;
    case MT21:
    case MM21:
      return PIXEL_FMT_NV12;
    case P010:
      return PIXEL_FMT_P016LE;
  }
  MCIL_INFO_PRINT("%d %s : Unmapped fourcc: %s",
                  __LINE__, __FUNCTION__, ToString().c_str());
  return PIXEL_FMT_UNKNOWN;
}

FourccValue FourccValue::ToSinglePlanar() const {
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
      return FourccValue(value_);
    case YM12:
      return FourccValue(YU12);
    case YM21:
      return FourccValue(YV12);
    case NM12:
      return FourccValue(NV12);
    case NM21:
      return FourccValue(NV21);
    default:
      break;
  }
  return FourccValue(NONE);
}

bool operator!=(const FourccValue& lhs, const FourccValue& rhs) {
  return !(lhs == rhs);
}

bool FourccValue::IsMultiPlanar() const {
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

std::string FourccValue::ToString() const {
  return mcil::FourccToString(static_cast<uint32_t>(value_));
}

}  // namespace mcil
