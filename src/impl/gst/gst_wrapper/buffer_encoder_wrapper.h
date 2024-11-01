#ifndef SRC_IMPL_GST_GST_WRAPPER_BUFFER_ENCODER_WRAPPER_H_
#define SRC_IMPL_GST_GST_WRAPPER_BUFFER_ENCODER_WRAPPER_H_
#if !defined(ENABLE_WRAPPER)
#include <buffer_encoder.h>
#else
#include <gst/gst.h>
#include <chrono>
#include <functional>
#include "log.h"

#include "base/encoder_types.h"

namespace cpplib_glue {
namespace mrf {
typedef mcil::VideoPixelFormat VideoPixelFormat;
typedef mcil::VideoCodecProfile VideoCodecProfile;
typedef mcil::EncoderConfig EncoderConfig;

class BufferEncoder {
 public:
  BufferEncoder();
  virtual ~BufferEncoder();

  using EncodedBufferCallback = std::function<void(const uint8_t* data,
                                                   size_t size,
                                                   uint64_t timestamp,
                                                   bool is_keyframe)>;

  void OnEncodedBufferCallback(const uint8_t* data,
                               size_t size,
                               uint64_t timestamp,
                               bool is_keyframe);
  bool Initialize(const EncoderConfig* config_data,
                  EncodedBufferCallback buffer_callback);
  void Destroy();
  bool EncodeBuffer(const uint8_t* y_buf,
                    size_t y_size,
                    const uint8_t* u_buf,
                    size_t u_size,
                    const uint8_t* v_buf,
                    size_t v_size,
                    uint64_t buffer_timestamp,
                    const bool request_key_frame);
  bool UpdateEncodingParams(uint32_t bitrate, uint32_t framerate);

 private:
  void* buffer_encoder_;
  EncodedBufferCallback encoded_buffer_callback_;
};

}  //  namespace mrf
}  //  namespace cpplib_glue

namespace mrf = cpplib_glue::mrf;

#endif  // !defined(USE_MEDIA_WRAPPER)
#endif  // SRC_IMPL_GST_GST_WRAPPER_BUFFER_ENCODER_WRAPPER_H_
