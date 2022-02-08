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

#ifndef SRC_BASE_CODEC_TYPES_H_
#define SRC_BASE_CODEC_TYPES_H_

#include <stdint.h>

#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace mcil {

/**
 * Video Codec Type enum
 */
enum VideoCodecType {
  VIDEO_CODEC_NONE,
  VIDEO_CODEC_H264,
  VIDEO_CODEC_VC1,
  VIDEO_CODEC_MPEG2,
  VIDEO_CODEC_MPEG4,
  VIDEO_CODEC_THEORA,
  VIDEO_CODEC_VP8,
  VIDEO_CODEC_VP9,
  VIDEO_CODEC_H265,
  VIDEO_CODEC_MJPEG,
  VIDEO_CODEC_MAX = VIDEO_CODEC_MJPEG,
};

/**
 * Video Pixel Format enum
 */
enum VideoPixelFormat {
  PIXEL_FMT_UNKNOWN = 0,
  PIXEL_FMT_I420 = 1,
  PIXEL_FMT_YV12 = 2,
  PIXEL_FMT_I422 = 3,
  PIXEL_FMT_I420A = 4,
  PIXEL_FMT_I444 = 5,
  PIXEL_FMT_NV12 = 6,
  PIXEL_FMT_NV21 = 7,
  PIXEL_FMT_UYVY = 8,
  PIXEL_FMT_YUY2 = 9,
  PIXEL_FMT_ARGB = 10,
  PIXEL_FMT_XRGB = 11,
  PIXEL_FMT_RGB24 = 12,
  PIXEL_FMT_MJPEG = 14,
  PIXEL_FMT_YUV420P9 = 16,
  PIXEL_FMT_YUV420P10 = 17,
  PIXEL_FMT_YUV422P9 = 18,
  PIXEL_FMT_YUV422P10 = 19,
  PIXEL_FMT_YUV444P9 = 20,
  PIXEL_FMT_YUV444P10 = 21,
  PIXEL_FMT_YUV420P12 = 22,
  PIXEL_FMT_YUV422P12 = 23,
  PIXEL_FMT_YUV444P12 = 24,
  PIXEL_FMT_Y16 = 26,
  PIXEL_FMT_ABGR = 27,
  PIXEL_FMT_XBGR = 28,
  PIXEL_FMT_P016LE = 29,
  PIXEL_FMT_XR30 = 30,
  PIXEL_FMT_XB30 = 31,
  PIXEL_FMT_BGRA = 32,
  PIXEL_FMT_MAX = PIXEL_FMT_BGRA,
};

/**
 * Video Codec Profile enum
 */
enum VideoCodecProfile {
  VIDEO_CODEC_PROFILE_UNKNOWN = -1,
  VIDEO_CODEC_PROFILE_MIN = VIDEO_CODEC_PROFILE_UNKNOWN,
  H264PROFILE_MIN = 0,
  H264PROFILE_BASELINE = H264PROFILE_MIN,
  H264PROFILE_MAIN = 1,
  H264PROFILE_EXTENDED = 2,
  H264PROFILE_HIGH = 3,
  H264PROFILE_HIGH10PROFILE = 4,
  H264PROFILE_HIGH422PROFILE = 5,
  H264PROFILE_HIGH444PREDICTIVEPROFILE = 6,
  H264PROFILE_SCALABLEBASELINE = 7,
  H264PROFILE_SCALABLEHIGH = 8,
  H264PROFILE_STEREOHIGH = 9,
  H264PROFILE_MULTIVIEWHIGH = 10,
  H264PROFILE_MAX = H264PROFILE_MULTIVIEWHIGH,
  VP8PROFILE_MIN = 11,
  VP8PROFILE_ANY = VP8PROFILE_MIN,
  VP8PROFILE_MAX = VP8PROFILE_ANY,
  VP9PROFILE_MIN = 12,
  VP9PROFILE_PROFILE0 = VP9PROFILE_MIN,
  VP9PROFILE_PROFILE1 = 13,
  VP9PROFILE_PROFILE2 = 14,
  VP9PROFILE_PROFILE3 = 15,
  VP9PROFILE_MAX = VP9PROFILE_PROFILE3,
  HEVCPROFILE_MIN = 16,
  HEVCPROFILE_MAIN = HEVCPROFILE_MIN,
  HEVCPROFILE_MAIN10 = 17,
  HEVCPROFILE_MAIN_STILL_PICTURE = 18,
  HEVCPROFILE_MAX = HEVCPROFILE_MAIN_STILL_PICTURE,
  DOLBYVISION_PROFILE0 = 19,
  DOLBYVISION_PROFILE4 = 20,
  DOLBYVISION_PROFILE5 = 21,
  DOLBYVISION_PROFILE7 = 22,
  THEORAPROFILE_MIN = 23,
  THEORAPROFILE_ANY = THEORAPROFILE_MIN,
  THEORAPROFILE_MAX = THEORAPROFILE_ANY,
  AV1PROFILE_MIN = 24,
  AV1PROFILE_PROFILE_MAIN = AV1PROFILE_MIN,
  AV1PROFILE_PROFILE_HIGH = 25,
  AV1PROFILE_PROFILE_PRO = 26,
  AV1PROFILE_MAX = AV1PROFILE_PROFILE_PRO,
  DOLBYVISION_PROFILE8 = 27,
  DOLBYVISION_PROFILE9 = 28,
  VIDEO_CODEC_PROFILE_MAX = DOLBYVISION_PROFILE9,
};

enum DeviceType {
  V4L2_DECODER,
  V4L2_ENCODER,
  JPEG_DECODER,
};

class Size {
 public:
  Size() = default;
  Size(uint32_t w, uint32_t h);

  bool IsEmpty() const { return !width || !height; }

  uint32_t width = 0;
  uint32_t height = 0;
};

class Rect {
 public:
  Rect() = default;
  Rect(int32_t x, int32_t y, uint32_t w, uint32_t h);
  Rect(const Size& size);

  bool Contains(const Rect& rect) const;
  bool IsEmpty() const { return !width || !height; }

  int32_t x = 0;
  int32_t y = 0;
  uint32_t width = 0;
  uint32_t height = 0;
};

class SupportedProfile {
 public:
  SupportedProfile() = default;
  ~SupportedProfile() = default;
  VideoCodecProfile profile;
  Size max_resolution;
  Size min_resolution;
  bool encrypted_only = false;
};

typedef std::vector<mcil::SupportedProfile> SupportedProfiles;

std::string FourccToString(uint32_t fourcc);

std::string GetProfileName(VideoCodecProfile profile);

}  //  namespace mcil

#endif  // SRC_BASE_CODEC_TYPES_H_
