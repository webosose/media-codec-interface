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

#ifndef SRC_BASE_ENCODER_TYPES_H_
#define SRC_BASE_ENCODER_TYPES_H_

#include "codec_types.h"
#include "video_buffers.h"
#include "video_frame.h"

namespace mcil {

// Same as the enum Error defined in upstream Chromium file
// media/video/video_encode_accelerator.h
enum EncoderError {
  kIllegalStateError,
  kInvalidArgumentError,
  kPlatformFailureError,
  kErrorMax = kPlatformFailureError
};

/* Encoder config data structure */
class EncoderConfig {
 public:
  EncoderConfig() = default;
  ~EncoderConfig() = default;

  uint32_t frameRate;
  uint32_t bitRate;
  int width;
  int height;
  VideoPixelFormat pixelFormat;
  size_t outputBufferSize;
  uint8_t h264OutputLevel;
  uint32_t gopLength;
  VideoCodecProfile profile;
};

/* EncoderClinet configure data structure */
class EncoderClientConfig {
 public:
  EncoderClientConfig() = default;
  ~EncoderClientConfig() = default;

  Size input_frame_size;
  bool should_control_buffer_feed = false;
  size_t output_buffer_byte_size = 0;
  bool should_inject_sps_and_pps = false;
};

}  // namespace mcil

#endif  // SRC_BASE_ENCODER_TYPES_H_
