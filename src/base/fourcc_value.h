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


#ifndef SRC_BASE_FOURCC_VALUE_H_
#define SRC_BASE_FOURCC_VALUE_H_

#include "decoder_types.h"

#define USE_V4L2_CODEC

namespace mcil {

// Composes a Fourcc value.
constexpr uint32_t ComposeFourcc(char a, char b, char c, char d) {
  return static_cast<uint32_t>(a) | (static_cast<uint32_t>(b) << 8) |
         (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24);
}

class FourccValue {
 public:
  enum Value : uint32_t {
    AR24 = ComposeFourcc('A', 'R', '2', '4'),

    AB24 = ComposeFourcc('A', 'B', '2', '4'),

    XR24 = ComposeFourcc('X', 'R', '2', '4'),

    XB24 = ComposeFourcc('X', 'B', '2', '4'),

    RGB4 = ComposeFourcc('R', 'G', 'B', '4'),

    YU12 = ComposeFourcc('Y', 'U', '1', '2'),

    YV12 = ComposeFourcc('Y', 'V', '1', '2'),

    YM12 = ComposeFourcc('Y', 'M', '1', '2'),

    YM21 = ComposeFourcc('Y', 'M', '2', '1'),

    YUYV = ComposeFourcc('Y', 'U', 'Y', 'V'),

    NV12 = ComposeFourcc('N', 'V', '1', '2'),

    NV21 = ComposeFourcc('N', 'V', '2', '1'),

    NM12 = ComposeFourcc('N', 'M', '1', '2'),

    NM21 = ComposeFourcc('N', 'M', '2', '1'),

    YM16 = ComposeFourcc('Y', 'M', '1', '6'),

    MT21 = ComposeFourcc('M', 'T', '2', '1'),

    MM21 = ComposeFourcc('M', 'M', '2', '1'),

    P010 = ComposeFourcc('P', '0', '1', '0'),

    NONE = ComposeFourcc('N', 'O', 'N', 'E'),
  };

  explicit FourccValue(FourccValue::Value fourcc);
  FourccValue& operator=(const FourccValue& fourcc);
  ~FourccValue();

  bool operator==(const FourccValue& rhs) const { return value_ == rhs.value_; }

  static FourccValue FromUint32(uint32_t fourcc);

  static FourccValue FromMCILPixelFormat(VideoPixelFormat pixel_format,
                                         bool single_planar = true);

  static Optional<FourccValue> FromV4L2PixFmt(uint32_t v4l2_pix_fmt);

  uint32_t ToV4L2PixFmt() const;

  Value value() { return value_; }

  VideoPixelFormat ToMCILPixelFormat() const;

  FourccValue ToSinglePlanar() const;

  bool IsMultiPlanar() const;

  std::string ToString() const;

 private:
  Value value_;
};

bool operator!=(const FourccValue& lhs, const FourccValue& rhs);

}  // namespace mcil

#endif  // SRC_BASE_FOURCC_VALUE_H_
