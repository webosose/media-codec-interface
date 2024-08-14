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

#ifndef SRC_BASE_DECODER_TYPES_H_
#define SRC_BASE_DECODER_TYPES_H_

#include "codec_types.h"
#include "video_frame.h"

namespace mcil {

// Same as the enum Error defined in upstream Chromium file
// media/video/video_decode_accelerator.h
enum DecoderError {
  ILLEGAL_STATE = 1,
  INVALID_ARGUMENT = 2,
  UNREADABLE_INPUT = 3,
  PLATFORM_FAILURE = 4,
  ERROR_MAX = PLATFORM_FAILURE,
};

// Same as the enum OutputMode defined in upstream Chromium file
// media/video/video_decode_accelerator.h
enum OutputMode {
  OUTPUT_ALLOCATE,
  OUTPUT_IMPORT,
};

// Same as the enum BufferId defined in upstream Chromium file
// media/video/video_decode_accelerator.h
enum BufferId {
  kFlushBufferId = -2  // Buffer id for flush buffer, queued by FlushTask().
};

enum PostTaskType {
  INVALID_DEVICE_POST_TASK,
  ENQUEUE_INPUT_BUFFER_TASK,
  ENQUEUE_OUTPUT_BUFFER_TASK,
  DEQUEUE_INPUT_BUFFER_TASK,
  DEQUEUE_OUTPUT_BUFFER_TASK,
  DECODER_DEVICE_READ_TASK,
  DECODER_DEVICE_WRITE_TASK,
  DISPLAY_DEVICE_READ_TASK,
  DISPLAY_DEVICE_WRITE_TASK,
  SCALER_DEVICE_READ_TASK,
};

/* Decoder configure data structure */
class DecoderConfig {
 public:
  DecoderConfig() = default;
  ~DecoderConfig() = default;

  uint32_t frameWidth;
  uint32_t frameHeight;
  VideoCodecProfile profile;
  OutputMode opMode;
};

/* DecoderClinet configure data structure */
class DecoderClientConfig {
 public:
  DecoderClientConfig() = default;
  ~DecoderClientConfig() = default;

  VideoPixelFormat output_pixel_format = PIXEL_FORMAT_UNKNOWN;
  bool should_control_buffer_feed = false;
};

}  // namespace mcil

#endif  // SRC_BASE_DECODER_TYPES_H_
