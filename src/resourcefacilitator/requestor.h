// Copyright (c) 2019-2021 LG Electronics, Inc.
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

#ifndef SRC_ResourceFacilitator_REQUESTOR_H_
#define SRC_ResourceFacilitator_REQUESTOR_H_

#include <string>
#include <functional>
#include <memory>
#include <map>
#include <resource_calculator.h>

#include "base/decoder_types.h"
#include "base/fourcc_value.h"

namespace mrc { class ResourceCalculator; }

namespace uMediaServer {   class ResourceManagerClient; }

namespace mcil {

namespace decoder {

struct videoResData_t {
  VideoCodecType vcodec;
  VideoCodecType vdecode;
  VideoCodecType vencode;

  int width;
  int height;
  int frameRate;
  int escanType;
  int e3DType;
  int parWidth;  // pixel-aspect-ratio width
  int parHeight; // pixel-aspect-ratio height
};

struct stream_info_t {
  int32_t codec;
  int32_t decode;
  int32_t encode;
  uint64_t bit_rate;
};

struct rational_t {
  int32_t num;
  int32_t den;
};

struct video_info_t : stream_info_t {
  uint32_t width;
  uint32_t height;
  rational_t frame_rate;
};

struct program_info_t {
  uint32_t video_stream;
};

struct source_info_t {
  std::string container;
  std::vector<program_info_t> programs;
  std::vector<video_info_t> video_streams;
};

struct media_info_t {
  std::string mediaId;
};

struct result_t {
  bool state;
  std::string mediaId;
};

struct error_t {
  int32_t errorCode;
  std::string errorText;
  std::string mediaId;
};

struct load_param_t {
  int32_t displayPath;
  std::string videoDisplayMode;
  std::string windowId;
  std::string uri;
};

typedef enum {
  MCIL_DEFAULT_DISPLAY = 0,
  MCIL_PRIMARY_DISPLAY = 0,
  MCIL_SECONDARY_DISPLAY,
} MCIL_DISPLAY_PATH;

typedef enum {
  MCIL_NOTIFY_LOAD_COMPLETED = 0,
  MCIL_NOTIFY_UNLOAD_COMPLETED,
  MCIL_NOTIFY_SOURCE_INFO,
  MCIL_NOTIFY_END_OF_STREAM,
  MCIL_NOTIFY_PLAYING,
  MCIL_NOTIFY_PAUSED,
  MCIL_NOTIFY_ERROR,
  MCIL_NOTIFY_VIDEO_INFO,
  MCIL_NOTIFY_ACTIVITY,
  MCIL_NOTIFY_ACQUIRE_RESOURCE,
  MCIL_NOTIFY_MAX
} MCIL_NOTIFY_TYPE_T;

typedef std::function<void()> Functor;
typedef std::function<bool(int32_t)> PlaneIDFunctor;
typedef mrc::ResourceCalculator MRC;
typedef std::multimap<std::string, int> PortResource_t;

class ResourceRequestor {
 public:
  explicit ResourceRequestor(const std::string& appId,
          const std::string& connectionId = "");
  virtual ~ResourceRequestor();

  const std::string getConnectionId() const { return connectionId_; }
  void registerUMSPolicyActionCallback(Functor callback) { cb_ = callback; }
  void registerPlaneIdCallback(PlaneIDFunctor callback) {
    planeIdCb_ = callback;
  }
  bool acquireResources(PortResource_t& resourceMMap,
          const source_info_t &sourceInfo,
          const std::string& display_mode,
          std::string& resources,
          const int32_t display_path = 0);

  bool releaseResource(std::string& resources);

  bool endOfStream();

  bool notifyForeground() const;
  bool notifyBackground() const;
  bool notifyActivity() const;
  bool notifyPipelineStatus(const std::string& status) const;
  void allowPolicyAction(const bool allow);
  void setAppId(std::string id);

 private:
  bool setSourceInfo(const source_info_t &sourceInfo);
  bool policyActionHandler(const char *action,
      const char *resources,
      const char *requestorType,
      const char *requestorName,
      const char *connectionId);
  void planeIdHandler(int32_t planePortIdx);
  bool parsePortInformation(const std::string& payload, PortResource_t& resourceMMap);
  bool parseResources(const std::string& payload, std::string& resources);

  // translate enum type from omx player to resource calculator
  int translateVideoCodec(const VideoCodecType vcodec) const;
  int translateScanType(const int escanType) const;
  int translate3DType(const int e3DType) const;
  mrc::ResourceListOptions calcVdecResources();
  mrc::ResourceListOptions calcVencResources();
  mrc::ResourceListOptions calcDisplayResource(const std::string &display_mode);

  std::shared_ptr<MRC> rc_;
  std::shared_ptr<uMediaServer::ResourceManagerClient> umsRMC_;

  std::string appId_;
  std::string instanceId_;
  std::string connectionId_;
  Functor cb_;
  PlaneIDFunctor planeIdCb_;
  videoResData_t videoResData_;

  bool allowPolicy_;
};

}  // namespace decoder
}  // namespace MCIL

#endif  // SRC_ResourceFacilitator_REQUESTOR_H_
