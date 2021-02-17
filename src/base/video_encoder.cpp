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

#include "video_encoder.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <map>
#include <memory>
#include <gio/gio.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>

#include "base/log.h"

const std::string kFormatYUV = "YUY2";
#define CURR_TIME_INTERVAL_MS    100
#define LOAD_DONE_TIMEOUT_MS     10

#define MEDIA_VIDEO_MAX      (15 * 1024 * 1024)  // 15MB
#define QUEUE_MAX_SIZE       (12 * 1024 * 1024)  // 12MB
#define QUEUE_MAX_TIME       (10 * GST_SECOND)   // 10Secs

#define BUFFER_MIN_PERCENT 50
#define MEDIA_CHANNEL_MAX  2


namespace mcil {

namespace encoder {

VideoEncoder::VideoEncoder() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

VideoEncoder::~VideoEncoder() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

bool VideoEncoder::Init(const ENCODER_INIT_DATA_T* loadData,
                        NEWFRAME_CALLBACK_T new_frame_cb) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  new_frame_cb_ = new_frame_cb;
  return true;
}

bool VideoEncoder::Deinit() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  return true;
}

MCIL_MEDIA_STATUS_T VideoEncoder::Feed(const uint8_t* bufferPtr,
                                       size_t bufferSize) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return MCIL_MEDIA_ERROR;
}

MCIL_MEDIA_STATUS_T VideoEncoder::Feed(const uint8_t* yBuf, size_t ySize,
                                       const uint8_t* uBuf, size_t uSize,
                                       const uint8_t* vBuf, size_t vSize,
                                       uint64_t bufferTimestamp,
                                       const bool requestKeyFrame) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return MCIL_MEDIA_ERROR;
}

bool VideoEncoder::IsEncoderAvailable() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return false;
}

bool VideoEncoder::UpdateEncodingResolution(uint32_t width, uint32_t height) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return false;
}

void VideoEncoder::RegisterCbFunction(CALLBACK_T callBackFunction)
{
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  cbFunction_ = callBackFunction;
}

}  // namespace encoder

}  // namespace mcil
