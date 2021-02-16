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

bool VideoEncoderAPI::IsCodecSupported(MCP_VIDEO_CODEC videoCodec) {
  MCP_INFO_PRINT("%d %s videoCodec(%d)", __LINE__, __FUNCTION__, videoCodec);
  return (videoCodec == MCP_VIDEO_CODEC_H264);
}

VideoEncoderAPI::VideoEncoderAPI() {
  MCP_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

VideoEncoderAPI::~VideoEncoderAPI() {
  MCP_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

void VideoEncoderAPI::RegisterCallback(
    ENCODER_CALLBACK_T callback, void *uData) {
  MCP_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  callback_ = callback;
  userData_ = uData;
}

bool VideoEncoderAPI::Init(const ENCODER_INIT_DATA_T* loadData,
                              NEWFRAME_CALLBACK_T new_frame_cb) {
  MCP_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  MCP_INFO_PRINT("Load loadData = %p", loadData);
  MCP_INFO_PRINT("Load width = %d", loadData->width);
  MCP_INFO_PRINT("Load height = %d", loadData->height);
  MCP_INFO_PRINT("Load FrameRate = %d", loadData->frameRate);
  MCP_INFO_PRINT("Load pixelFormat = %d", loadData->pixelFormat);

  videoEncoder_ = mcil::encoder::VideoEncoder::Create(loadData->codecType);
  if (!videoEncoder_) {
    MCP_INFO_PRINT("Encoder Buffer not created");
    return false;
  }
  bool ret = videoEncoder_ ->Init(loadData, new_frame_cb);
  videoEncoder_->RegisterCbFunction (
          std::bind(&VideoEncoderAPI::Notify, this ,
              std::placeholders::_1, std::placeholders::_2,
              std::placeholders::_3, std::placeholders::_4));
  videoEncoder_ -> RegisterCallBack(
          std::bind(&VideoEncoderAPI::OnEncodedDataAvailable, this,
              std::placeholders::_1, std::placeholders::_2));

  return ret;
}

bool VideoEncoderAPI::Deinit() {
  MCP_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  return videoEncoder_ ->Deinit();
}

MCP_MEDIA_STATUS_T VideoEncoderAPI::Encode(const uint8_t* bufferPtr,
                                              size_t bufferSize) {
  if (!videoEncoder_) {
    MCP_INFO_PRINT(" Error: encoder (%p) should Init", videoEncoder_.get());
    return MCP_MEDIA_ERROR;
  }
  return videoEncoder_->Feed(bufferPtr, bufferSize);
}

MCP_MEDIA_STATUS_T VideoEncoderAPI::Encode(const uint8_t* yBuf, size_t ySize,
                                              const uint8_t* uBuf, size_t uSize,
                                              const uint8_t* vBuf, size_t vSize,
                                              uint64_t bufferTimestamp,
                                              const bool requestKeyFrame) {
  if (!videoEncoder_) {
    MCP_INFO_PRINT(" Error: encoder (%p) should Init", videoEncoder_.get());
    return MCP_MEDIA_ERROR;
  }
  
  return videoEncoder_->Feed(yBuf, ySize, uBuf, uSize, vBuf, vSize,
                            bufferTimestamp, requestKeyFrame);
}

bool VideoEncoderAPI::OnEncodedDataAvailable(uint8_t* buffer,
                                                ENCODED_BUFFER_T* encData) {
  bool res;

  if (nullptr != callback_) {
    encData->encodedBuffer = buffer;
    callback_(ENCODER_CB_BUFFER_ENCODED, encData, userData_);
    return true;
  }
  return false;
}

void VideoEncoderAPI::Notify(const gint notification, const gint64 numValue,
        const gchar *strValue, void *payload)
{
  MCP_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  if (nullptr != callback_) {
    callback_(notification, payload, userData_);
    return;
  }
}

bool VideoEncoderAPI::UpdateEncodingResolution(uint32_t width, uint32_t height)
{
  if (!videoEncoder_) {
    MCP_INFO_PRINT(" Error: encoder (%p) should Init", videoEncoder_.get());
    return false;
  }

  return videoEncoder_->UpdateEncodingResolution(width, height);
}

bool VideoEncoderAPI::UpdateEncodingParams(const ENCODING_PARAMS_T* properties)
{
  //Dynamic updation of bitrate and framerate needs to be handled
  return true;
}

} // namespace encoder

}  // namespace mcil
