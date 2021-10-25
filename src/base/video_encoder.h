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

#include "video_encoder_api.h"
#include "encoder_types.h"
#include "video_encoder_delegate.h"

namespace mcil {

namespace encoder {

class VideoEncoder {
 public:
  static std::shared_ptr<VideoEncoder> Create(VideoCodecType type);

  static mcil::SupportedProfiles GetSupportedProfiles();

  VideoEncoder();
  ~VideoEncoder();

  virtual bool Initialize(const EncoderConfig* configData,
                          VideoEncoderDelegate* delegate);
  virtual bool Destroy();
  virtual bool EncodeBuffers(const uint8_t* yBuf, size_t ySize,
                             const uint8_t* uBuf, size_t uSize,
                             const uint8_t* vBuf, size_t vSize,
                             uint64_t bufferTimestamp,
                             const bool requestKeyFrame);

  virtual bool IsEncoderAvailable();
  virtual bool UpdateEncodingResolution(uint32_t width, uint32_t height);
  virtual bool UpdateEncodingParams(const EncodingParams* properties);

 private:
  friend class GstVideoEncoder;
  friend class LxVideoEncoder;

  EncoderConfig* configData_;
  VideoEncoderDelegate* delegate_ = nullptr;
};

}  // namespace encoder

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_ENCODER_H_
