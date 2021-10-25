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

#include "video_decoder_api.h"
#include "base/decoder_types.h"
#include "base/video_decoder.h"

#ifdef MCIL_INFO_PRINT
#undef MCIL_INFO_PRINT
#endif
#define MCIL_INFO_PRINT(...) void(0)

namespace mcil {

namespace decoder {

 mcil::SupportedProfiles VideoDecoderAPI::GetSupportedProfiles() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  return mcil::decoder::VideoDecoder::GetSupportedProfiles();
}

VideoDecoderAPI::VideoDecoderAPI(VideoDecoderDelegate* delegate)
  : delegate_(delegate) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

VideoDecoderAPI::~VideoDecoderAPI() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

bool VideoDecoderAPI::Initialize(const DecoderConfig* decoderConfig,
                                 VideoPixelFormat* output_pix_fmt,
                                 bool* consider_egl_image_creation) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  MCIL_INFO_PRINT("Load decoderConfig = %p", decoderConfig);
  MCIL_INFO_PRINT("Load frameWidth = %d", decoderConfig->frameWidth);
  MCIL_INFO_PRINT("Load frameHeight = %d", decoderConfig->frameHeight);
  MCIL_INFO_PRINT("Load codecType = %d", decoderConfig->codecType);

  videoDecoder_ = mcil::decoder::VideoDecoder::Create(decoderConfig->codecType);
  if (!videoDecoder_) {
    MCIL_INFO_PRINT("Encoder Buffer not created");
    return false;
  }

  return videoDecoder_ ->Initialize(
      decoderConfig, delegate_, output_pix_fmt, consider_egl_image_creation);
}

bool VideoDecoderAPI::DoReset(bool full_reset, bool* reset_pending) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->DoReset(full_reset, reset_pending);
}

void VideoDecoderAPI::Destroy() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return;
  }
  return videoDecoder_->Destroy();
}

bool VideoDecoderAPI::FeedBuffers(
    const void* buffer, size_t size, const int32_t id, int64_t buffer_pts) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->FeedBuffers(buffer, size, id, buffer_pts);
}

bool VideoDecoderAPI::FlushBuffers() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->FlushBuffers();
}

bool VideoDecoderAPI::DidFlushBuffersDone() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->DidFlushBuffersDone();
}

bool VideoDecoderAPI::EnqueueBuffers() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->EnqueueBuffers();
}

bool VideoDecoderAPI::DequeueBuffers() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->DequeueBuffers();
}

bool VideoDecoderAPI::StartDevicePoll(bool poll_device, bool* event_pending) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->StartDevicePoll(poll_device, event_pending);
}

void VideoDecoderAPI::RunDecodeBufferTask(bool event_pending) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p) ", videoDecoder_.get());
    return;
  }

  return videoDecoder_->RunDecodeBufferTask(event_pending);
}

void VideoDecoderAPI::SetDecoderState(DecoderState state) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return;
  }
  return videoDecoder_->SetDecoderState(state);
}

int64_t VideoDecoderAPI::GetCurrentInputBufferId() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->GetCurrentInputBufferId();
}

size_t VideoDecoderAPI::GetFreeBuffersCount(QueueType queue_type) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return false;
  }
  return videoDecoder_->GetFreeBuffersCount(queue_type);
}

std::vector<WritableBufferRef*>
VideoDecoderAPI::AllocateOutputBuffers(std::vector<OutputRecord>* records) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p)", videoDecoder_.get());
    return empty_output_buffer;
  }
  return videoDecoder_->AllocateOutputBuffers(records);
}

bool VideoDecoderAPI::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p) ", videoDecoder_.get());
    return false;
  }

  return videoDecoder_->CanCreateEGLImageFrom(pixel_format);
}

void VideoDecoderAPI::OnEGLImagesCreationCompleted() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);

  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p) ", videoDecoder_.get());
    return;
  }

  return videoDecoder_->OnEGLImagesCreationCompleted();
}

} // namespace decoder

}  // namespace mcil
