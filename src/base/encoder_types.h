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

typedef enum MCIL_MEDIA_STATUS {
  MCIL_MEDIA_OK = 0,
  MCIL_MEDIA_ERROR = -1,
  MCIL_MEDIA_NOT_IMPLEMENTED = -2,
  MCIL_MEDIA_NOT_SUPPORTED = -6,
  MCIL_MEDIA_BUFFER_FULL = -7,                         /**< function doesn't works cause buffer is full */
  MCIL_MEDIA_INVALID_PARAMS = -3,                      /**< Invalid parameters */
  MCIL_MEDIA_NOT_READY = -11,                          /**< API's resource is not ready */
} MCIL_MEDIA_STATUS_T;

typedef struct ENCODING_ERRORS {
  gint32 errorCode;
  gchar* errorStr;
} ENCODING_ERRORS_T;

/**
 * Data structure for encoding parameters
 */
typedef struct ENCODING_PARAMS {
  gint32 bitRate;
  guint32 frameRate;
} ENCODING_PARAMS_T;

/**
 * Load data structure for Buffer Player
 */
typedef struct ENCODER_INIT_DATA {
  /* config for video */
  guint32 frameRate;
  guint32 bitRate;
  guint32 width;
  guint32 height;
  MCIL_PIXEL_FMT pixelFormat;
  MCIL_VIDEO_CODEC codecType;
  guint32 bufferSize;
} ENCODER_INIT_DATA_T;

typedef enum {
  ENCODER_CB_LOAD_COMPLETE = 0,
  ENCODER_CB_NOTIFY_PLAYING,
  ENCODER_CB_NOTIFY_PAUSED,
  ENCODER_CB_BUFFER_ENCODED,
  ENCODER_CB_NOTIFY_EOS,
  ENCODER_CB_NOTIFY_ERROR,
  ENCODER_CB_SOURCE_INFO,
  ENCODER_CB_UNLOAD_COMPLETE,
  ENCODER_CB_TYPE_MAX = ENCODER_CB_UNLOAD_COMPLETE,
} ENCODER_CB_TYPE_T;

using ENCODER_CALLBACK_T = std::function<void(
    const gint type, const void* cbData, void *userData)>;

using NEWFRAME_CALLBACK_T = std::function<void(
    const uint8_t* buffer, uint32_t bufferSize, uint64_t timeStamp, bool isKeyFrame)>;

#endif  // SRC_BASE_ENCODER_TYPES_H_
