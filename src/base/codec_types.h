// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_BASE_CODEC_TYPES_H_
#define SRC_BASE_CODEC_TYPES_H_

#include <cstdint>

#include <chrono>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ref_counted.h"

namespace mcil {

/**
 * Video Codec Type enum
 */
enum VideoCodec {
  VIDEO_CODEC_NONE = 0,
  VIDEO_CODEC_H264 = 1,
  VIDEO_CODEC_VC1 = 2,
  VIDEO_CODEC_MPEG2 = 3,
  VIDEO_CODEC_MPEG4 = 4,
  VIDEO_CODEC_THEORA = 5,
  VIDEO_CODEC_VP8 = 6,
  VIDEO_CODEC_VP9 = 7,
  VIDEO_CODEC_HEVC = 8,
  VIDEO_CODEC_DOLBYVISION = 9,
  VIDEO_CODEC_AV1 = 10,
  VIDEO_CODEC_MAX = VIDEO_CODEC_AV1,
};

/**
 * Video Pixel Format enum
 */
enum VideoPixelFormat {
  PIXEL_FORMAT_UNKNOWN = 0,
  PIXEL_FORMAT_I420 = 1,
  PIXEL_FORMAT_YV12 = 2,
  PIXEL_FORMAT_I422 = 3,
  PIXEL_FORMAT_I420A = 4,
  PIXEL_FORMAT_I444 = 5,
  PIXEL_FORMAT_NV12 = 6,
  PIXEL_FORMAT_NV21 = 7,
  PIXEL_FORMAT_UYVY = 8,
  PIXEL_FORMAT_YUY2 = 9,
  PIXEL_FORMAT_ARGB = 10,
  PIXEL_FORMAT_XRGB = 11,
  PIXEL_FORMAT_RGB24 = 12,
  PIXEL_FORMAT_MJPEG = 14,
  PIXEL_FORMAT_YUV420P9 = 16,
  PIXEL_FORMAT_YUV420P10 = 17,
  PIXEL_FORMAT_YUV422P9 = 18,
  PIXEL_FORMAT_YUV422P10 = 19,
  PIXEL_FORMAT_YUV444P9 = 20,
  PIXEL_FORMAT_YUV444P10 = 21,
  PIXEL_FORMAT_YUV420P12 = 22,
  PIXEL_FORMAT_YUV422P12 = 23,
  PIXEL_FORMAT_YUV444P12 = 24,
  PIXEL_FORMAT_Y16 = 26,
  PIXEL_FORMAT_ABGR = 27,
  PIXEL_FORMAT_XBGR = 28,
  PIXEL_FORMAT_P016LE = 29,
  PIXEL_FORMAT_XR30 = 30,
  PIXEL_FORMAT_XB30 = 31,
  PIXEL_FORMAT_BGRA = 32,
  PIXEL_FORMAT_RGBAF16 = 33,
  PIXEL_FORMAT_MAX = PIXEL_FORMAT_RGBAF16,
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
  IMAGE_PROCESSOR,
};

enum QueueType {
  INVALID_QUEUE,
  INPUT_QUEUE,
  OUTPUT_QUEUE,
};

// Combination of enum State defined in upstream Chromium files
// v4l2_video_decode_accelerator.h and v4l2_video_encode_accelerator.h
enum CodecState {
  kUninitialized = 0,
  kInitialized = 1,
  kDecoding = 2,
  kEncoding = 3,
  kFlushing = 4,
  kResetting =5,
  kChangingResolution = 6,
  kAwaitingPictureBuffers = 7,
  kDecoderError = 8,
  kEncoderError= 9,
  kDestroying = 10,
  kCodecStateMax = kDestroying,
};

enum RateControlMode : uint8_t {
  kNoMode = 0,
  kConstantMode = 0b0001,
  kVariableMode = 0b0010,
};

class Size {
 public:
  Size() = default;
  Size(uint32_t w, uint32_t h);

  void SetSize(uint32_t w, uint32_t h);
  uint32_t GetArea() const { return width * height; };
  bool IsEmpty() const { return (width == 0) || (height == 0); }

  uint32_t width = 0;
  uint32_t height = 0;
};

inline bool operator==(const Size& lhs, const Size& rhs) {
  return (lhs.width == rhs.width) && (lhs.height == rhs.height);
}

inline bool operator!=(const Size& lhs, const Size& rhs) {
  return !(lhs == rhs);
}

class Rect {
 public:
  Rect() = default;
  Rect(int32_t x, int32_t y, uint32_t w, uint32_t h);
  Rect(const Size& size_config);

  const Size getSize() { return Size(width, height); }
  bool Contains(const Rect& bounded_rect) const;
  bool IsEmpty() const { return (width == 0) || (height == 0); }

  int32_t x = 0;
  int32_t y = 0;
  uint32_t width = 0;
  uint32_t height = 0;
};

inline bool operator==(const Rect& lhs, const Rect& rhs) {
  return ((lhs.x == rhs.x) && (lhs.y == rhs.y)) &&
         ((lhs.width == rhs.width) && (lhs.height == rhs.height));
}

inline bool operator!=(const Rect& lhs, const Rect& rhs) {
  return !(lhs == rhs);
}

class SupportedProfile {
 public:
  SupportedProfile() = default;
  ~SupportedProfile() = default;
  SupportedProfile(VideoCodecProfile prof, Size min, Size max);

  VideoCodecProfile profile;
  Size max_resolution;
  Size min_resolution;
  bool encrypted_only = false;
  RateControlMode rc_modes = kNoMode;
};

std::string GetProfileName(VideoCodecProfile profile);
std::string VideoPixelFormatToString(VideoPixelFormat format);
std::string FourccToString(uint32_t fourcc);
VideoCodec VideoCodecProfileToVideoCodec(VideoCodecProfile profile);

typedef std::vector<SupportedProfile> SupportedProfiles;
typedef std::chrono::system_clock::time_point ChronoTime;

}  //  namespace mcil

#endif  // SRC_BASE_CODEC_TYPES_H_
