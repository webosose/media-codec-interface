#ifndef SRC_RESOURCE_RESOURCE_WRAPPER_RESOURCE_CALCULATOR_WRAPPER_H_
#define SRC_RESOURCE_RESOURCE_WRAPPER_RESOURCE_CALCULATOR_WRAPPER_H_
#if !defined(ENABLE_WRAPPER)
#include <resource_calculator.h>
#else
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "resource_calculator_type_converter.h"

namespace cpplib_glue {
namespace mrc {

class ResourceCalculator {
 public:
  static ResourceCalculator* create();

  virtual ~ResourceCalculator();

  enum VideoCodec : int32_t {
    kVideoEtc = (1 << 0),
    kVideoH264 = (1 << 1),
    kVideoH265 = (1 << 2),
    kVideoMPEG = (1 << 3), // MPEG1/2
    kVideoMVC = (1 << 4),
    kVideoSVC = (1 << 5),
    kVideoVP9 = (1 << 6),
    kVideoRM = (1 << 7),
    kVideoAVS = (1 << 8),
    kVideoVP8 = (1 << 9),
    kVideoMJPEG = (1 << 10),
    kVideoMPEG4 = (1 << 11),
    kVideoJPEG = (1 << 12),
    kVideoAV1 = (1 << 13),
    kVideoWMV = (1 << 14),
    kVideoH266 = (1 << 15),
    kVideoH263 = (1 << 16)
  };
  typedef int32_t VideoCodecs;

  enum ScanType {
    kScanProgressive,
    kScanInterlaced,
  };

  enum _3DType {
    k3DNone,
    k3DSequential,
    k3DMultiStream,
  };

  enum AudioCodec : int32_t {
    kAudioEtc = (1 << 0),
    kAudioMPEG = (1 << 1),
    kAudioPCM = (1 << 2),
    kAudioDTS = (1 << 3),
    kAudioDTSE = (1 << 4),
    kAudioMPEGH = (1 << 5),
    kAudioAC4 = (1 << 6),
    kAudioATMOS = (1 << 7),
    kAudioDescription = (1 << 8),
    kAudioMAT = (1 << 9),
    kAudioMax = (1 << 10),
  };
  typedef int32_t AudioCodecs;

  virtual ResourceListOptions calcVdecResourceOptions(VideoCodecs codecs,
                                                      int32_t width,
                                                      int32_t height,
                                                      int32_t frame_rate_data,
                                                      ScanType scan_type_data,
                                                      _3DType type_data_3d);

  virtual ResourceListOptions calcVencResourceOptions(VideoCodecs codecs,
                                                      int32_t width,
                                                      int32_t height,
                                                      int32_t frame_rate);

 protected:
  ResourceCalculator();

 private:
  void* resource_calculator_;
};

}  //  namespace mrc
}  //  namespace cpplib_glue

namespace mrc = cpplib_glue::mrc;

#endif  // !defined(USE_MEDIA_WRAPPER)
#endif  // SRC_RESOURCE_RESOURCE_WRAPPER_RESOURCE_CALCULATOR_WRAPPER_H_
