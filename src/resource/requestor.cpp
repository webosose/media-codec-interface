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

#include "requestor.h"

#include <boost/regex.hpp>

#include <string>
#include <set>
#include <utility>
#include <cmath>
#include <ResourceManagerClient.h>

#include "base/log.h"

using namespace std;
using mrc::ResourceCalculator;
using namespace pbnjson;

namespace mcil {

ResourceRequestor::ResourceRequestor(const std::string& connectionId)
  : rc_(std::shared_ptr<MRC>(MRC::create())),
    cb_(nullptr),
    videoResData_{VIDEO_CODEC_NONE, VIDEO_CODEC_NONE, VIDEO_CODEC_NONE,
                  0, 0, 0, 0, 0, 0, 0},
    allowPolicy_(true) {
  try {
    if (connectionId.empty()) {
      umsRMC_ = make_shared<uMediaServer::ResourceManagerClient> ();
      umsRMC_->registerPipeline("media"); // only rmc case
      connectionId_ = (umsRMC_->getConnectionID() ?
          std::string(umsRMC_->getConnectionID()) : std::string()); // after registerPipeline
      if (connectionId_.empty()) {
        MCIL_ERROR_PRINT("Failed to get connection ID");
        exit(0);
      }
    } else {
      umsRMC_ = make_shared<uMediaServer::ResourceManagerClient> (connectionId);
      connectionId_ = connectionId;
    }
  } catch (const std::exception &e) {
    MCIL_ERROR_PRINT("Failed to create ResourceRequestor [%s]", e.what());
    exit(0);
  }

  if (connectionId_.empty()) {
    MCIL_ERROR_PRINT("Connection id is empty");
    assert(0);
  }

  umsRMC_->registerPolicyActionHandler(
      std::bind(&ResourceRequestor::PolicyActionHandler, this,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3, std::placeholders::_4,
                std::placeholders::_5));
  MCIL_DEBUG_PRINT("ResourceRequestor creation done");
}

ResourceRequestor::~ResourceRequestor() { }

bool ResourceRequestor::AcquireResources(PortResource_t& resourceMMap,
                                         const source_info_t &sourceInfo,
                                         std::string& resources) {
  std::string payload = GetSourceString(sourceInfo);
  if (payload.empty()) {
    MCIL_ERROR_PRINT("Fail to getSourceString");
    return false;
  }

  MCIL_DEBUG_PRINT("payload string = %s", payload.c_str());

  string response;
  if (!umsRMC_->acquire(payload, response)) {
    MCIL_ERROR_PRINT("fail to acquire!!! response : %s", response.c_str());
    return false;
  }

  try {
    ParsePortInformation(response, resourceMMap);
    ParseResources(response, resources);
  } catch (const std::runtime_error & err) {
    MCIL_ERROR_PRINT(" err=%s, response:%s", err.what(), response.c_str());
    return false;
  }

  return true;
}

#if defined (ENABLE_REACQUIRE)
bool ResourceRequestor::ReacquireResources(PortResource_t& resourceMMap,
                                           const source_info_t &sourceInfo,
                                           std::string& resources) {
  std::string request = GetSourceString(sourceInfo);
  if (request.empty()) {
    MCIL_ERROR_PRINT("Fail to getSourceString");
    return false;
  }

  JValue reacquire_obj = pbnjson::Object();
  reacquire_obj.put("new", request);
  reacquire_obj.put("old", resources);

  JSchemaFragment input_schema("{}");
  JGenerator serializer(nullptr);

  std::string payload;
  if (!serializer.toString(reacquire_obj, input_schema, payload)) {
    MCIL_ERROR_PRINT("[%s], fail to serializer to string", __func__);
    return false;
  }

  MCIL_DEBUG_PRINT("payload string = %s", payload.c_str());

  std::string response;
  // TODO: Enable for platform extensions selectively when available
#if !defined(PLATFORM_EXTENSION)
  if (!umsRMC_->reacquire(payload, response)) {
    MCIL_ERROR_PRINT("fail to acquire!!! response : %s", response.c_str());
    return false;
  }
#endif

  try {
    ParsePortInformation(response, resourceMMap);
    ParseResources(response, resources);
  } catch (const std::runtime_error & err) {
    MCIL_ERROR_PRINT(" err=%s, response:%s", err.what(), response.c_str());
    return false;
  }

  return true;
}
#endif

mrc::ResourceListOptions ResourceRequestor::CalcVdecResources() {
  mrc::ResourceListOptions vdec_resource;
  MCIL_DEBUG_PRINT("Codec type:%d",videoResData_.vdecode);
  if (videoResData_.vdecode != VIDEO_CODEC_NONE) {
    vdec_resource = rc_->calcVdecResourceOptions(
        (MRC::VideoCodecs)TranslateVideoCodec(videoResData_.vdecode),
        videoResData_.width,
        videoResData_.height,
        videoResData_.frameRate,
        (MRC::ScanType)TranslateScanType(videoResData_.escanType),
        (MRC::_3DType)Translate3DType(videoResData_.e3DType));
  }

  return vdec_resource;
}

mrc::ResourceListOptions ResourceRequestor::CalcVencResources() {
  mrc::ResourceListOptions venc_resource;

  MCIL_DEBUG_PRINT("Codec type:%d",videoResData_.vencode);
  if (videoResData_.vencode != VIDEO_CODEC_NONE) {
    venc_resource = rc_->calcVencResourceOptions(
        (MRC::VideoCodecs)TranslateVideoCodec(videoResData_.vencode),
        videoResData_.width,
        videoResData_.height,
        videoResData_.frameRate);
  }

  return venc_resource;
}

bool ResourceRequestor::ReleaseResource(std::string& resources) {
  if (resources.empty()) {
    MCIL_DEBUG_PRINT("[%s], resource already empty", __func__);
    return true;
  }

  if (!umsRMC_->release(resources)) {
    MCIL_ERROR_PRINT("release error : %s", resources.c_str());
    return false;
  }

  resources = "";
  return true;
}

bool ResourceRequestor::NotifyForeground() const {
  return umsRMC_->notifyForeground();
}

bool ResourceRequestor::NotifyBackground() const {
  return umsRMC_->notifyBackground();
}

void ResourceRequestor::AllowPolicyAction(const bool allow) {
  allowPolicy_ = allow;
}

bool ResourceRequestor::PolicyActionHandler(const char *action,
                                            const char *resources,
                                            const char *requestorType,
                                            const char *requestorName,
                                            const char *connectionId) {
  MCIL_DEBUG_PRINT("action:%s, resources:%s, type:%s, name:%s, id:%s",
      action, resources, requestorType, requestorName, connectionId);

  if (allowPolicy_) {
    if (nullptr != cb_) {
      cb_();
    }
    if (!umsRMC_->release(resources)) {
      MCIL_ERROR_PRINT("release error : %s", resources);
      return false;
    }
  }

  return allowPolicy_;
}

std::string ResourceRequestor::GetSourceString(
    const source_info_t &sourceInfo) {
  std::string payload;
  mrc::ResourceListOptions finalOptions;
  if (!SetSourceInfo(sourceInfo)) {
    MCIL_ERROR_PRINT("Failed to set source info!");
    return payload;
  }

  mrc::ResourceListOptions vdec_resource = CalcVdecResources();
  if (!vdec_resource.empty()) {
    mrc::concatResourceListOptions(&finalOptions, &vdec_resource);
    MCIL_DEBUG_PRINT("vdec_resource size:%lu, %s, %d",
        vdec_resource.size(), vdec_resource[0].front().type.c_str(),
        vdec_resource[0].front().quantity);
  }

  mrc::ResourceListOptions venc_resource = CalcVencResources();
  if (!venc_resource.empty()) {
    mrc::concatResourceListOptions(&finalOptions, &venc_resource);
    MCIL_DEBUG_PRINT("venc_resource size:%lu, %s, %d",
        venc_resource.size(), venc_resource[0].front().type.c_str(),
        venc_resource[0].front().quantity);
  }

  JSchemaFragment input_schema("{}");
  JGenerator serializer(nullptr);

  JValue objArray = pbnjson::Array();
  for (auto& option : finalOptions) {
    for (auto const & it : option) {
      JValue obj = pbnjson::Object();
      obj.put("resource", it.type);
      obj.put("qty", it.quantity);
      MCIL_DEBUG_PRINT("calculator return : %s, %d",
                       it.type.c_str(), it.quantity);
      objArray << obj;
    }
  }

  if (!serializer.toString(objArray, input_schema, payload)) {
    MCIL_ERROR_PRINT("[%s], fail to serializer to string", __func__);
    return payload;
  }

  return payload;
}

bool ResourceRequestor::ParsePortInformation(const std::string& payload,
                                             PortResource_t& resourceMMap) {
  pbnjson::JDomParser parser;
  pbnjson::JSchemaFragment input_schema("{}");
  if (!parser.parse(payload, input_schema)) {
    throw std::runtime_error(" : payload parsing failure");
  }

  pbnjson::JValue parsed = parser.getDom();
  if (!parsed.hasKey("resources")) {
    throw std::runtime_error("payload must have \"resources key\"");
  }

  int32_t res_arraysize = parsed["resources"].arraySize();

  for (int32_t i=0; i < res_arraysize; ++i) {
    std::string resourceName = parsed["resources"][i]["resource"].asString();
    int32_t value = parsed["resources"][i]["index"].asNumber<int32_t>();
    resourceMMap.insert(std::make_pair(resourceName, value));
  }

  for (auto& it : resourceMMap) {
    MCIL_DEBUG_PRINT("port Resource- %s, : [%d] ", it.first.c_str(), it.second);
  }

  return true;
}

bool ResourceRequestor::ParseResources(const std::string& payload,
                                       std::string& resources) {
  resources = std::string();

  pbnjson::JDomParser parser;
  pbnjson::JSchemaFragment input_schema("{}");
  pbnjson::JGenerator serializer(nullptr);

  if (!parser.parse(payload, input_schema)) {
    throw std::runtime_error(" : payload parsing failure");
  }

  pbnjson::JValue parsed = parser.getDom();
  if (!parsed.hasKey("resources")) {
    throw std::runtime_error("payload must have \"resources key\"");
  }

  pbnjson::JValue objArray = pbnjson::Array();
  for (int32_t i=0; i < parsed["resources"].arraySize(); ++i) {
    pbnjson::JValue obj = pbnjson::Object();
    obj.put("resource", parsed["resources"][i]["resource"].asString());
    obj.put("index", parsed["resources"][i]["index"].asNumber<int32_t>());
    objArray << obj;
  }

  if (!serializer.toString(objArray, input_schema, resources)) {
    throw std::runtime_error("failed to serializer toString");
  }

  return true;
}

int32_t ResourceRequestor::TranslateVideoCodec(const VideoCodec vcodec) const {
  MRC::VideoCodec ev = MRC::kVideoEtc;
  switch (vcodec) {
    case VIDEO_CODEC_NONE:
      ev = MRC::kVideoEtc;    break;
    case VIDEO_CODEC_H264:
      ev = MRC::kVideoH264;   break;
    case VIDEO_CODEC_HEVC:
      ev = MRC::kVideoH265;   break;
    case VIDEO_CODEC_MPEG2:
      ev = MRC::kVideoMPEG;   break;
    case VIDEO_CODEC_MPEG4:
      ev = MRC::kVideoMPEG4;  break;
    case VIDEO_CODEC_VP8:
      ev = MRC::kVideoVP8;    break;
    case VIDEO_CODEC_VP9:
      ev = MRC::kVideoVP9;    break;
    default:
      ev = MRC::kVideoH264;   break;
  }

  return static_cast<int32_t>(ev);
}

int32_t ResourceRequestor::TranslateScanType(const int32_t escanType) const {
  MRC::ScanType scan = MRC::kScanProgressive;

  switch (escanType) {
    default:
      break;
  }

  return static_cast<int32_t>(scan);
}

int32_t ResourceRequestor::Translate3DType(const int32_t e3DType) const {
  MRC::_3DType my3d = MRC::k3DNone;

  switch (e3DType) {
    default:
      my3d = MRC::k3DNone;
      break;
  }

  return static_cast<int32_t>(my3d);
}

bool ResourceRequestor::SetSourceInfo(const source_info_t &sourceInfo) {
  if (sourceInfo.video_streams.empty()){
    MCIL_ERROR_PRINT("Video/Audio streams are empty!");
    return false;
  }

  video_info_t video_stream_info = sourceInfo.video_streams.front();
  if ((video_stream_info.encode < VIDEO_CODEC_NONE) || (video_stream_info.encode > VIDEO_CODEC_MAX) ||
      (video_stream_info.decode < VIDEO_CODEC_NONE) || (video_stream_info.decode > VIDEO_CODEC_MAX)) {
    MCIL_ERROR_PRINT("Invalid Video/Audio Codec!");
    return false;
  }

  videoResData_.width = video_stream_info.width;
  videoResData_.height = video_stream_info.height;
  videoResData_.vencode = (VideoCodec)video_stream_info.encode;
  videoResData_.vdecode = (VideoCodec)video_stream_info.decode;
  videoResData_.frameRate =
  std::round(static_cast< float_t >(video_stream_info.frame_rate.num) /
                 static_cast< float_t >(video_stream_info.frame_rate.den));
  videoResData_.escanType = 0;

  return true;
}

}  // namespace mcil
