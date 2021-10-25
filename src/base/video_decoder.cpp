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

#include "video_decoder.h"

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

namespace decoder {

VideoDecoder::VideoDecoder() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

VideoDecoder::~VideoDecoder() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

bool VideoDecoder::Initialize(const DecoderConfig* decoderConfig,
                              VideoDecoderDelegate* delegate,
                              VideoPixelFormat* output_pix_fmt,
                              bool* consider_egl_image_creation) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  return true;
}

bool VideoDecoder::DoReset(bool full_reset, bool* reset_pending) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  return true;
}

void VideoDecoder::Destroy() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
}

bool VideoDecoder::FeedBuffers(
    const void* buffer, size_t size, const int32_t id, int64_t buffer_pts) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

bool VideoDecoder::FlushBuffers() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

bool VideoDecoder::DidFlushBuffersDone() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

bool VideoDecoder::EnqueueBuffers() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

bool VideoDecoder::DequeueBuffers() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

bool VideoDecoder::StartDevicePoll(bool poll_device, bool* event_pending) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

void VideoDecoder::RunDecodeBufferTask(bool event_pending) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
}

void VideoDecoder::SetDecoderState(DecoderState state) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
}

int64_t VideoDecoder::GetCurrentInputBufferId() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return kFlushBufferId;
}

size_t VideoDecoder::GetFreeBuffersCount(QueueType queue_type) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return 0;
}

std::vector<WritableBufferRef*>
VideoDecoder::AllocateOutputBuffers(std::vector<OutputRecord>* records) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return empty_output_buffer;
}

bool VideoDecoder::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

void VideoDecoder::OnEGLImagesCreationCompleted() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
}

}  // namespace decoder

}  // namespace mcil
