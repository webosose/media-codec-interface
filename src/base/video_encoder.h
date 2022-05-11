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
  static scoped_refptr<VideoEncoder> Create(VideoCodecType type);

  virtual bool Initialize(const EncoderConfig* configData,
                          VideoEncoderClient* client);
  virtual bool Destroy();
  virtual bool EncodeBuffers(const uint8_t* yBuf, size_t ySize,
                             const uint8_t* uBuf, size_t uSize,
                             const uint8_t* vBuf, size_t vSize,
                             uint64_t bufferTimestamp,
                             const bool requestKeyFrame);

  virtual bool IsEncoderAvailable();
  virtual bool UpdateEncodingResolution(uint32_t width, uint32_t height);
  virtual bool UpdateEncodingParams(const EncodingParams* properties);
  virtual void ServiceDeviceTask();
  virtual size_t GetFreeBuffersCount(QueueType queue_type);
  virtual void SetEncoderState(CodecState state);
  virtual bool Flush();

protected:
  friend class RefCounted<VideoEncoder>;

  VideoEncoder();
  virtual ~VideoEncoder();
};

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_ENCODER_H_
