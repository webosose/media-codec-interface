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

#include "video_decoder_api.h"
#include "decoder_types.h"
#include "video_decoder_delegate.h"

namespace mcil {

namespace decoder {

class VideoDecoder {
 public:
  static std::shared_ptr<VideoDecoder> Create(VideoCodecType type);
  static mcil::SupportedProfiles GetSupportedProfiles();

  VideoDecoder();
  ~VideoDecoder();

  virtual bool Initialize(const DecoderConfig* config,
                          VideoDecoderDelegate* delegate,
                          VideoPixelFormat* output_pix_fmt,
                          bool* should_control_buffer_feed,
                          int32_t vdec_port_index);
  virtual bool DoReset(bool full_reset, bool* reset_pending);
  virtual void Destroy();

  virtual bool FeedBuffers(const void* buffer, size_t size,
                           const int32_t id, uint64_t buffer_pts);
  virtual bool FlushBuffers();
  virtual bool DidFlushBuffersDone();

  virtual bool EnqueueBuffers();
  virtual bool DequeueBuffers();
  virtual void ReusePictureBuffer(int32_t pic_buffer_id);

  virtual bool StartDevicePoll(bool poll_device, bool* event_pending);

  virtual void RunDecodeBufferTask(bool event_pending, bool has_output);
  virtual void RunDecoderPostTask(PostTaskType task, bool value);

  virtual void SetDecoderState(DecoderState state);

  virtual int64_t GetCurrentInputBufferId();
  virtual size_t GetFreeBuffersCount(QueueType queue_type);
  virtual std::vector<WritableBufferRef*> AllocateOutputBuffers(
      std::vector<OutputRecord>* output_records);
  virtual bool CanCreateEGLImageFrom(VideoPixelFormat pixel_format);
  virtual void OnEGLImagesCreationCompleted();

 private:

  std::vector<WritableBufferRef*> empty_output_buffer;
  VideoDecoderDelegate* delegate_ = nullptr;
};

}  // namespace decoder

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_DECODER_H_
