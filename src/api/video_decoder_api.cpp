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

VideoDecoderAPI::~VideoDecoderAPI() noexcept(false) {
  if (vdec_port_index_ != -1) {
    VideoResource::GetInstance().ReleaseVideoResource(
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

  decoder_ = VideoDecoder::Create();
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Failed: decoder (%p) ", decoder_.get());
    return false;
  }
  #if defined (ENABLE_REACQUIRE)
  ResolutionChangeCb cb = [this] (uint32_t width, uint32_t height) {
               this->OnResolutionChanged(width, height); };
  decoder_->SetResolutionChangeCb(std::move(cb));
  #endif

  if (state_ == kInitialized) {
    decoder_->SetDecoderState(state_);
    state_ = kUninitialized;
  }

  codec_type_ = codec_type;
  frame_width_ = decoder_config->frameWidth;
  frame_height_ = decoder_config->frameHeight;

  return decoder_->Initialize(decoder_config, client_, client_config, 0);
}

void VideoDecoderAPI::Destroy() {
  if (vdec_port_index_ != -1)
    VideoResource::GetInstance().ReleaseVideoResource(
        V4L2_DECODER, resources_, vdec_port_index_);
  vdec_port_index_ = -1;
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }
  return decoder_->Destroy();
}

bool VideoDecoderAPI::ResetInputBuffer() {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }
  return decoder_->ResetInputBuffer();
}

bool VideoDecoderAPI::ResetDecodingBuffers(bool* reset_pending) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }
  return decoder_->ResetDecodingBuffers(reset_pending);
}

bool VideoDecoderAPI::CanNotifyResetDone() {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return true;
  }
  return decoder_->CanNotifyResetDone();
}

bool VideoDecoderAPI::DecodeBuffer(const void* buffer,
                                   size_t size,
                                   const int32_t id,
                                   int64_t buffer_pts) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }

  return decoder_->DecodeBuffer(buffer, size, id, buffer_pts);
}

bool VideoDecoderAPI::FlushInputBuffers() {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }
  return decoder_->FlushInputBuffers();
}

bool VideoDecoderAPI::DidFlushBuffersDone() {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }
  return decoder_->DidFlushBuffersDone();
}

void VideoDecoderAPI::EnqueueBuffers() {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }
  return decoder_->EnqueueBuffers();
}

void VideoDecoderAPI::DequeueBuffers() {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }
  return decoder_->DequeueBuffers();
}

void VideoDecoderAPI::ReusePictureBuffer(int32_t pic_buffer_id) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }
  return decoder_->ReusePictureBuffer(pic_buffer_id);
}

void VideoDecoderAPI::RunDecodeBufferTask(bool event_pending, bool has_output) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }

  return decoder_->RunDecodeBufferTask(event_pending, has_output);
}

void VideoDecoderAPI::RunDecoderPostTask(PostTaskType task, bool value) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }

  return decoder_->RunDecoderPostTask(task, value);
}

void VideoDecoderAPI::SetDecoderState(CodecState state) {
  if (!decoder_) {
    state_ = state;
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }
  return decoder_->SetDecoderState(state);
}

bool VideoDecoderAPI::GetCurrentInputBufferId(int32_t* buffer_id) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }
  return decoder_->GetCurrentInputBufferId(buffer_id);
}

size_t VideoDecoderAPI::GetFreeBuffersCount(QueueType queue_type) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }
  return decoder_->GetFreeBuffersCount(queue_type);
}


bool VideoDecoderAPI::AllocateOutputBuffers(
    uint32_t count, std::vector<WritableBufferRef*>& buffers) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }
  return decoder_->AllocateOutputBuffers(count, buffers);
}

bool VideoDecoderAPI::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return false;
  }

  return decoder_->CanCreateEGLImageFrom(pixel_format);
}

void VideoDecoderAPI::OnEGLImagesCreationCompleted() {
  if (!decoder_) {
    MCIL_ERROR_PRINT(" Error: decoder (%p) ", decoder_.get());
    return;
  }

  return decoder_->OnEGLImagesCreationCompleted();
}

#if defined (ENABLE_REACQUIRE)
void VideoDecoderAPI::OnResolutionChanged(uint32_t width, uint32_t height) {
  if (codec_type_ == VIDEO_CODEC_NONE) {
    MCIL_ERROR_PRINT(" codec_type_(%d) not set", codec_type_);
    return;
  }

  if (frame_width_ == width && frame_height_ == height) {
    MCIL_DEBUG_PRINT(" Resoltion did not change [%d x %d]", width, height);
    return;
  }

  frame_width_ = width;
  frame_height_ = height;
  MCIL_DEBUG_PRINT(" new resolution [%d x %d]",
                   frame_width_.load(), frame_height_.load());

  VideoResource::GetInstance().Reacquire(V4L2_DECODER,
                                         codec_type_,
                                         frame_width_,
                                         frame_height_,
                                         MCIL_MAX_FRAME_RATE,
                                         resources_,
                                         &vdec_port_index_);
  MCIL_DEBUG_PRINT(" resources_=%s", resources_.c_str());
}
#endif

}  // namespace mcil
