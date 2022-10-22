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

#include <atomic>

#include "decoder_types.h"
#include "video_buffers.h"

namespace mcil {

class VideoDecoder;
class VideoDecoderClient;

class VideoDecoderAPI {
 public:
  VideoDecoderAPI(VideoDecoderClient* client);
  ~VideoDecoderAPI();

  static SupportedProfiles GetSupportedProfiles();

  bool Initialize(const DecoderConfig* decoder_config,
                  DecoderClientConfig* client_config);
  void Destroy();
  bool ResetInputBuffer();
  bool ResetDecodingBuffers(bool* reset_pending);
  bool CanNotifyResetDone();

  bool DecodeBuffer(const void* buffer, size_t size,
                    const int32_t id, int64_t buffer_pts);
  bool FlushInputBuffers();
  bool DidFlushBuffersDone();

  void EnqueueBuffers();
  void DequeueBuffers();
  void ReusePictureBuffer(int32_t pic_buffer_id);

  void RunDecodeBufferTask(bool event_pending, bool has_output);
  void RunDecoderPostTask(PostTaskType task, bool value);

  void SetDecoderState(CodecState state);
  bool GetCurrentInputBufferId(int32_t* buffer_id);
  size_t GetFreeBuffersCount(QueueType queue_type);
  bool AllocateOutputBuffers(uint32_t count,
                             std::vector<WritableBufferRef*>& buffers);
  bool CanCreateEGLImageFrom(VideoPixelFormat pixel_format);
  void OnEGLImagesCreationCompleted();

 private:
  void OnResolutionChanged(uint32_t width, uint32_t height);

  VideoDecoderClient* client_;

  scoped_refptr<VideoDecoder> video_decoder_;

  VideoCodec codec_type_ = VIDEO_CODEC_NONE;

  std::atomic<uint32_t> frame_width_{0};
  std::atomic<uint32_t> frame_height_{0};

  int32_t vdec_port_index_ = -1;
  bool resource_requested_ = false;

  std::string resources_ = "";

  CodecState state_ = kUninitialized;
};

}  // namespace mcil

#endif  // SRC_VIDEO_DECODER_API_H_
