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

#include "video_encoder_api.h"

#include <functional>
#include <vector>
#include <map>

#include <base/log.h>

#include "base/encoder_types.h"
#include "base/video_encoder.h"
#include "resource/video_resource.h"

namespace mcil {

// static
SupportedProfiles VideoEncoderAPI::GetSupportedProfiles() {
  return VideoEncoder::GetSupportedProfiles();
}

VideoEncoderAPI::VideoEncoderAPI(VideoEncoderClient* client)
  : client_(client) {
  MCIL_DEBUG_PRINT(" Ctor");
}

VideoEncoderAPI::~VideoEncoderAPI() {
  MCIL_DEBUG_PRINT(" Dtor");
  if (venc_port_index_ != -1)
    VideoResource::GetInstance().Release(V4L2_ENCODER,
                                         resources_,
                                         venc_port_index_);
}

bool VideoEncoderAPI::Initialize(const EncoderConfig* configData) {
  MCIL_DEBUG_PRINT(" encoder_config = %p", configData);

  if (!VideoResource::GetInstance().Acquire(V4L2_ENCODER,
                                            configData->codecType,
                                            configData->width,
                                            configData->height,
                                            configData->frameRate,
                                            resources_,
                                            &venc_port_index_)) {
    MCIL_INFO_PRINT(" Failed to acquire resources");
    return false;
  }

  videoEncoder_ = VideoEncoder::Create(configData->codecType);
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Encoder is not created or null.");
    return false;
  }
  return videoEncoder_ ->Initialize(configData, client_);
}

bool VideoEncoderAPI::Destroy() {
  if (venc_port_index_ != -1)
    VideoResource::GetInstance().Release(V4L2_ENCODER,
                                         resources_,
                                         venc_port_index_);

  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Encoder is not created or null.");
    return true;
  }

  return videoEncoder_ ->Destroy();
}

bool VideoEncoderAPI::EncodeBuffers(const uint8_t* yBuf, size_t ySize,
                                    const uint8_t* uBuf, size_t uSize,
                                    const uint8_t* vBuf, size_t vSize,
                                    uint64_t bufferTimestamp,
                                    const bool requestKeyFrame) {
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Encoder is not created or null.");
    return false;
  }

  return videoEncoder_->EncodeBuffers(yBuf, ySize, uBuf, uSize, vBuf, vSize,
                                      bufferTimestamp, requestKeyFrame);
}

bool VideoEncoderAPI::IsEncoderAvailable() {
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Encoder is not created or null.");
    return false;
  }

  return videoEncoder_->IsEncoderAvailable();
}

bool VideoEncoderAPI::UpdateEncodingResolution(uint32_t width, uint32_t height) {
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Encoder is not created or null.");
    return false;
  }

  return videoEncoder_->UpdateEncodingResolution(width, height);
}

bool VideoEncoderAPI::UpdateEncodingParams(const EncodingParams* properties) {
  if (!videoEncoder_) {
    MCIL_INFO_PRINT(" Encoder is not created or null.");
    return false;
  }

  return videoEncoder_->UpdateEncodingParams(properties);
}

}  // namespace mcil
