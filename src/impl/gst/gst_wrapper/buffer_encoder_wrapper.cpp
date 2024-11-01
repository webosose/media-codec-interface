#include "buffer_encoder_wrapper.h"

#include <buffer_encoder_lib.h>

namespace cpplib_glue {
namespace mrf {

BufferEncoder::BufferEncoder() : buffer_encoder_(CreateBufferEncoder(this)) {}

BufferEncoder::~BufferEncoder() {
  DestroyBufferEncoder(buffer_encoder_);
}

void OnEncodedBufferCallbackHandler(void* that,
                                    const uint8_t* data,
                                    size_t size,
                                    uint64_t timestamp,
                                    bool is_keyframe) {
  cpplib_glue::mrf::BufferEncoder* buffer_encoder =
      static_cast<cpplib_glue::mrf::BufferEncoder*>(that);
  buffer_encoder->OnEncodedBufferCallback(data, size, timestamp, is_keyframe);
}

void BufferEncoder::OnEncodedBufferCallback(const uint8_t* data,
                                            size_t size,
                                            uint64_t timestamp,
                                            bool is_keyframe) {
  encoded_buffer_callback_(data, size, timestamp, is_keyframe);
}

bool BufferEncoder::Initialize(const EncoderConfig* config_data,
                               EncodedBufferCallback buffer_callback) {
  encoded_buffer_callback_ = std::move(buffer_callback);
  return BufferEncoderInitialize(
      buffer_encoder_, &OnEncodedBufferCallbackHandler, config_data->frameRate,
      config_data->bitRate, config_data->width, config_data->height,
      (std::underlying_type<VideoPixelFormat>::type)config_data->pixelFormat,
      config_data->outputBufferSize, config_data->h264OutputLevel,
      config_data->gopLength,
      (std::underlying_type<VideoCodecProfile>::type)config_data->profile);
}

void BufferEncoder::Destroy() {
  BufferEncoderDestroy(buffer_encoder_);
}

bool BufferEncoder::EncodeBuffer(const uint8_t* y_buf,
                                 size_t y_size,
                                 const uint8_t* u_buf,
                                 size_t u_size,
                                 const uint8_t* v_buf,
                                 size_t v_size,
                                 uint64_t buffer_timestamp,
                                 const bool request_key_frame) {
  return BufferEncoderEncodeBuffer(buffer_encoder_, y_buf, y_size, u_buf, u_size,
                                   v_buf, v_size, buffer_timestamp,
                                   request_key_frame);
}

bool BufferEncoder::UpdateEncodingParams(uint32_t bitrate, uint32_t framerate) {
  return BufferEncoderUpdateEncodingParams(buffer_encoder_, bitrate, framerate);
}

}  //  namespace mrf
}  //  namespace cpplib_glue
