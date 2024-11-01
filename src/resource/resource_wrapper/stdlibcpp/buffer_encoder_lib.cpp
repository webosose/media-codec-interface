#include "buffer_encoder_lib.h"

#include "buffer_encoder_lib_impl.h"

using stdlibcpp::BufferEncoderImpl;

void* CreateBufferEncoder(void* wrapper) {
  return new BufferEncoderImpl(wrapper);
}

void DestroyBufferEncoder(void* ptr) {
  BufferEncoderImpl* be = static_cast<BufferEncoderImpl*>(ptr);
  if (be)
    delete be;
}

bool BufferEncoderEncodeBuffer(void* ptr,
                               const uint8_t* y_buf,
                               size_t y_size,
                               const uint8_t* u_buf,
                               size_t u_size,
                               const uint8_t* v_buf,
                               size_t v_size,
                               uint64_t buffer_timestamp,
                               const bool request_key_frame) {
  BufferEncoderImpl* be = static_cast<BufferEncoderImpl*>(ptr);
  return be->EncodeBuffer(y_buf, y_size, u_buf, u_size, v_buf, v_size,
                          buffer_timestamp, request_key_frame);
}

void BufferEncoderDestroy(void* ptr) {
  BufferEncoderImpl* be = static_cast<BufferEncoderImpl*>(ptr);
  be->Destroy();
}

bool BufferEncoderUpdateEncodingParams(void* ptr,
                                       uint32_t bit_rate,
                                       uint32_t frame_rate) {
  BufferEncoderImpl* be = static_cast<BufferEncoderImpl*>(ptr);
  return be->UpdateEncodingParams(bit_rate, frame_rate);
}

bool BufferEncoderInitialize(void* ptr,
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
  BufferEncoderImpl* be = static_cast<BufferEncoderImpl*>(ptr);
  return be->InitializeWithHandler(callback, frame_rate, bit_rate, width,
                                   height, pixel_format, output_buffer_size,
                                   h264_output_level, gop_length, profile);
}
