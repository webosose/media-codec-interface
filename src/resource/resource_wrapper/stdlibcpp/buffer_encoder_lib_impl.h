#ifndef SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_BUFFER_ENCODER_LIB_IMPL_H_
#define SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_BUFFER_ENCODER_LIB_IMPL_H_

#include <buffer_encoder.h>

#include "buffer_encoder_lib.h"

namespace stdlibcpp {
class BufferEncoderImpl : public mrf::BufferEncoder {
 public:
  BufferEncoderImpl(void* wrapper);

  bool InitializeWithHandler(funcOnBufferCallbackHandler callback,
                             uint32_t frame_rate,
                             uint32_t bit_rate,
                             int width,
                             int height,
                             int pixel_format,
                             size_t output_buffer_size,
                             uint8_t h264_output_level,
                             uint32_t gop_length,
                             int profile);

 private:
  void OnEncodedBufferCallback(const uint8_t* data,
                               size_t size,
                               uint64_t timestamp,
                               bool is_keyframe);

 private:
  funcOnBufferCallbackHandler callback_;
  void* wrapper_;
};

}  // namespace stdlibcpp

#endif  // SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_BUFFER_ENCODER_LIB_IMPL_H_
