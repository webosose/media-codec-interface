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

#ifndef SRC_RESOURCE_REQUESTOR_H_
#define SRC_RESOURCE_REQUESTOR_H_

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

typedef std::function<void()> Functor;
typedef std::function<bool(int32_t)> PlaneIDFunctor;
typedef mrc::ResourceCalculator MRC;
typedef std::multimap<std::string, int> PortResource_t;

class ResourceRequestor {
 public:
  explicit ResourceRequestor(const std::string& connectionId = "");
  virtual ~ResourceRequestor();

  const std::string GetConnectionId() const { return connectionId_; }
  void RegisterUMSPolicyActionCallback(Functor callback) { cb_ = callback; }

  bool AcquireResources(PortResource_t& resourceMMap,
          const source_info_t &sourceInfo,
          std::string& resources);

  bool ReleaseResource(std::string& resources);

  bool NotifyForeground() const;
  bool NotifyBackground() const;
  bool NotifyActivity() const;
  bool NotifyPipelineStatus(const std::string& status) const;
  void AllowPolicyAction(const bool allow);

 private:
  bool SetSourceInfo(const source_info_t &sourceInfo);
  bool PolicyActionHandler(const char *action,
      const char *resources,
      const char *requestorType,
      const char *requestorName,
      const char *connectionId);

  bool ParsePortInformation(const std::string& payload, PortResource_t& resourceMMap);
  bool ParseResources(const std::string& payload, std::string& resources);

  // translate enum type from omx player to resource calculator
  int TranslateVideoCodec(const VideoCodecType vcodec) const;
  int TranslateScanType(const int escanType) const;
  int Translate3DType(const int e3DType) const;

  mrc::ResourceListOptions CalcVdecResources();
  mrc::ResourceListOptions CalcVencResources();

  std::shared_ptr<MRC> rc_;
  std::shared_ptr<uMediaServer::ResourceManagerClient> umsRMC_;

  std::string connectionId_;
  Functor cb_;
  videoResData_t videoResData_;

  bool allowPolicy_;
};

}  // namespace decoder
}  // namespace MCIL

#endif  // SRC_RESOURCE_REQUESTOR_H_
