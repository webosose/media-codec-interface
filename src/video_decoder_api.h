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


#ifndef SRC_VIDEO_DECODER_API_H_
#define SRC_VIDEO_DECODER_API_H_

#include "decoder_types.h"

namespace mcil {

namespace decoder {

class VideoDecoder;
class VideoDecoderDelegate;

class VideoDecoderAPI {
 public:
  VideoDecoderAPI(VideoDecoderDelegate* delegate);
  ~VideoDecoderAPI();

  static mcil::SupportedProfiles GetSupportedProfiles();

  bool Initialize(const DecoderConfig* decoderConfig,
                  VideoPixelFormat* output_pix_fmt,
                  bool* consider_egl_image_creation);
  bool DoReset(bool full_reset, bool* reset_pending);
  void Destroy();

  bool FeedBuffers(const void* buffer, size_t size,
                   const int32_t id, int64_t buffer_pts);
  bool FlushBuffers();
  bool DidFlushBuffersDone();

  bool EnqueueBuffers();
  bool DequeueBuffers();

  bool StartDevicePoll(bool poll_device, bool* event_pending);
  void RunDecodeBufferTask(bool event_pending);

  void SetDecoderState(DecoderState state);
  int64_t GetCurrentInputBufferId();
  size_t GetFreeBuffersCount(QueueType queue_type);
  std::vector<WritableBufferRef*> AllocateOutputBuffers(std::vector<OutputRecord>*);
  bool CanCreateEGLImageFrom(VideoPixelFormat pixel_format);
  void OnEGLImagesCreationCompleted();

 private:
  VideoDecoderDelegate* delegate_;
  std::vector<WritableBufferRef*> empty_output_buffer;
  std::shared_ptr<mcil::decoder::VideoDecoder> videoDecoder_;
};

}  // namespace decoder

}  // namespace mcil

#endif  // SRC_VIDEO_DECODER_API_H_
