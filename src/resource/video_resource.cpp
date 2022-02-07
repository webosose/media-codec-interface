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

#include "video_resource.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <map>

#include "base/log.h"
#include "decoder_types.h"

#define MCIL_MAX_FRAME_RATE 60 //Set to maximum framerate supported in webcodec

#ifdef MCIL_DEBUG_PRINT
//#undef MCIL_DEBUG_PRINT
#endif
//#define MCIL_DEBUG_PRINT MCIL_INFO_PRINT

namespace mcil {

namespace decoder {

//Static member definition
VideoResource& VideoResource::GetInstance() {
  static VideoResource rm_instance;
  return rm_instance;
}

VideoResource::VideoResource()
  : requestor_(std::make_unique<ResourceRequestor> ()) {
}

VideoResource::~VideoResource() {
}

bool VideoResource::Acquire(const DecoderConfig* decoder_config,
                           std::string& resources,
                           int32_t *vdec_index) {
  MCIL_INFO_PRINT(": called");

  PortResource_t resourceMMap;
  ACQUIRE_RESOURCE_INFO_T resource_info;

  video_info_t video_stream_info = {};

  video_stream_info.width = decoder_config->frameWidth;
  video_stream_info.height = decoder_config->frameHeight;

  if (*vdec_index == V4L2_DECODER) {
    MCIL_DEBUG_PRINT(": decoder resource needed");
    video_stream_info.encode = VIDEO_CODEC_NONE;
    video_stream_info.decode = decoder_config->codecType;
  } else if (*vdec_index == V4L2_ENCODER) {
    MCIL_DEBUG_PRINT(": encoder resource needed");
    video_stream_info.encode = decoder_config->codecType;
    video_stream_info.decode = VIDEO_CODEC_NONE;
  }

  video_stream_info.frame_rate.num = MCIL_MAX_FRAME_RATE;
  video_stream_info.frame_rate.den = 1;
  MCIL_DEBUG_PRINT("[video info] width: %d, height: %d, frameRate: %d/%d",
            video_stream_info.width, video_stream_info.height,
            video_stream_info.frame_rate.num, video_stream_info.frame_rate.den);

  program_info_t program;
  program.video_stream = 1;
  source_info_.programs.push_back(program);
  source_info_.video_streams.push_back(video_stream_info);

  resource_info.sourceInfo = &source_info_;
  resource_info.result = true;

  if (requestor_) {
    requestor_->notifyForeground();
    requestor_->registerUMSPolicyActionCallback([this]() {
      requestor_->notifyBackground();
    });


    if (!requestor_->acquireResources(resourceMMap,
            source_info_, resources)) {
      MCIL_INFO_PRINT("resource acquisition failed");
      return false;
    }

    for (auto it : resourceMMap) {
      MCIL_DEBUG_PRINT("Resource::[%s]=>index:%d", it.first.c_str(), it.second);
      if (it.first == "VDEC") {
        *vdec_index = it.second;
      }
    }

    if (vdec_index_list_.find(*vdec_index) == vdec_index_list_.end()) {
      MCIL_DEBUG_PRINT(" resource acquired:%d", *vdec_index);
      vdec_index_list_.insert(*vdec_index);
    } else {
      MCIL_INFO_PRINT(" failed to get new resource:%d", *vdec_index);
      *vdec_index = -1;
      return false;
    }
  } else {
    MCIL_INFO_PRINT(" failed creating requestor_");
    return false;
  }

  return true;

}

bool VideoResource::Release(std::string& resources, int32_t vdec_index) {
  vdec_index_list_.erase(vdec_index);

  if (requestor_)
    return requestor_->releaseResource(resources);
  return false;
}

}  // namespace decoder

}  // namespace mcil
