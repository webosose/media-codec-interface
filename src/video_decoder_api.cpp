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
#include "resource/video_resource.h"

#define MCIL_MAX_FRAME_RATE 60 // Set to maximum framerate supported in webcodec

namespace mcil {

SupportedProfiles VideoDecoderAPI::GetSupportedProfiles() {
  MCIL_INFO_PRINT(" ");
  return VideoDecoder::GetSupportedProfiles();
}

VideoDecoderAPI::VideoDecoderAPI(VideoDecoderClient* client)
  : client_(client) {
  MCIL_INFO_PRINT(" Ctor ");
}

VideoDecoderAPI::~VideoDecoderAPI() {
  MCIL_INFO_PRINT(" Dtor ");
}

bool VideoDecoderAPI::Initialize(const DecoderConfig* decoder_config,
                                 VideoPixelFormat* output_pix_fmt,
                                 bool* should_control_buffer_feed) {
  MCIL_INFO_PRINT(" decoder_config = %p", decoder_config);
  MCIL_INFO_PRINT(" frameWidth = %d", decoder_config->frameWidth);
  MCIL_INFO_PRINT(" frameHeight = %d", decoder_config->frameHeight);
  MCIL_INFO_PRINT(" codecType = %d", decoder_config->codecType);

  if (!VideoResource::GetInstance().Acquire(V4L2_DECODER,
                                            decoder_config->codecType,
                                            decoder_config->frameWidth,
                                            decoder_config->frameHeight,
                                            MCIL_MAX_FRAME_RATE,
                                            resources_,
                                            &vdec_port_index_)) {
    MCIL_INFO_PRINT(" Failed to acquire resources");
    return false;
  }

  video_decoder_ = VideoDecoder::Create(decoder_config->codecType);
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }

  if (state_ == kInitialized) {
    video_decoder_->SetDecoderState(state_);
    state_ = kUninitialized;
  }

  return video_decoder_ ->Initialize(decoder_config, client_,
      output_pix_fmt, should_control_buffer_feed, vdec_port_index_);
}

void VideoDecoderAPI::Destroy() {
  if (vdec_port_index_ != -1)
    VideoResource::GetInstance().Release(V4L2_DECODER,
                                         resources_,
                                         vdec_port_index_);

  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }

  return video_decoder_->Destroy();
}

bool VideoDecoderAPI::ResetInputBuffer() {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->ResetInputBuffer();
}

bool VideoDecoderAPI::ResetDecodingBuffers(bool* reset_pending) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->ResetDecodingBuffers(reset_pending);
}

bool VideoDecoderAPI::CanNotifyResetDone() {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return true;
  }
  return video_decoder_->CanNotifyResetDone();
}

bool VideoDecoderAPI::FeedBuffers(
    const void* buffer, size_t size, const int32_t id, int64_t buffer_pts) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->FeedBuffers(buffer, size, id, buffer_pts);
}

bool VideoDecoderAPI::FlushInputBuffers() {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->FlushInputBuffers();
}

bool VideoDecoderAPI::DidFlushBuffersDone() {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->DidFlushBuffersDone();
}

void VideoDecoderAPI::EnqueueBuffers() {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }
  return video_decoder_->EnqueueBuffers();
}

void VideoDecoderAPI::DequeueBuffers() {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }
  return video_decoder_->DequeueBuffers();
}

void VideoDecoderAPI::ReusePictureBuffer(int32_t pic_buffer_id) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }
  return video_decoder_->ReusePictureBuffer(pic_buffer_id);
}

void VideoDecoderAPI::RunDecodeBufferTask(bool event_pending, bool has_output) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }

  return video_decoder_->RunDecodeBufferTask(event_pending, has_output);
}

void VideoDecoderAPI::RunDecoderPostTask(PostTaskType task, bool value) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }

  return video_decoder_->RunDecoderPostTask(task, value);
}

void VideoDecoderAPI::SetDecoderState(DecoderState state) {
  if (!video_decoder_) {
    state_ == state;
    MCIL_INFO_PRINT(" decoder is not created or null. state[%d]", state);
    return;
  }
  return video_decoder_->SetDecoderState(state);
}

bool VideoDecoderAPI::GetCurrentInputBufferId(int32_t* buffer_id) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->GetCurrentInputBufferId(buffer_id);
}

size_t VideoDecoderAPI::GetFreeBuffersCount(QueueType queue_type) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->GetFreeBuffersCount(queue_type);
}


bool VideoDecoderAPI::AllocateOutputBuffers(
    uint32_t count, std::vector<WritableBufferRef*>& buffers) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }
  return video_decoder_->AllocateOutputBuffers(count, buffers);
}

bool VideoDecoderAPI::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return false;
  }

  return video_decoder_->CanCreateEGLImageFrom(pixel_format);
}

void VideoDecoderAPI::OnEGLImagesCreationCompleted() {
  if (!video_decoder_) {
    MCIL_INFO_PRINT(" decoder is not created or null.");
    return;
  }

  return video_decoder_->OnEGLImagesCreationCompleted();
}

}  // namespace mcil
