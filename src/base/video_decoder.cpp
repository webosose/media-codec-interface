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

namespace decoder {

VideoDecoder::VideoDecoder() {
}

VideoDecoder::~VideoDecoder() {
}

bool VideoDecoder::Initialize(const DecoderConfig* decoderConfig,
                              VideoDecoderDelegate* delegate,
                              VideoPixelFormat* output_pix_fmt,
                              bool* should_control_buffer_feed) {
  delegate_ = delegate;
  return true;
}

bool VideoDecoder::DoReset(bool full_reset, bool* reset_pending) {
  return false;
}

void VideoDecoder::Destroy() {
}

bool VideoDecoder::FeedBuffers(
    const void* buffer, size_t size, const int32_t id, uint64_t buffer_pts) {
  return false;
}

bool VideoDecoder::FlushBuffers() {
  return false;
}

bool VideoDecoder::DidFlushBuffersDone() {
  return false;
}

bool VideoDecoder::EnqueueBuffers() {
  return false;
}

bool VideoDecoder::DequeueBuffers() {
  return false;
}

void VideoDecoder::ReusePictureBuffer(int32_t pic_buffer_id) {
  return;
}

bool VideoDecoder::StartDevicePoll(bool poll_device, bool* event_pending) {
  return false;
}

void VideoDecoder::RunDecodeBufferTask(bool event_pending, bool has_output) {
}

void VideoDecoder::RunDecoderPostTask(PostTaskType task, bool value) {
}

void VideoDecoder::SetDecoderState(DecoderState state) {
}

int64_t VideoDecoder::GetCurrentInputBufferId() {
  return kFlushBufferId;
}

size_t VideoDecoder::GetFreeBuffersCount(QueueType queue_type) {
  return 0;
}

std::vector<WritableBufferRef*>
VideoDecoder::AllocateOutputBuffers(std::vector<OutputRecord>* records) {
  return empty_output_buffer;
}

bool VideoDecoder::CanCreateEGLImageFrom(VideoPixelFormat pixel_format) {
  return true;
}

void VideoDecoder::OnEGLImagesCreationCompleted() {
}

}  // namespace decoder

}  // namespace mcil
