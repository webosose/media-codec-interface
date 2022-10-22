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

#ifndef SRC_BASE_VIDEO_DECODER_H_
#define SRC_BASE_VIDEO_DECODER_H_

#include <functional>

#include "decoder_types.h"
#include "video_buffers.h"

namespace mcil {

class VideoDecoderClient;

using ResolutionChangeCb = std::function<void(uint32_t, uint32_t)>;

class VideoDecoder : public RefCounted<VideoDecoder> {
 public:
  static SupportedProfiles GetSupportedProfiles();
  static scoped_refptr<VideoDecoder> Create();

  virtual bool Initialize(const DecoderConfig* config,
                          VideoDecoderClient* client,
                          DecoderClientConfig* client_config,
                          int32_t vdec_port_index) = 0;
  virtual void Destroy() = 0;
  virtual bool ResetInputBuffer() = 0;
  virtual bool ResetDecodingBuffers(bool* reset_pending) = 0;
  virtual bool CanNotifyResetDone() = 0;

  virtual bool DecodeBuffer(const void* buffer, size_t size,
                            const int32_t id, int64_t buffer_pts) = 0;
  virtual bool FlushInputBuffers() = 0;
  virtual bool DidFlushBuffersDone() = 0;

  virtual void EnqueueBuffers() = 0;
  virtual void DequeueBuffers() = 0;
  virtual void ReusePictureBuffer(int32_t pic_buffer_id) = 0;

  virtual void RunDecodeBufferTask(bool event_pending, bool has_output) = 0;
  virtual void SetDecoderState(CodecState state) = 0;

  virtual bool GetCurrentInputBufferId(int32_t* buffer_id) = 0;
  virtual size_t GetFreeBuffersCount(QueueType queue_type) = 0;
  virtual bool AllocateOutputBuffers(
      uint32_t count, std::vector<WritableBufferRef*>& buffers) = 0;
  virtual bool CanCreateEGLImageFrom(VideoPixelFormat pixel_format) = 0;

  virtual void RunDecoderPostTask(PostTaskType task, bool value) = 0;
  virtual void OnEGLImagesCreationCompleted() = 0;

  virtual void SetResolutionChangeCb(ResolutionChangeCb cb) {}

 protected:
  friend class RefCounted<VideoDecoder>;

  VideoDecoder() = default;
  virtual ~VideoDecoder() = default;
};

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_DECODER_H_
