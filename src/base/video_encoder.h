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
                          int venc_port_index);
  virtual void Destroy();
  virtual bool IsFlushSupported();

  virtual bool EncodeFrame(scoped_refptr<VideoFrame> frame,
                           bool force_keyframe);
  virtual bool FlushFrames();
  virtual bool EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                            const uint8_t* uBuf, size_t uSize,
                            const uint8_t* vBuf, size_t vSize,
                            uint64_t bufferTimestamp,
                            bool requestKeyFrame);
  virtual bool UpdateEncodingParams(uint32_t bitrate, uint32_t framerate);
  virtual bool StartDevicePoll();
  virtual void RunEncodeBufferTask();
  virtual void SendStartCommand(bool start);
  virtual void SetEncoderState(CodecState state);
  virtual size_t GetFreeBuffersCount(QueueType queue_type);
  virtual void EnqueueBuffers();
  virtual scoped_refptr<VideoFrame> GetDeviceInputFrame();
  virtual bool NegotiateInputFormat(VideoPixelFormat format,
                                    const Size& frame_size);
 protected:
  friend class RefCounted<VideoEncoder>;

  VideoEncoder() = default;
  virtual ~VideoEncoder() = default;
};

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_ENCODER_H_
