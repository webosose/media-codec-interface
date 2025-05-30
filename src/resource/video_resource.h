// Copyright (c) 2022 LG Electronics, Inc.
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

#ifndef SRC_RESOURCE_VIDEO_RESOURCE_H_
#define SRC_RESOURCE_VIDEO_RESOURCE_H_

#include <mutex>
#include <set>
#include <thread>

#include "base/decoder_types.h"
#include "resource/requestor.h"

namespace mcil {

class ResourceRequestor;
struct source_info_t;

typedef struct ACQUIRE_RESOURCE_INFO {
  source_info_t* sourceInfo;
  bool result;
} ACQUIRE_RESOURCE_INFO_T;

class VideoResource {
 public:
  static VideoResource& GetInstance();
  VideoResource(VideoResource const&) = delete;
  void operator=(VideoResource const&)  = delete;

  ~VideoResource();

  bool Acquire(DeviceType device_type,
               VideoCodec video_codec,
               uint32_t frame_width,
               uint32_t frame_height,
               uint32_t frame_rate,
               std::string& resources,
               int32_t *resource_index);
#if defined (ENABLE_REACQUIRE)
  bool Reacquire(DeviceType device_type,
                 VideoCodec video_codec,
                 uint32_t frame_width,
                 uint32_t frame_height,
                 uint32_t frame_rate,
                 std::string& resources,
                 int32_t *resource_index);
#endif
  bool ReleaseVideoResource(DeviceType device_type,
               std::string& resources,
               int32_t resource_index);

 private:
  VideoResource();

  bool GetSourceInfo(DeviceType device_type,
                     VideoCodec video_codec,
                     uint32_t frame_width,
                     uint32_t frame_height,
                     uint32_t frame_rate,
                     source_info_t* source_info);

  bool AddToIndexList(PortResource_t resourceMMap, int32_t *resource_index);
  void RemoveFromIndexList(DeviceType device_type, int32_t resource_index);

  std::unique_ptr<ResourceRequestor> requestor_;
  std::set<int32_t> vdec_index_list_;
  std::set<int32_t> venc_index_list_;
  mutable std::mutex mutex_;
};

}  // namespace mcil

#endif  // SRC_RESOURCE_VIDEO_RESOURCE_H_
