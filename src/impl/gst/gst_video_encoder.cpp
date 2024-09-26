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

#include "gst_video_encoder.h"

#include <cstring>
#include <cstdlib>
#include <glib.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <map>
#include <memory>
#include <gio/gio.h>

#include "base/log.h"
#include "base/video_encoder_client.h"

#define GST_V4L2_ENCODER

namespace mcil {

// static
scoped_refptr<VideoEncoder> GstVideoEncoder::Create() {
  return new GstVideoEncoder();
}

// static
SupportedProfiles GstVideoEncoder::GetSupportedProfiles() {
  SupportedProfiles supported_profiles;

  SupportedProfile supported_profile;
  supported_profile.profile = H264PROFILE_BASELINE;
  supported_profile.max_resolution = Size(1920, 1080);
  supported_profile.min_resolution = Size(1, 1);
  supported_profiles.push_back(supported_profile);

  return supported_profiles;
}

GstVideoEncoder::GstVideoEncoder()
  : VideoEncoder(), gst_pipeline_() {
}

GstVideoEncoder::~GstVideoEncoder() {
  Destroy();
}

bool GstVideoEncoder::Initialize(const EncoderConfig* config,
                                  VideoEncoderClient* client,
                                  EncoderClientConfig* client_config,
                                  int32_t venc_port_index) {
  client_ = client;
  if (client_ == nullptr) {
    MCIL_ERROR_PRINT(" Delegate not provided");
    return false;
  }
  MCIL_INFO_PRINT(": width: %d, height: %d, fps: %d",
                  config->width, config->height, config->frameRate);

  // Cast config to the expected type
  const mrp::EncoderConfig* mrp_config =
      reinterpret_cast<const mrp::EncoderConfig*>(config);

  auto buffer_callback = [this](const uint8_t* data, size_t size,
                                uint64_t timestamp, bool is_keyframe) {
    this->OnEncodedBuffer(data, size, timestamp, is_keyframe);
  };

  if (!gst_pipeline_.Initialize(mrp_config, buffer_callback)) {
    MCIL_ERROR_PRINT("Gst Pipeline Initialize Failed");
    return false;
  }

  if (client_config != nullptr) {
    client_config->input_frame_size = Size(config->width, config->height);
    client_config->should_control_buffer_feed = true;
    client_config->should_inject_sps_and_pps = false;
  }

  return true;
}

void GstVideoEncoder::Destroy() {
  gst_pipeline_.Destroy();
}

bool GstVideoEncoder::EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                                   const uint8_t* uBuf, size_t uSize,
                                   const uint8_t* vBuf, size_t vSize,
                                   uint64_t bufferTimestamp,
                                   const bool requestKeyFrame) {
  MCIL_DEBUG_PRINT("Call Encode Buffer");
  return gst_pipeline_.EncodeBuffer(yBuf, ySize, uBuf, uSize, vBuf, vSize,
                                    bufferTimestamp, requestKeyFrame);
}

bool GstVideoEncoder::UpdateEncodingParams(uint32_t bitrate,
                                           uint32_t framerate) {
  MCIL_DEBUG_PRINT(": bitrate=%d, framerate=%d", bitrate, framerate);
  return gst_pipeline_.UpdateEncodingParams(bitrate, framerate);
}

/* called when the appsink notifies us that there is a new buffer ready for
 * processing */
void GstVideoEncoder::OnEncodedBuffer(const uint8_t* data, size_t size,
                                      uint64_t timestamp, bool is_keyframe) {
  MCIL_DEBUG_PRINT("Receive OnEncodeBuffer");
  if (client_ != nullptr) {
    MCIL_DEBUG_PRINT("Call VideoEncoderClient::BitstreamBufferReady");
    client_->BitstreamBufferReady(data, size, timestamp, is_keyframe);
  }
}

}  // namespace mcil
