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

#ifndef SRC_BASE_VIDEO_ENCODER_CLIENT_H_
#define SRC_BASE_VIDEO_ENCODER_CLIENT_H_

#include "encoder_types.h"

namespace mcil {

// Video encoder client interface. This interface is implemented
// by the components that uses the VideoEncoderAPI.
class VideoEncoderClient {
 public:
  virtual void CreateInputBuffers(size_t count) = 0;
  virtual void DestroyInputBuffers() = 0;

  virtual void EnqueueInputBuffer(size_t buffer_index) = 0;
  virtual void DequeueInputBuffer(size_t buffer_index) = 0;

  virtual void BitstreamBufferReady(ReadableBufferRef) = 0;
  virtual void BitstreamBufferReady(const uint8_t* buffer,
                                    size_t buffer_size,
                                    uint64_t timestamp,
                                    bool is_key_frame) = 0;
  virtual void PumpBitstreamBuffers() = 0;

  virtual uint8_t GetH264LevelLimit(const EncoderConfig* config) = 0;
  virtual void StopDevicePoll() = 0;
  virtual void NotifyFlushIfNeeded(bool flush) = 0;
  virtual void NotifyEncodeBufferTask() = 0;
  virtual void NotifyEncoderError(EncoderError error) = 0;
  virtual void NotifyEncoderState(CodecState state) = 0;

 protected:
  virtual ~VideoEncoderClient() = default;
};

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_ENCODER_CLIENT_H_
