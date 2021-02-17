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

#include <functional>

#include "base/encoder_types.h"

using namespace std;
using CALLBACK_T = std::function<void(const gint type, const gint64 numValue,
        const gchar *strValue, void *udata)>;

namespace mcil {

namespace encoder {

class VideoEncoder {
 public:
  static std::shared_ptr<VideoEncoder> Create(MCIL_VIDEO_CODEC type);
  static bool IsCodecSupported(MCIL_VIDEO_CODEC videoCodec);

  VideoEncoder();
  ~VideoEncoder();

  virtual bool Init(const ENCODER_INIT_DATA_T* loadData,
                      NEWFRAME_CALLBACK_T new_frame_cb);
  virtual bool Deinit();
  virtual MCIL_MEDIA_STATUS_T Feed(const uint8_t* bufferPtr, size_t bufferSize);
  virtual MCIL_MEDIA_STATUS_T Feed(const uint8_t* yBuf, size_t ySize,
                                   const uint8_t* uBuf, size_t uSize,
                                   const uint8_t* vBuf, size_t vSize,
                                   uint64_t bufferTimestamp,
                                   const bool requestKeyFrame);

  virtual bool IsEncoderAvailable();
  virtual bool UpdateEncodingResolution(uint32_t width, uint32_t height);

  void RegisterCbFunction(CALLBACK_T);

 private:
  friend class GstVideoEncoder;
  friend class LxVideoEncoder;

  CALLBACK_T cbFunction_ = nullptr;

  ENCODER_INIT_DATA_T* initData_;

  NEWFRAME_CALLBACK_T new_frame_cb_ = nullptr;
};

}  // namespace encoder

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_ENCODER_H_
