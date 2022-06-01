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

#ifndef SRC_BASE_VIDEO_DECODER_CLIENT_H_
#define SRC_BASE_VIDEO_DECODER_CLIENT_H_

#include "decoder_types.h"
#include "video_buffers.h"

namespace mcil {

// Video decoder client interface. This interface is implemented
// by the components that uses the VideoDecoderAPI.
class VideoDecoderClient {
 public:
  virtual bool CreateOutputBuffers(VideoPixelFormat pixel_format,
                                   uint32_t buffer_count,
                                   uint32_t texture_target) = 0;
  virtual bool DestroyOutputBuffers() = 0;
  virtual void ScheduleDecodeBufferTaskIfNeeded() = 0;
  virtual void StartResolutionChange() = 0;
  virtual void NotifyFlushDone() = 0;
  virtual void NotifyFlushDoneIfNeeded() = 0;
  virtual void NotifyResetDone() = 0;
  virtual bool IsDestroyPending() = 0;
  virtual void OnStartDevicePoll() = 0;
  virtual void OnStopDevicePoll() = 0;
  virtual void CreateBuffersForFormat(const Size& coded_size,
                                      const Size& visible_size) = 0;
  virtual void SendBufferToClient(
      size_t buffer_index, int32_t buffer_id, ReadableBufferRef buffer) = 0;
  virtual void CheckGLFences() = 0;

  virtual void NotifyDecoderError(DecoderError error) = 0;
  virtual void NotifyDecodeBufferTask(bool event_pending, bool has_output) = 0;
  virtual void NotifyDecoderPostTask(PostTaskType task, bool value) = 0;
  virtual void NotifyDecodeBufferDone() = 0;

 protected:
  virtual ~VideoDecoderClient() = default;
};

}  // namespace mcil

#endif  // SRC_BASE_VIDEO_DECODER_CLIENT_H_
