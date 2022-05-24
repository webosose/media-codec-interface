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

#include "decoder_types.h"
#include "video_buffers.h"

namespace mcil {

class VideoDecoderClient;

class VideoDecoder : public RefCounted<VideoDecoder> {
 public:
  static SupportedProfiles GetSupportedProfiles();
  static scoped_refptr<VideoDecoder> Create();

  virtual bool Initialize(const DecoderConfig* config,
                          VideoDecoderClient* client,
                          VideoPixelFormat* output_pix_fmt,
                          int32_t vdec_port_index,
                          bool* should_control_buffer_feed);
  virtual void Destroy();
  virtual bool ResetInputBuffer();
  virtual bool ResetDecodingBuffers(bool* reset_pending);
  virtual bool CanNotifyResetDone();

  virtual bool DecodeBuffer(const void* buffer, size_t size,
                            const int32_t id, int64_t buffer_pts);
  virtual bool FlushInputBuffers();
  virtual bool DidFlushBuffersDone();

  virtual void EnqueueBuffers();
  virtual void DequeueBuffers();
  virtual void ReusePictureBuffer(int32_t pic_buffer_id);

  virtual void RunDecodeBufferTask(bool event_pending, bool has_output);
  virtual void RunDecoderPostTask(PostTaskType task, bool value);

  virtual void SetDecoderState(CodecState state);

  virtual bool GetCurrentInputBufferId(int32_t* buffer_id);
  virtual size_t GetFreeBuffersCount(QueueType queue_type);
  virtual bool AllocateOutputBuffers(
      uint32_t count, std::vector<WritableBufferRef*>& buffers);
  virtual bool CanCreateEGLImageFrom(VideoPixelFormat pixel_format);
  virtual void OnEGLImagesCreationCompleted();

 protected:
  friend class RefCounted<VideoDecoder>;

  VideoDecoder();
  virtual ~VideoDecoder();
};

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_DECODER_H_
