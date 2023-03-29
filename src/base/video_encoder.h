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

#ifndef SRC_BASE_VIDEO_ENCODER_H_
#define SRC_BASE_VIDEO_ENCODER_H_

#include "encoder_types.h"

namespace mcil {

class VideoEncoderClient;

class VideoEncoder : public RefCounted<VideoEncoder> {
 public:
  static SupportedProfiles GetSupportedProfiles();
  static scoped_refptr<VideoEncoder> Create();

  virtual bool Initialize(const EncoderConfig* configData,
                          VideoEncoderClient* client,
                          EncoderClientConfig* client_config,
                          int venc_port_index) = 0;
  virtual void Destroy() = 0;
  virtual bool IsFlushSupported() = 0;
  virtual bool EncodeFrame(scoped_refptr<VideoFrame> frame,
                           bool force_keyframe) = 0;
  virtual bool FlushFrames() = 0;
  virtual bool UpdateEncodingParams(uint32_t bitrate, uint32_t framerate) = 0;
  virtual bool StartDevicePoll() = 0;
  virtual void RunEncodeBufferTask() = 0;
  virtual void SendStartCommand(bool start) = 0;
  virtual void SetEncoderState(CodecState state) = 0;
  virtual size_t GetFreeBuffersCount(QueueType queue_type) = 0;
  virtual void EnqueueBuffers() = 0;
  virtual scoped_refptr<VideoFrame> GetDeviceInputFrame() = 0;
  virtual bool NegotiateInputFormat(VideoPixelFormat format,
                                    const Size& frame_size) = 0;

  virtual bool EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                            const uint8_t* uBuf, size_t uSize,
                            const uint8_t* vBuf, size_t vSize,
                            uint64_t bufferTimestamp,
                            bool requestKeyFrame) { return true; }

 protected:
  friend class RefCounted<VideoEncoder>;

  VideoEncoder() = default;
  virtual ~VideoEncoder() noexcept(false) {};
};

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_ENCODER_H_
