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

#include <set>

#include "base/decoder_types.h"
#include "resource/requestor.h"

class UMSConnectorHandle;
class UMSConnectorMessage;

namespace mcil {

namespace decoder {

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

  bool Acquire(const DecoderConfig* decoder_config,
               std::string& resources,
               int32_t *vdec_index);
  bool Release(std::string& resources, int32_t vdec_index);

 private:
  VideoResource();

  std::unique_ptr<ResourceRequestor> requestor_;
  source_info_t source_info_;
  std::set<int32_t> vdec_index_list_;
};

}  // namespace decoder

}  // namespace mcil

#endif  // SRC_RESOURCE_VIDEO_RESOURCE_H_
