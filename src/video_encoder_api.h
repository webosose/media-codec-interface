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


#ifndef SRC_MEDIA_CODEC_INTERFACE_VIDEO_ENCODER_API_H_
#define SRC_MEDIA_CODEC_INTERFACE_VIDEO_ENCODER_API_H_

#include <glib.h>
#include <string>
#include <memory>
#include <functional>

#include "encoder_types.h"

namespace mcil {

namespace encoder {

typedef std::function<bool(uint8_t*, ENCODED_BUFFER_T*)> FunctorEncoder;

class VideoEncoder;

class VideoEncoderAPI {
  public:
    VideoEncoderAPI();
    ~VideoEncoderAPI();

    static bool IsCodecSupported(MCP_VIDEO_CODEC videoCodec);

    bool Init(const ENCODER_INIT_DATA_T* loadData,
              NEWFRAME_CALLBACK_T new_frame_cb);
    bool Deinit();

    MCP_MEDIA_STATUS_T Encode(const uint8_t* bufferPtr, size_t bufferSize);
    MCP_MEDIA_STATUS_T Encode(const uint8_t* yBuf, size_t ySize,
                              const uint8_t* uBuf, size_t uSize,
                              const uint8_t* vBuf, size_t vSize,
                              uint64_t bufferTimestamp,
                              const bool requestKeyFrame);

    void RegisterCallback(ENCODER_CALLBACK_T callback, void *uData);
    bool UpdateEncodingResolution(uint32_t width, uint32_t height);
    bool UpdateEncodingParams(const ENCODING_PARAMS_T* properties);

  private:
    bool OnEncodedDataAvailable(uint8_t* buffer, ENCODED_BUFFER_T* encData);
    void Notify(const gint notification, const gint64 numValue,
                const gchar *strValue, void *payload);

    std::string appId_;
    std::string media_id_;
    std::string instanceId_;
    std::string connectionId_;
    ENCODER_CALLBACK_T callback_;

    std::shared_ptr<mcil::encoder::VideoEncoder> videoEncoder_;
    void *userData_ = nullptr;
};

}  // namespace encoder

}  // namespace mcil

#endif  // SRC_MEDIA_CODEC_INTERFACE_VIDEO_ENCODER_API_H_
