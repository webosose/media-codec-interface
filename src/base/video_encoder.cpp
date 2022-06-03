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

#include "video_encoder.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <map>
#include <memory>
#include <gio/gio.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>

#include "base/log.h"

namespace mcil {

VideoEncoder::VideoEncoder() = default;

VideoEncoder::~VideoEncoder() = default;

bool VideoEncoder::Initialize(const EncoderConfig* configData,
                              VideoEncoderClient* client,
                              EncoderClientConfig* client_config,
                              int venc_port_index) {
  return true;
}

void VideoEncoder::Destroy() {
}

bool VideoEncoder::IsFlushSupported() {
  return true;
}

bool VideoEncoder::EncodeFrame(scoped_refptr<VideoFrame> frame,
                               bool force_keyframe) {
  return false;
}

bool VideoEncoder::EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                                const uint8_t* uBuf, size_t uSize,
                                const uint8_t* vBuf, size_t vSize,
                                uint64_t bufferTimestamp,
                                bool requestKeyFrame) {
  return false;
}

bool VideoEncoder::UpdateEncodingParams(uint32_t bitrate, uint32_t framerate) {
  return false;
}

bool VideoEncoder::StartDevicePoll() {
  return true;
}

void VideoEncoder::RunEncodeBufferTask() {
}

void VideoEncoder::SendStartCommand(bool start) {
}

void VideoEncoder::SetEncoderState(CodecState state) {
}

size_t VideoEncoder::GetFreeBuffersCount(QueueType queue_type) {
  return 0;
}

void VideoEncoder::EnqueueBuffers() {
}

scoped_refptr<VideoFrame> VideoEncoder::GetDeviceInputFrame() {
  return nullptr;
}

bool VideoEncoder::NegotiateInputFormat(VideoPixelFormat format,
                                        const Size& frame_size) {
  return false;
}

}  // namespace mcil
