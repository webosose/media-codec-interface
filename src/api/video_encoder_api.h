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

#ifndef SRC_VIDEO_ENCODER_API_H_
#define SRC_VIDEO_ENCODER_API_H_

#include "encoder_types.h"

namespace mcil {

class VideoEncoder;
class VideoEncoderClient;

class VideoEncoderAPI {
 public:
  static SupportedProfiles GetSupportedProfiles();

  VideoEncoderAPI(VideoEncoderClient* client);
  ~VideoEncoderAPI() noexcept(false);

  bool Initialize(const EncoderConfig* configData,
                  EncoderClientConfig* client_config);
  void Destroy();
  bool IsFlushSupported();
  bool EncodeFrame(scoped_refptr<VideoFrame> frame,
                   bool force_keyframe);
  bool FlushFrames();
  bool EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                    const uint8_t* uBuf, size_t uSize,
                    const uint8_t* vBuf, size_t vSize,
                    uint64_t bufferTimestamp,
                    bool requestKeyFrame);
  bool UpdateEncodingParams(uint32_t bitrate, uint32_t framerate);
  bool StartDevicePoll();
  void RunEncodeBufferTask();
  void SendStartCommand(bool start);
  void SetEncoderState(CodecState state);
  size_t GetFreeBuffersCount(QueueType queue_type);
  void EnqueueBuffers();
  scoped_refptr<VideoFrame> GetDeviceInputFrame();
  bool NegotiateInputFormat(VideoPixelFormat format,
                            const Size& frame_size);

 private:
  VideoEncoderClient* client_;
  scoped_refptr<VideoEncoder> encoder_;
  int32_t venc_port_index_ = -1;
  std::string resources_ = "";
};

}  // namespace mcil

#endif  // SRC_VIDEO_ENCODER_API_H_
