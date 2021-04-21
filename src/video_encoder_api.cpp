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


#include <functional>
#include <vector>
#include <map>

#include <base/log.h>

#include "video_encoder_api.h"
#include "base/encoder_types.h"
#include "base/video_encoder.h"

namespace mcil {

namespace encoder {

bool VideoEncoderAPI::IsCodecSupported(MCIL_VIDEO_CODEC videoCodec) {
  return mcil::encoder::VideoEncoder::IsCodecSupported(videoCodec);
}

VideoEncoderAPI::VideoEncoderAPI() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

VideoEncoderAPI::~VideoEncoderAPI() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

bool VideoEncoderAPI::Init(const ENCODER_INIT_DATA_T* loadData,
                              NEWFRAME_CALLBACK_T new_frame_cb) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  MCIL_INFO_PRINT("Load loadData = %p", loadData);
  MCIL_INFO_PRINT("Load width = %d", loadData->width);
  MCIL_INFO_PRINT("Load height = %d", loadData->height);
  MCIL_INFO_PRINT("Load FrameRate = %d", loadData->frameRate);
  MCIL_INFO_PRINT("Load pixelFormat = %d", loadData->pixelFormat);

  videoEncoder_ = mcil::encoder::VideoEncoder::Create(loadData->codecType);
  if (!videoEncoder_) {
    MCIL_INFO_PRINT("Encoder Buffer not created");
    return false;
  }
  bool ret = videoEncoder_ ->Init(loadData, new_frame_cb);
  videoEncoder_->RegisterCbFunction (
          std::bind(&VideoEncoderAPI::Notify, this ,
              std::placeholders::_1, std::placeholders::_2,
              std::placeholders::_3, std::placeholders::_4));

  return ret;
}

bool VideoEncoderAPI::Deinit() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  return videoEncoder_ ->Deinit();
}

MCIL_MEDIA_STATUS_T VideoEncoderAPI::Encode(const uint8_t* bufferPtr,
                                              size_t bufferSize) {
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Error: encoder (%p)", videoEncoder_.get());
    return MCIL_MEDIA_ERROR;
  }
  return videoEncoder_->Feed(bufferPtr, bufferSize);
}

MCIL_MEDIA_STATUS_T VideoEncoderAPI::Encode(const uint8_t* yBuf, size_t ySize,
                                              const uint8_t* uBuf, size_t uSize,
                                              const uint8_t* vBuf, size_t vSize,
                                              uint64_t bufferTimestamp,
                                              const bool requestKeyFrame) {
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Error: encoder (%p) ", videoEncoder_.get());
    return MCIL_MEDIA_ERROR;
  }

  return videoEncoder_->Feed(yBuf, ySize, uBuf, uSize, vBuf, vSize,
                            bufferTimestamp, requestKeyFrame);
}

void VideoEncoderAPI::Notify(const gint notification, const gint64 numValue,
        const gchar *strValue, void *payload)
{
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  if (nullptr != callback_) {
    callback_(notification, payload, userData_);
    return;
  }
}

bool VideoEncoderAPI::IsEncoderAvailable() {
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Error: encoder (%p)", videoEncoder_.get());
    return false;
  }

  return videoEncoder_->IsEncoderAvailable();
}

bool VideoEncoderAPI::UpdateEncodingResolution(uint32_t width, uint32_t height)
{
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Error: encoder (%p) ", videoEncoder_.get());
    return false;
  }

  return videoEncoder_->UpdateEncodingResolution(width, height);
}

bool VideoEncoderAPI::UpdateEncodingParams(const ENCODING_PARAMS_T* properties)
{
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Error: encoder (%p) ", videoEncoder_.get());
    return false;
  }

  return videoEncoder_->UpdateEncodingParams(properties);
}

} // namespace encoder

}  // namespace mcil
