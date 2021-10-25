
#include "codec_types.h"

#include <sstream>

namespace mcil {

Size::Size(uint32_t w, uint32_t h)
 : width(w), height(h) {}

Rect::Rect(int32_t x, int32_t y, uint32_t w, uint32_t h)
 :  x(x), y(y), width(w), height(h) {}

Rect::Rect(const Size& size)
 : width(size.width), height(size.height) {}

bool Rect::Contains(const Rect& rect) const {
  return (rect.x >= x && rect.width <= width && rect.y >= y &&
          rect.height <= height);
}

std::string FourccToString(uint32_t fourcc) {
  std::string result = "0000";
  for (size_t i = 0; i < 4; ++i, fourcc >>= 8) {
    const char c = static_cast<char>(fourcc & 0xFF);
    if (c <= 0x1f || c >= 0x7f) {
      std::ostringstream string_stream;
      string_stream << std::hex << "0x" << fourcc;
      return string_stream.str();
    }
    result[i] = c;
  }
  return result;
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

}  //  namespace mcil
