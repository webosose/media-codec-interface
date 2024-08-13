// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_BASE_FOURCC_H_
#define SRC_BASE_FOURCC_H_

#include "decoder_types.h"
#include "optional.h"

namespace mcil {

// Composes a Fourcc value.
constexpr uint32_t ComposeFourcc(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  return static_cast<uint32_t>(a) | (static_cast<uint32_t>(b) << 8) |
         (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24);
}

class Fourcc {
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

  explicit Fourcc(Fourcc::Value fourcc_src);
  Fourcc& operator=(const Fourcc& fourcc_src);
  ~Fourcc();

  bool operator==(const Fourcc& rhs) const {
    return fourcc_value_ == rhs.fourcc_value_;
  }

  static Fourcc FromUint32(uint32_t fourcc_src);

  static Optional<Fourcc> FromVideoPixelFormat(
      VideoPixelFormat pixel_format, bool single_planar = true);

  static Optional<Fourcc> FromV4L2PixFmt(uint32_t v4l2_pix_fmt);

  uint32_t ToV4L2PixFmt() const;

  Value getValue() { return fourcc_value_; }

  VideoPixelFormat ToVideoPixelFormat() const;

  Fourcc ToSinglePlanar() const;

  bool IsMultiPlanar() const;

  std::string ToString() const;

 private:
  Value fourcc_value_;
};

bool operator!=(const Fourcc& lhs, const Fourcc& rhs);

}  // namespace mcil

#endif  // SRC_BASE_FOURCC_H_
