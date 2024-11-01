#ifndef SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_BUFFER_ENCODER_LIB_H_
#define SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_BUFFER_ENCODER_LIB_H_

#include <stddef.h>
#include <cstdint>

extern "C" {

typedef void (*funcOnBufferCallbackHandler)(void*,
                                            const uint8_t*,
                                            size_t,
                                            uint64_t,
                                            bool);
void* CreateBufferEncoder(void* wrapper);
void DestroyBufferEncoder(void* ptr);

bool BufferEncoderEncodeBuffer(void* ptr,
                               const uint8_t* y_buf,
                               size_t y_size,
                               const uint8_t* u_buf,
                               size_t u_size,
                               const uint8_t* v_buf,
                               size_t v_size,
                               uint64_t buffer_timestamp,
                               const bool request_key_frame);

void BufferEncoderDestroy(void* ptr);

bool BufferEncoderUpdateEncodingParams(void* ptr,
                                       uint32_t bit_rate,
                                       uint32_t frame_rate);

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
                             int profile);
}
#endif  // SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_BUFFER_ENCODER_LIB_H_
