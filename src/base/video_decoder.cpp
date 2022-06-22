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

namespace mcil {


bool VideoDecoder::Initialize(const DecoderConfig* decoderConfig,
                              VideoDecoderClient* client,
                              DecoderClientConfig* client_config,
                              int32_t vdec_port_index) {
  return true;
}

void VideoDecoder::Destroy() {
}

bool VideoDecoder::ResetInputBuffer() {
  return false;
}

bool VideoDecoder::ResetDecodingBuffers(bool* reset_pending) {
  return false;
}

bool VideoDecoder::CanNotifyResetDone() {
  return true;
}

bool VideoDecoder::DecodeBuffer(const void* buffer,
                                size_t size,
                                const int32_t id,
                                int64_t buffer_pts) {
  return false;
}

bool VideoDecoder::FlushInputBuffers() {
  return false;
}

bool VideoDecoder::DidFlushBuffersDone() {
  return false;
}

void VideoDecoder::EnqueueBuffers() {
}

void VideoDecoder::DequeueBuffers() {
}

void VideoDecoder::ReusePictureBuffer(int32_t pic_buffer_id) {
  return;
}

void VideoDecoder::RunDecodeBufferTask(bool event_pending, bool has_output) {
}

void VideoDecoder::RunDecoderPostTask(PostTaskType task, bool value) {
}

void VideoDecoder::SetDecoderState(CodecState state) {
}

bool VideoDecoder::GetCurrentInputBufferId(int32_t* buffer_id) {
  return false;
}

size_t VideoDecoder::GetFreeBuffersCount(QueueType queue_type) {
  return 0;
}

bool VideoDecoder::AllocateOutputBuffers(
    uint32_t count, std::vector<WritableBufferRef*>& buffers) {
  return false;
}

bool VideoDecoder::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  return true;
}

void VideoDecoder::OnEGLImagesCreationCompleted() {
}

}  // namespace mcil
