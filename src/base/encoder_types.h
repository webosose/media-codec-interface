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
  EncodingParams() = default;
  ~EncodingParams() = default;

  uint32_t bitRate;
  uint32_t frameRate;
};

/* Encoder config data structure */
class EncoderConfig {
 public:
  EncoderConfig() = default;
  ~EncoderConfig() = default;

  uint32_t frameRate;
  uint32_t bitRate;
  uint32_t width;
  uint32_t height;
  VideoPixelFormat pixelFormat;
  VideoCodecType codecType;
  uint32_t bufferSize;
};

}  // namespace mcil

#endif  // SRC_BASE_ENCODER_TYPES_H_
