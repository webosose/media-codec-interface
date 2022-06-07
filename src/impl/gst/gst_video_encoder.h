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

#ifndef SRC_IMPL_GST_GST_VIDEO_ENCODER_H_
#define SRC_IMPL_GST_GST_VIDEO_ENCODER_H_

#include "base/video_encoder.h"

#include <gst/gst.h>

using namespace std;

namespace mcil {

class GstVideoEncoder : public VideoEncoder {
 public:
  GstVideoEncoder();
  ~GstVideoEncoder() override;

  bool Initialize(const EncoderConfig* config,
                  VideoEncoderClient* client,
                  EncoderClientConfig* client_config,
                  int venc_port_index) override;
  void Destroy() override;
  bool EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                    const uint8_t* uBuf, size_t uSize,
                    const uint8_t* vBuf, size_t vSize,
                    uint64_t bufferTimestamp,
                    const bool requestKeyFrame) override;
  bool UpdateEncodingParams(uint32_t bitrate, uint32_t framerate) override;

  static gboolean HandleBusMessage(
      GstBus *bus_, GstMessage *message, gpointer user_data);

 private:
  bool CreatePipeline(const EncoderConfig* configData);
  bool CreateEncoder(VideoCodecProfile profile);
  bool CreateSink();
  bool LinkElements(const EncoderConfig* configData);

  static GstFlowReturn OnEncodedBuffer(GstElement* elt, gpointer* data);

  VideoEncoderClient* client_;

  GstBus *bus_ = nullptr;
  bool load_complete_ = false;
  GstElement *pipeline_ = nullptr;
  GstElement *source_ = nullptr;
  GstElement *filter_YUY2_ = nullptr;
  GstElement *parse_ = nullptr;
  GstElement *converter_ = nullptr;
  GstElement *filter_NV12_ = nullptr;
  GstElement *encoder_ = nullptr;
  GstElement *sink_ = nullptr;
  GstCaps *caps_YUY2_ = nullptr;
  GstCaps *caps_NV12_ = nullptr;

  int32_t bitrate_ = 0;

  ChronoTime start_time_;
  uint32_t current_seconds_ = 0;
  uint32_t buffers_per_sec_ = 0;
};

}  // namespace mcil

#endif  // SRC_IMPL_GST_GST_VIDEO_ENCODER_H_
