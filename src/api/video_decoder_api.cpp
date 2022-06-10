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

#include "video_decoder_api.h"

#include <functional>
#include <vector>
#include <map>

#include <base/log.h>

#include "base/decoder_types.h"
#include "base/video_decoder.h"
#include "resource/video_resource.h"

#define MCIL_MAX_FRAME_RATE 60 //Set to maximum framerate supported in webcodec

namespace mcil {

// static
SupportedProfiles VideoDecoderAPI::GetSupportedProfiles() {
  return VideoDecoder::GetSupportedProfiles();
}

VideoDecoderAPI::VideoDecoderAPI(VideoDecoderClient* client)
  : client_(client) {
}

VideoDecoderAPI::~VideoDecoderAPI() {
  if (vdec_port_index_ != -1) {
    VideoResource::GetInstance().Release(
        V4L2_DECODER, resources_, vdec_port_index_);
  }
}

bool VideoDecoderAPI::Initialize(const DecoderConfig* decoder_config,
                                 DecoderClientConfig* client_config) {
  MCIL_DEBUG_PRINT(" decoder_config = %p", decoder_config);
  VideoCodec codec_type =
      VideoCodecProfileToVideoCodec(decoder_config->profile);
  if (!VideoResource::GetInstance().Acquire(V4L2_DECODER,
                                            codec_type,
                                            decoder_config->frameWidth,
                                            decoder_config->frameHeight,
                                            MCIL_MAX_FRAME_RATE,
                                            resources_,
                                            &vdec_port_index_)) {
    MCIL_ERROR_PRINT(" Failed to acquire resources");
    return false;
  }

  video_decoder_ = VideoDecoder::Create();
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Failed: decoder (%p) ", video_decoder_.get());
    return false;
  }

  if (state_ == kInitialized) {
    video_decoder_->SetDecoderState(state_);
    state_ = kUninitialized;
  }

  return video_decoder_ ->Initialize(
      decoder_config, client_, client_config, vdec_port_index_);
}

void VideoDecoderAPI::Destroy() {
  if (vdec_port_index_ != -1)
    VideoResource::GetInstance().Release(
        V4L2_DECODER, resources_, vdec_port_index_);
  vdec_port_index_ = -1;
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }
  return video_decoder_->Destroy();
}

bool VideoDecoderAPI::ResetInputBuffer() {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->ResetInputBuffer();
}

bool VideoDecoderAPI::ResetDecodingBuffers(bool* reset_pending) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->ResetDecodingBuffers(reset_pending);
}

bool VideoDecoderAPI::CanNotifyResetDone() {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return true;
  }
  return video_decoder_->CanNotifyResetDone();
}

bool VideoDecoderAPI::DecodeBuffer(const void* buffer,
                                   size_t size,
                                   const int32_t id,
                                   int64_t buffer_pts) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->DecodeBuffer(buffer, size, id, buffer_pts);
}

bool VideoDecoderAPI::FlushInputBuffers() {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->FlushInputBuffers();
}

bool VideoDecoderAPI::DidFlushBuffersDone() {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->DidFlushBuffersDone();
}

void VideoDecoderAPI::EnqueueBuffers() {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }
  return video_decoder_->EnqueueBuffers();
}

void VideoDecoderAPI::DequeueBuffers() {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }
  return video_decoder_->DequeueBuffers();
}

void VideoDecoderAPI::ReusePictureBuffer(int32_t pic_buffer_id) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }
  return video_decoder_->ReusePictureBuffer(pic_buffer_id);
}

void VideoDecoderAPI::RunDecodeBufferTask(bool event_pending, bool has_output) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }

  return video_decoder_->RunDecodeBufferTask(event_pending, has_output);
}

void VideoDecoderAPI::RunDecoderPostTask(PostTaskType task, bool value) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }

  return video_decoder_->RunDecoderPostTask(task, value);
}

void VideoDecoderAPI::SetDecoderState(CodecState state) {
  if (!video_decoder_) {
    state_ == state;
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }
  return video_decoder_->SetDecoderState(state);
}

bool VideoDecoderAPI::GetCurrentInputBufferId(int32_t* buffer_id) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->GetCurrentInputBufferId(buffer_id);
}

size_t VideoDecoderAPI::GetFreeBuffersCount(QueueType queue_type) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->GetFreeBuffersCount(queue_type);
}


bool VideoDecoderAPI::AllocateOutputBuffers(
    uint32_t count, std::vector<WritableBufferRef*>& buffers) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }
  return video_decoder_->AllocateOutputBuffers(count, buffers);
}

bool VideoDecoderAPI::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return false;
  }

  return video_decoder_->CanCreateEGLImageFrom(pixel_format);
}

void VideoDecoderAPI::OnEGLImagesCreationCompleted() {
  if (!video_decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", video_decoder_.get());
    return;
  }

  return video_decoder_->OnEGLImagesCreationCompleted();
}

}  // namespace mcil
