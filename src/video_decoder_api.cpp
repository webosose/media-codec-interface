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
#include "base/vdec_resource_handler.h"

namespace mcil {

namespace decoder {

 mcil::SupportedProfiles VideoDecoderAPI::GetSupportedProfiles() {
  MCIL_INFO_PRINT(" ");
  return mcil::decoder::VideoDecoder::GetSupportedProfiles();
}

VideoDecoderAPI::VideoDecoderAPI(VideoDecoderDelegate* delegate)
  : delegate_(delegate) {
  MCIL_INFO_PRINT(" Ctor ");
}

VideoDecoderAPI::~VideoDecoderAPI() {
  MCIL_INFO_PRINT(" Dtor ");
}

bool VideoDecoderAPI::Initialize(const DecoderConfig* decoderConfig,
                                 VideoPixelFormat* output_pix_fmt,
                                 bool* should_control_buffer_feed) {
  MCIL_INFO_PRINT(" decoderConfig = %p", decoderConfig);
  MCIL_INFO_PRINT(" frameWidth = %d", decoderConfig->frameWidth);
  MCIL_INFO_PRINT(" frameHeight = %d", decoderConfig->frameHeight);
  MCIL_INFO_PRINT(" codecType = %d", decoderConfig->codecType);

  if (!mcil::decoder::VdecResourceHandler::getInstance().SetupResource(decoderConfig, resources_, &vdec_port_index_)) {
    MCIL_INFO_PRINT(" Failed to acquire resources");
    return false;
  }

  videoDecoder_ = mcil::decoder::VideoDecoder::Create(decoderConfig->codecType);
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }

  if (state_ == kInitialized) {
    videoDecoder_->SetDecoderState(state_);
    state_ = kUninitialized;
  }

  return videoDecoder_ ->Initialize(
      decoderConfig, delegate_, output_pix_fmt, should_control_buffer_feed, vdec_port_index_);
}

bool VideoDecoderAPI::DoReset(bool full_reset, bool* reset_pending) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->DoReset(full_reset, reset_pending);
}

void VideoDecoderAPI::Destroy() {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }
  return videoDecoder_->Destroy();
}

bool VideoDecoderAPI::FeedBuffers(
    const void* buffer, size_t size, const int32_t id, uint64_t buffer_pts) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->FeedBuffers(buffer, size, id, buffer_pts);
}

bool VideoDecoderAPI::FlushBuffers() {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->FlushBuffers();
}

bool VideoDecoderAPI::DidFlushBuffersDone() {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->DidFlushBuffersDone();
}

bool VideoDecoderAPI::EnqueueBuffers() {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->EnqueueBuffers();
}

bool VideoDecoderAPI::DequeueBuffers() {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->DequeueBuffers();
}

void VideoDecoderAPI::ReusePictureBuffer(int32_t pic_buffer_id) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }
  return videoDecoder_->ReusePictureBuffer(pic_buffer_id);
}

bool VideoDecoderAPI::StartDevicePoll(bool poll_device, bool* event_pending) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->StartDevicePoll(poll_device, event_pending);
}

void VideoDecoderAPI::RunDecodeBufferTask(bool event_pending, bool has_output) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p) ", videoDecoder_.get());
    return;
  }

  return videoDecoder_->RunDecodeBufferTask(event_pending, has_output);
}

void VideoDecoderAPI::RunDecoderPostTask(PostTaskType task, bool value) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p) ", videoDecoder_.get());
    return;
  }

  return videoDecoder_->RunDecoderPostTask(task, value);
}

void VideoDecoderAPI::SetDecoderState(DecoderState state) {
  if (!videoDecoder_) {
    state_ == state;
    MCIL_INFO_PRINT(" decoder is not created or null. state[%d]", state);
    return;
  }
  return videoDecoder_->SetDecoderState(state);
}

int64_t VideoDecoderAPI::GetCurrentInputBufferId() {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->GetCurrentInputBufferId();
}

size_t VideoDecoderAPI::GetFreeBuffersCount(QueueType queue_type) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return videoDecoder_->GetFreeBuffersCount(queue_type);
}

std::vector<WritableBufferRef*>
VideoDecoderAPI::AllocateOutputBuffers(std::vector<OutputRecord>* records) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return empty_output_buffer;
  }
  return videoDecoder_->AllocateOutputBuffers(records);
}

bool VideoDecoderAPI::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }

  return videoDecoder_->CanCreateEGLImageFrom(pixel_format);
}

void VideoDecoderAPI::OnEGLImagesCreationCompleted() {
  if (!videoDecoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }

  return videoDecoder_->OnEGLImagesCreationCompleted();
}

} // namespace decoder

}  // namespace mcil
