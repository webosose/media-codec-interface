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

#ifndef SRC_BASE_VIDEO_ENCODER_DELEGATE_H_
#define SRC_BASE_VIDEO_ENCODER_DELEGATE_H_

#include "decoder_types.h"

namespace mcil {

namespace encoder {

// Video encoder delegate interface. This interface is implemented
// by the components that uses the VideoEncoderAPI.
class VideoEncoderDelegate {
 public:
  virtual void EncodedBufferReady(const uint8_t* buffer,
                                  size_t buffer_size,
                                  uint64_t timestamp,
                                  bool is_key_frame) = 0;
 protected:
  virtual ~VideoEncoderDelegate() = default;
};

}  // namespace encoder

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_ENCODER_DELEGATE_H_
