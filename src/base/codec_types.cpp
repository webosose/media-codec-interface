// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "codec_types.h"

#include <sstream>

namespace mcil {

Size::Size(uint32_t w, uint32_t h)
 : width(w), height(h) {}

void Size::SetSize(uint32_t w, uint32_t h) {
  width = w;
  height = h;
}

Rect::Rect(int32_t x, int32_t y, uint32_t w, uint32_t h)
 :  x(x), y(y), width(w), height(h) {}

Rect::Rect(const Size& size)
 : width(size.width), height(size.height) {}

bool Rect::Contains(const Rect& rect) const {
  return (rect.x >= x && rect.width <= width && rect.y >= y &&
          rect.height <= height);
}

SupportedProfile::SupportedProfile(
    VideoCodecProfile prof, Size min, Size max) {
  profile = prof;
  min_resolution = min;
  max_resolution = max;
}

std::string GetProfileName(VideoCodecProfile profile) {
  switch (profile) {
    case VIDEO_CODEC_PROFILE_UNKNOWN:
      return "unknown";
    case H264PROFILE_BASELINE:
      return "h264 baseline";
    case H264PROFILE_MAIN:
      return "h264 main";
    case H264PROFILE_EXTENDED:
      return "h264 extended";
    case H264PROFILE_HIGH:
      return "h264 high";
    case H264PROFILE_HIGH10PROFILE:
      return "h264 high 10";
    case H264PROFILE_HIGH422PROFILE:
      return "h264 high 4:2:2";
    case H264PROFILE_HIGH444PREDICTIVEPROFILE:
      return "h264 high 4:4:4 predictive";
    case H264PROFILE_SCALABLEBASELINE:
      return "h264 scalable baseline";
    case H264PROFILE_SCALABLEHIGH:
      return "h264 scalable high";
    case H264PROFILE_STEREOHIGH:
      return "h264 stereo high";
    case H264PROFILE_MULTIVIEWHIGH:
      return "h264 multiview high";
    case HEVCPROFILE_MAIN:
      return "hevc main";
    case HEVCPROFILE_MAIN10:
      return "hevc main 10";
    case HEVCPROFILE_MAIN_STILL_PICTURE:
      return "hevc main still-picture";
    case VP8PROFILE_ANY:
      return "vp8";
    case VP9PROFILE_PROFILE0:
      return "vp9 profile0";
    case VP9PROFILE_PROFILE1:
      return "vp9 profile1";
    case VP9PROFILE_PROFILE2:
      return "vp9 profile2";
    case VP9PROFILE_PROFILE3:
      return "vp9 profile3";
    case DOLBYVISION_PROFILE0:
      return "dolby vision profile 0";
    case DOLBYVISION_PROFILE4:
      return "dolby vision profile 4";
    case DOLBYVISION_PROFILE5:
      return "dolby vision profile 5";
    case DOLBYVISION_PROFILE7:
      return "dolby vision profile 7";
    case DOLBYVISION_PROFILE8:
      return "dolby vision profile 8";
    case DOLBYVISION_PROFILE9:
      return "dolby vision profile 9";
    case THEORAPROFILE_ANY:
      return "theora";
    case AV1PROFILE_PROFILE_MAIN:
      return "av1 profile main";
    case AV1PROFILE_PROFILE_HIGH:
      return "av1 profile high";
    case AV1PROFILE_PROFILE_PRO:
      return "av1 profile pro";
  }
  return "Unknown VideoCodecProfile";
}

std::string VideoPixelFormatToString(VideoPixelFormat format) {
  switch (format) {
    case PIXEL_FORMAT_UNKNOWN:
      return "PIXEL_FORMAT_UNKNOWN";
    case PIXEL_FORMAT_I420:
      return "PIXEL_FORMAT_I420";
    case PIXEL_FORMAT_YV12:
      return "PIXEL_FORMAT_YV12";
    case PIXEL_FORMAT_I422:
      return "PIXEL_FORMAT_I422";
    case PIXEL_FORMAT_I420A:
      return "PIXEL_FORMAT_I420A";
    case PIXEL_FORMAT_I444:
      return "PIXEL_FORMAT_I444";
    case PIXEL_FORMAT_NV12:
      return "PIXEL_FORMAT_NV12";
    case PIXEL_FORMAT_NV21:
      return "PIXEL_FORMAT_NV21";
    case PIXEL_FORMAT_UYVY:
      return "PIXEL_FORMAT_UYVY";
    case PIXEL_FORMAT_YUY2:
      return "PIXEL_FORMAT_YUY2";
    case PIXEL_FORMAT_ARGB:
      return "PIXEL_FORMAT_ARGB";
    case PIXEL_FORMAT_XRGB:
      return "PIXEL_FORMAT_XRGB";
    case PIXEL_FORMAT_RGB24:
      return "PIXEL_FORMAT_RGB24";
    case PIXEL_FORMAT_MJPEG:
      return "PIXEL_FORMAT_MJPEG";
    case PIXEL_FORMAT_YUV420P9:
      return "PIXEL_FORMAT_YUV420P9";
    case PIXEL_FORMAT_YUV420P10:
      return "PIXEL_FORMAT_YUV420P10";
    case PIXEL_FORMAT_YUV422P9:
      return "PIXEL_FORMAT_YUV422P9";
    case PIXEL_FORMAT_YUV422P10:
      return "PIXEL_FORMAT_YUV422P10";
    case PIXEL_FORMAT_YUV444P9:
      return "PIXEL_FORMAT_YUV444P9";
    case PIXEL_FORMAT_YUV444P10:
      return "PIXEL_FORMAT_YUV444P10";
    case PIXEL_FORMAT_YUV420P12:
      return "PIXEL_FORMAT_YUV420P12";
    case PIXEL_FORMAT_YUV422P12:
      return "PIXEL_FORMAT_YUV422P12";
    case PIXEL_FORMAT_YUV444P12:
      return "PIXEL_FORMAT_YUV444P12";
    case PIXEL_FORMAT_Y16:
      return "PIXEL_FORMAT_Y16";
    case PIXEL_FORMAT_ABGR:
      return "PIXEL_FORMAT_ABGR";
    case PIXEL_FORMAT_XBGR:
      return "PIXEL_FORMAT_XBGR";
    case PIXEL_FORMAT_P016LE:
      return "PIXEL_FORMAT_P016LE";
    case PIXEL_FORMAT_XR30:
      return "PIXEL_FORMAT_XR30";
    case PIXEL_FORMAT_XB30:
      return "PIXEL_FORMAT_XB30";
    case PIXEL_FORMAT_BGRA:
      return "PIXEL_FORMAT_BGRA";
    case PIXEL_FORMAT_RGBAF16:
      return "PIXEL_FORMAT_RGBAF16";
  }

  return "Invalid VideoPixelFormat";
}

std::string FourccToString(uint32_t fourcc) {
  std::string result = "0000";
  for (size_t i = 0; i < 4; ++i, fourcc >>= 8) {
    const uint8_t c = static_cast<uint8_t>(fourcc & 0xFF);
    if (c <= 0x1f || c >= 0x7f) {
      std::ostringstream string_stream;
      string_stream << std::hex << "0x" << fourcc;
      return string_stream.str();
    }
    result[i] = c;
  }
  return result;
}

VideoCodec VideoCodecProfileToVideoCodec(VideoCodecProfile profile) {
  if (profile >= H264PROFILE_MIN && profile <= H264PROFILE_MAX) {
    return VIDEO_CODEC_H264;
  } else if (profile >= VP8PROFILE_MIN && profile <= VP8PROFILE_MAX) {
    return VIDEO_CODEC_VP8;
  } else if (profile >= VP9PROFILE_MIN && profile <= VP9PROFILE_MAX) {
    return VIDEO_CODEC_VP9;
  } else {
    return VIDEO_CODEC_NONE;
  }
}

}  //  namespace mcil
