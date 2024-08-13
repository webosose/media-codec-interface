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
}

VideoEncoderAPI::~VideoEncoderAPI() noexcept(false) {
  if (venc_port_index_ != -1) {
    VideoResource::GetInstance().ReleaseVideoResource(
        V4L2_ENCODER, resources_, venc_port_index_);
  }
}

bool VideoEncoderAPI::Initialize(const EncoderConfig* encoder_config,
                                 EncoderClientConfig* client_config) {
  MCIL_DEBUG_PRINT(" encoder_config = %p", encoder_config);

  VideoCodec codec_type =
      VideoCodecProfileToVideoCodec(encoder_config->profile);
  if (!VideoResource::GetInstance().Acquire(V4L2_ENCODER,
                                            codec_type,
                                            encoder_config->width,
                                            encoder_config->height,
                                            encoder_config->frameRate,
                                            resources_,
                                            &venc_port_index_)) {
    MCIL_ERROR_PRINT(" Failed to acquire resources");
    return false;
  }

  encoder_ = VideoEncoder::Create();
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Failed: encoder (%p) ", encoder_.get());
    return false;
  }
  return encoder_->Initialize(
      encoder_config, client_, client_config, venc_port_index_);
}

void VideoEncoderAPI::Destroy() {
  if (venc_port_index_ != -1)
    VideoResource::GetInstance().ReleaseVideoResource(
        V4L2_ENCODER, resources_, venc_port_index_);
  venc_port_index_ = -1;
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return;
  }

  encoder_->Destroy();
}

bool VideoEncoderAPI::IsFlushSupported() {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return false;
  }
  return encoder_->IsFlushSupported();
}

bool VideoEncoderAPI::EncodeFrame(scoped_refptr<VideoFrame> frame,
                                  bool force_keyframe) {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return false;
  }

  return encoder_->EncodeFrame(std::move(frame), force_keyframe);
}

bool VideoEncoderAPI::FlushFrames() {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return false;
  }

  return encoder_->FlushFrames();
}

bool VideoEncoderAPI::EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                                   const uint8_t* uBuf, size_t uSize,
                                   const uint8_t* vBuf, size_t vSize,
                                   uint64_t bufferTimestamp,
                                   bool requestKeyFrame) {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return false;
  }

  return encoder_->EncodeBuffer(yBuf, ySize, uBuf, uSize, vBuf, vSize,
                                      bufferTimestamp, requestKeyFrame);
}

bool VideoEncoderAPI::UpdateEncodingParams(
    uint32_t bitrate, uint32_t framerate) {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return false;
  }

  return encoder_->UpdateEncodingParams(bitrate, framerate);
}

bool VideoEncoderAPI::StartDevicePoll() {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return false;
  }

  return encoder_->StartDevicePoll();
}

void VideoEncoderAPI::RunEncodeBufferTask() {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return;
  }

  encoder_->RunEncodeBufferTask();
}

void VideoEncoderAPI::SendStartCommand(bool start) {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return;
  }

  encoder_->SendStartCommand(start);
}

void VideoEncoderAPI::SetEncoderState(CodecState state) {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return;
  }

  encoder_->SetEncoderState(state);
}

size_t VideoEncoderAPI::GetFreeBuffersCount(QueueType queue_type) {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return 0;
  }

  return encoder_->GetFreeBuffersCount(queue_type);
}

void VideoEncoderAPI::EnqueueBuffers() {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return;
  }

  encoder_->EnqueueBuffers();
}

scoped_refptr<VideoFrame> VideoEncoderAPI::GetDeviceInputFrame() {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return nullptr;
  }
  return encoder_->GetDeviceInputFrame();
}

bool VideoEncoderAPI::NegotiateInputFormat(VideoPixelFormat format,
                                           const Size& frame_size) {
  if (!encoder_) {
    MCIL_ERROR_PRINT(" Error: encoder (%p) ", encoder_.get());
    return false;
  }
  return encoder_->NegotiateInputFormat(format, frame_size);
}

}  // namespace mcil
