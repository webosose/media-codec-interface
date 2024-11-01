#include "buffer_encoder_lib_impl.h"

namespace stdlibcpp {

BufferEncoderImpl::BufferEncoderImpl(void* wrapper) : wrapper_(wrapper) {}

bool BufferEncoderImpl::InitializeWithHandler(
    funcOnBufferCallbackHandler callback,
    uint32_t frame_rate,
    uint32_t bit_rate,
    int width,
    int height,
    int pixel_format,
    size_t output_buffer_size,
    uint8_t h264_output_level,
    uint32_t gop_length,
    int profile) {
  callback_ = callback;

  mrf::EncoderConfig config;
  config.frameRate = frame_rate;
  config.bitRate = bit_rate;
  config.width = width;
  config.height = height;
  config.pixelFormat = static_cast<mrf::VideoPixelFormat>(pixel_format);
  config.outputBufferSize = output_buffer_size;
  config.h264OutputLevel = h264_output_level;
  config.gopLength = gop_length;
  config.profile = static_cast<mrf::VideoCodecProfile>(profile);

  auto buffer_callback = [this](const uint8_t* data, size_t size,
                                uint64_t timestamp, bool is_keyframe) {
    this->OnEncodedBufferCallback(data, size, timestamp, is_keyframe);
  };

  return Initialize(&config, buffer_callback);
}

void BufferEncoderImpl::OnEncodedBufferCallback(const uint8_t* data,
                                                size_t size,
                                                uint64_t timestamp,
                                                bool is_keyframe) {
  callback_(wrapper_, data, size, timestamp, is_keyframe);
}

}  // namespace stdlibcpp
