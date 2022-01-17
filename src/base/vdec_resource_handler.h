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

#ifndef SRC_BASE_VDEC_RESOURCE_HANDLER_H_
#define SRC_BASE_VDEC_RESOURCE_HANDLER_H_

#include "resourcefacilitator/requestor.h"
#include "base/decoder_types.h"
#include <set>

class UMSConnectorHandle;
class UMSConnectorMessage;

namespace mcil {

namespace decoder {

class ResourceRequestor;
struct source_info_t;

typedef struct ACQUIRE_RESOURCE_INFO {
  source_info_t* sourceInfo;
  char *displayMode;
  bool result;
} ACQUIRE_RESOURCE_INFO_T;

class VdecResourceHandler {
public:
  ~VdecResourceHandler();

  static VdecResourceHandler& getInstance();

  bool SetupResource(const DecoderConfig* decoderConfig, std::string& resources, int *vdec_index);
  bool ReleaseResource(std::string& resources, int vdec_index);

  void LoadCommon();
  uint32_t VideoCodecProfileToV4L2PixFmt(VideoCodecProfile profile);

private:
  VdecResourceHandler();

  std::unique_ptr<ResourceRequestor> resourceRequestor_;
  std::string media_id_;  // connection_id
  std::string app_id_;
  source_info_t source_info_;
  std::set<int> vdec_index_list_;
};

}  // namespace decoder

}  // namespace mcil

#endif  // SRC_BASE_VDEC_RESOURCE_HANDLER_H_
