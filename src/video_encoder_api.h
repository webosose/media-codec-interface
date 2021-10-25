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

namespace encoder {

class VideoEncoder;
class VideoEncoderDelegate;

class VideoEncoderAPI {
 public:
  static mcil::SupportedProfiles GetSupportedProfiles();

  VideoEncoderAPI(VideoEncoderDelegate* delegate);
  ~VideoEncoderAPI();

  bool Initialize(const EncoderConfig* configData);
  bool Destroy();

  bool EncodeBuffers(const uint8_t* yBuf, size_t ySize,
                     const uint8_t* uBuf, size_t uSize,
                     const uint8_t* vBuf, size_t vSize,
                     uint64_t bufferTimestamp,
                     const bool requestKeyFrame);

  bool IsEncoderAvailable();
  bool UpdateEncodingResolution(uint32_t width, uint32_t height);
  bool UpdateEncodingParams(const EncodingParams* properties);

 private:
  VideoEncoderDelegate* delegate_;
  std::shared_ptr<mcil::encoder::VideoEncoder> videoEncoder_;
};

}  // namespace encoder

}  // namespace mcil

#endif  // SRC_VIDEO_ENCODER_API_H_
