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

#include "vdec_resource_handler.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <map>

#include "base/log.h"
#include "resourcefacilitator/parser.h"
#include "codec_types.h"

#ifdef MCIL_DEBUG_PRINT
//#undef MCIL_DEBUG_PRINT
#endif
//#define MCIL_DEBUG_PRINT MCIL_INFO_PRINT

namespace mcil {

namespace decoder {

#define LX_MAKE_FOURCC(a,b,c,d) ((unsigned int)((a)|(b)<<8|(c)<<16|(d)<<24))

//Static member definition
VdecResourceHandler& VdecResourceHandler::getInstance() {
    static VdecResourceHandler rm_instance;
    return rm_instance;
}

VdecResourceHandler::VdecResourceHandler()
            :resourceRequestor_(std::make_unique<ResourceRequestor> ("MCIL", "")) {
}

VdecResourceHandler::~VdecResourceHandler() {

}

bool VdecResourceHandler::ReleaseResource(std::string& resources, int vdec_index) {
  vdec_index_list_.erase(vdec_index);
  if (resourceRequestor_)
    return resourceRequestor_->releaseResource(resources);
  return false;
}

bool VdecResourceHandler::SetupResource(const DecoderConfig* decoderConfig, std::string& resources, int *vdec_index) {
  MCIL_INFO_PRINT(": called");

  PortResource_t resourceMMap;
  ACQUIRE_RESOURCE_INFO_T resource_info;
  std::string display_mode_ = std::string("Textured");

  uint32_t display_path_ = MCIL_DEFAULT_DISPLAY;
  video_info_t video_stream_info = {};

  video_stream_info.width = decoderConfig->frameWidth;
  video_stream_info.height = decoderConfig->frameHeight;
  uint32_t input_format_fourcc = VideoCodecProfileToV4L2PixFmt(decoderConfig->profile);
  video_stream_info.encode = input_format_fourcc;
  video_stream_info.decode = input_format_fourcc;
  video_stream_info.frame_rate.num = 30;  //TODO Need to get from chromium
  video_stream_info.frame_rate.den = 1;
  MCIL_DEBUG_PRINT("[video info] width: %d, height: %d, frameRate: %d/%d",
            video_stream_info.width, video_stream_info.height,
            video_stream_info.frame_rate.num, video_stream_info.frame_rate.den);

  program_info_t program;
  program.video_stream = 1;
  source_info_.programs.push_back(program);
  source_info_.video_streams.push_back(video_stream_info);

  resource_info.sourceInfo = &source_info_;
  resource_info.displayMode = const_cast<char*>(display_mode_.c_str());
  resource_info.result = true;

  LoadCommon();

  if (resourceRequestor_) {
    if (!resourceRequestor_->acquireResources(
                resourceMMap, source_info_, display_mode_, resources, display_path_))
    {
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
    MCIL_INFO_PRINT(" failed creating resourceRequestor_");
    return false;
  }

  return true;

}

void VdecResourceHandler::LoadCommon()
{
    if (!resourceRequestor_)
        MCIL_DEBUG_PRINT("NotifyForeground fails");
    else
        resourceRequestor_->notifyForeground();


    if (resourceRequestor_) {
        resourceRequestor_->registerUMSPolicyActionCallback([this]() {
            if (!resourceRequestor_)
                MCIL_DEBUG_PRINT("notifyBackground fails");
            else
                resourceRequestor_->notifyBackground();
            });
    }
}

uint32_t VdecResourceHandler::VideoCodecProfileToV4L2PixFmt(VideoCodecProfile profile) {
  uint32_t fourcc = 0;
  if (profile >= H264PROFILE_MIN && profile < H264PROFILE_MAX) {
    fourcc = LX_MAKE_FOURCC('a','v','c','1');
  } else if (profile >= VP8PROFILE_MIN && profile <= VP8PROFILE_MAX) { // VP8
    fourcc = LX_MAKE_FOURCC('V','P','8','0');
  } else if (profile >= VP9PROFILE_MIN && profile <= VP9PROFILE_MAX) { // VP9
    fourcc = LX_MAKE_FOURCC('v','p','9','0');
  } else if (profile >= AV1PROFILE_MIN && profile < AV1PROFILE_MAX) { // AV1
    fourcc = LX_MAKE_FOURCC('A','V','1','0');
  }
  
  return fourcc;
}

}  // namespace decoder

}  // namespace mcil
