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

#ifndef SRC_BASE_CODEC_TYPES_H_
#define SRC_BASE_CODEC_TYPES_H_

typedef enum {
  MEDIA_OK = 0,
  MEDIA_ERROR = -1,
  MEDIA_NOT_IMPLEMENTED = -2,
  MEDIA_NOT_SUPPORTED = -6,
  MEDIA_BUFFER_FULL = -7,                         /**< function doesn't works cause buffer is full */
  MEDIA_INVALID_PARAMS = -3,                      /**< Invalid parameters */
  MEDIA_NOT_READY = -11,                          /**< API's resource is not ready */
} MEDIA_STATUS_T;

typedef enum {
  NOTIFY_LOAD_COMPLETED = 0,
  NOTIFY_UNLOAD_COMPLETED,
  NOTIFY_SOURCE_INFO,
  NOTIFY_END_OF_STREAM,
  NOTIFY_CURRENT_TIME,
  NOTIFY_SEEK_DONE,
  NOTIFY_PLAYING,
  NOTIFY_PAUSED,
  NOTIFY_NEED_DATA,
  NOTIFY_ENOUGH_DATA,
  NOTIFY_SEEK_DATA,
  NOTIFY_ERROR,
  NOTIFY_VIDEO_INFO,
  NOTIFY_AUDIO_INFO,
  NOTIFY_BUFFER_FULL,  // NOTIFY_BUFFERING_END? need to check the chromium media backend
  NOTIFY_BUFFER_NEED,  // NOTIFY_BUFFERING_START?
  NOTIFY_BUFFER_RANGE,
  NOTIFY_BUFFERING_START,
  NOTIFY_BUFFERING_END,
  NOTIFY_ACTIVITY,
  NOTIFY_ACQUIRE_RESOURCE,
  NOTIFY_MAX
} NOTIFY_TYPE_T;

typedef enum MEDIA_MESSAGE {
  MEDIA_MSG_MSG_NONE = 0x00,                         /**< no message */
  MEDIA_MSG_ERR_LOAD = 0x0068,
  MEDIA_MSG_ERR_POLICY = 0x0259,
  MEDIA_MSG_ERR_PLAY = 0xf000,
  MEDIA_CB_MSG_LAST
} MEDIA_MESSAGE_T;

/**
 * video codec
 */
typedef enum {
  MCP_VIDEO_CODEC_NONE,
  MCP_VIDEO_CODEC_H264,
  MCP_VIDEO_CODEC_VC1,
  MCP_VIDEO_CODEC_MPEG2,
  MCP_VIDEO_CODEC_MPEG4,
  MCP_VIDEO_CODEC_THEORA,
  MCP_VIDEO_CODEC_VP8,
  MCP_VIDEO_CODEC_VP9,
  MCP_VIDEO_CODEC_H265,
  MCP_VIDEO_CODEC_MJPEG,
  MCP_VIDEO_CODEC_MAX = MCP_VIDEO_CODEC_MJPEG,
} MCP_VIDEO_CODEC;

/* video codec */
typedef enum {
  MCP_PIXEL_NONE,
  MCP_PIXEL_I420,
  MCP_PIXEL_YUY2,
  MCP_PIXEL_YUYV,
  MCP_PIXEL_RGB16,
} MCP_PIXEL_FMT;

#endif  // SRC_BASE_CODEC_TYPES_H_
