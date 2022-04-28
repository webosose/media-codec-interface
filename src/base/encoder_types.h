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

namespace mcil {

/**
 * Class for encoding parameters
 */
class EncodingParams {
 public:
  EncodingParams();
  ~EncodingParams();

  uint32_t bitRate = 0;
  uint32_t frameRate = 0;
};

/* Class for Encoder config data */
class EncoderConfig {
 public:
  EncoderConfig();
  ~EncoderConfig();

  uint32_t frameRate = 0;
  uint32_t bitRate = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  VideoPixelFormat pixelFormat = PIXEL_FORMAT_UNKNOWN;
  VideoCodecType codecType = VIDEO_CODEC_NONE;
  uint32_t bufferSize = 0;
};

}  // namespace mcil

#endif  // SRC_BASE_ENCODER_TYPES_H_
