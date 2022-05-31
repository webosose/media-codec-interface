// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v4l2_device.h"

#include <libdrm/drm_fourcc.h>
#include <string.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>

#include "base/log.h"
#include "v4l2/generic_v4l2_device.h"
#include "v4l2/v4l2_queue.h"

#ifdef MCIL_DEBUG_PRINT
//#undef MCIL_DEBUG_PRINT
#endif
//#define MCIL_DEBUG_PRINT MCIL_INFO_PRINT

namespace mcil {

namespace {

VideoCodecProfile
V4L2ProfileToVideoCodecProfile(VideoCodec codec, uint32_t profile) {
  switch (codec) {
    case VIDEO_CODEC_H264:
      switch (profile) {
        case V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE:
        case V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE:
          return H264PROFILE_BASELINE;
        case V4L2_MPEG_VIDEO_H264_PROFILE_MAIN:
          return H264PROFILE_MAIN;
        case V4L2_MPEG_VIDEO_H264_PROFILE_EXTENDED:
          return H264PROFILE_EXTENDED;
        case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH:
          return H264PROFILE_HIGH;
        case V4L2_MPEG_VIDEO_H264_PROFILE_STEREO_HIGH:
          return H264PROFILE_STEREOHIGH;
        case V4L2_MPEG_VIDEO_H264_PROFILE_MULTIVIEW_HIGH:
          return H264PROFILE_MULTIVIEWHIGH;
      }
      break;
    case VIDEO_CODEC_VP8:
      switch (profile) {
        case V4L2_MPEG_VIDEO_VP8_PROFILE_0:
        case V4L2_MPEG_VIDEO_VP8_PROFILE_1:
        case V4L2_MPEG_VIDEO_VP8_PROFILE_2:
        case V4L2_MPEG_VIDEO_VP8_PROFILE_3:
          return VP8PROFILE_ANY;
      }
      break;
    case VIDEO_CODEC_VP9:
      switch (profile) {
        case V4L2_MPEG_VIDEO_VP9_PROFILE_0:
          return VP9PROFILE_PROFILE0;
        case V4L2_MPEG_VIDEO_VP9_PROFILE_1:
          return VP9PROFILE_PROFILE1;
        case V4L2_MPEG_VIDEO_VP9_PROFILE_2:
          return VP9PROFILE_PROFILE2;
        case V4L2_MPEG_VIDEO_VP9_PROFILE_3:
          return VP9PROFILE_PROFILE3;
      }
      break;
    default:
      break;
  }
  MCIL_DEBUG_PRINT(", Unknown codec: %d", codec);
  return VIDEO_CODEC_PROFILE_UNKNOWN;
}

} // namespace

#if !defined(PLATFORM_EXTENSION)
// static
scoped_refptr<V4L2Device> V4L2Device::Create() {
  scoped_refptr<V4L2Device> device = new GenericV4L2Device();
  if (device->Initialize())
    return device;

  MCIL_INFO_PRINT(" Failed to create a V4L2Device");
  return nullptr;
}
#endif

// static
uint32_t V4L2Device::VideoCodecProfileToV4L2PixFmt(VideoCodecProfile profile) {
  if (profile >= H264PROFILE_MIN && profile <= H264PROFILE_MAX) {
    return V4L2_PIX_FMT_H264;
  } else if (profile >= VP8PROFILE_MIN && profile <= VP8PROFILE_MAX) {
    return V4L2_PIX_FMT_VP8;
  } else if (profile >= VP9PROFILE_MIN && profile <= VP9PROFILE_MAX) {
    return V4L2_PIX_FMT_VP9;
  } else {
    MCIL_INFO_PRINT(": Unrecognized: %s", GetProfileName(profile).c_str());
    return 0;
  }
}

// static
Size V4L2Device::AllocatedSizeFromV4L2Format(const struct v4l2_format& format) {
  Size coded_size;
  Size visible_size;
  VideoPixelFormat frame_format = PIXEL_FORMAT_UNKNOWN;
  size_t bytesperline = 0;
  size_t sizeimage = 0;

  if (V4L2_TYPE_IS_MULTIPLANAR(format.type)) {
    bytesperline =
        static_cast<size_t>(format.fmt.pix_mp.plane_fmt[0].bytesperline);
    for (size_t i = 0; i < format.fmt.pix_mp.num_planes; ++i) {
      sizeimage +=
          static_cast<size_t>(format.fmt.pix_mp.plane_fmt[i].sizeimage);
    }
    visible_size.SetSize(format.fmt.pix_mp.width, format.fmt.pix_mp.height);
    const uint32_t pix_fmt = format.fmt.pix_mp.pixelformat;
    const auto frame_fourcc = Fourcc::FromV4L2PixFmt(pix_fmt);
    if (!frame_fourcc) {
      MCIL_INFO_PRINT(" Unsupported format: %s",
                      FourccToString(pix_fmt).c_str());
      return coded_size;
    }
    frame_format = frame_fourcc->ToVideoPixelFormat();
  } else {
    bytesperline = static_cast<size_t>(format.fmt.pix.bytesperline);
    sizeimage = static_cast<size_t>(format.fmt.pix.sizeimage);
    visible_size.SetSize(format.fmt.pix.width, format.fmt.pix.height);
    const uint32_t fourcc = format.fmt.pix.pixelformat;
    const auto frame_fourcc = Fourcc::FromV4L2PixFmt(fourcc);
    if (!frame_fourcc) {
      MCIL_INFO_PRINT(" Unsupported format: %s",
                      FourccToString(fourcc).c_str());
      return coded_size;
    }
    frame_format = frame_fourcc ? frame_fourcc->ToVideoPixelFormat()
                                : PIXEL_FORMAT_UNKNOWN;
  }

  int plane_horiz_bits_per_pixel =
      VideoFrame::PlaneHorizontalBitsPerPixel(frame_format, 0);

  int total_bpp = 0;
  for (size_t i = 0; i < VideoFrame::NumPlanes(frame_format); ++i)
    total_bpp += VideoFrame::PlaneBitsPerPixel(frame_format, i);

  if (sizeimage == 0 || bytesperline == 0 || plane_horiz_bits_per_pixel == 0 ||
      total_bpp == 0 || (bytesperline * 8) % plane_horiz_bits_per_pixel != 0) {
    MCIL_INFO_PRINT(" Invalid format provided"); 
    return coded_size;
  }

  int coded_width = bytesperline * 8 / plane_horiz_bits_per_pixel;
  std::div_t res = std::div(sizeimage * 8, coded_width * total_bpp);
  int coded_height = res.quot + std::min(res.rem, 1);

  coded_size.SetSize(coded_width, coded_height);
  MCIL_DEBUG_PRINT(" coded_size= %dx%d", coded_size.width, coded_size.height);

  return coded_size;
}

// static
int32_t V4L2Device::VideoCodecProfileToV4L2H264Profile(
    VideoCodecProfile profile) {
  switch (profile) {
    case H264PROFILE_BASELINE:
      return V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
    case H264PROFILE_MAIN:
      return V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
    case H264PROFILE_EXTENDED:
      return V4L2_MPEG_VIDEO_H264_PROFILE_EXTENDED;
    case H264PROFILE_HIGH:
      return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
    case H264PROFILE_HIGH10PROFILE:
      return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10;
    case H264PROFILE_HIGH422PROFILE:
      return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_422;
    case H264PROFILE_HIGH444PREDICTIVEPROFILE:
      return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_444_PREDICTIVE;
    case H264PROFILE_SCALABLEBASELINE:
      return V4L2_MPEG_VIDEO_H264_PROFILE_SCALABLE_BASELINE;
    case H264PROFILE_SCALABLEHIGH:
      return V4L2_MPEG_VIDEO_H264_PROFILE_SCALABLE_HIGH;
    case H264PROFILE_STEREOHIGH:
      return V4L2_MPEG_VIDEO_H264_PROFILE_STEREO_HIGH;
    case H264PROFILE_MULTIVIEWHIGH:
      return V4L2_MPEG_VIDEO_H264_PROFILE_MULTIVIEW_HIGH;
    default:
      break;
  }
  return -1;
}

// static
int32_t V4L2Device::H264LevelIdcToV4L2H264Level(uint8_t level_idc) {
  switch (level_idc) {
    case 10:
      return V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
    case 9:
      return V4L2_MPEG_VIDEO_H264_LEVEL_1B;
    case 11:
      return V4L2_MPEG_VIDEO_H264_LEVEL_1_1;
    case 12:
      return V4L2_MPEG_VIDEO_H264_LEVEL_1_2;
    case 13:
      return V4L2_MPEG_VIDEO_H264_LEVEL_1_3;
    case 20:
      return V4L2_MPEG_VIDEO_H264_LEVEL_2_0;
    case 21:
      return V4L2_MPEG_VIDEO_H264_LEVEL_2_1;
    case 22:
      return V4L2_MPEG_VIDEO_H264_LEVEL_2_2;
    case 30:
      return V4L2_MPEG_VIDEO_H264_LEVEL_3_0;
    case 31:
      return V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
    case 32:
      return V4L2_MPEG_VIDEO_H264_LEVEL_3_2;
    case 40:
      return V4L2_MPEG_VIDEO_H264_LEVEL_4_0;
    case 41:
      return V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
    case 42:
      return V4L2_MPEG_VIDEO_H264_LEVEL_4_2;
    case 50:
      return V4L2_MPEG_VIDEO_H264_LEVEL_5_0;
    case 51:
      return V4L2_MPEG_VIDEO_H264_LEVEL_5_1;
    default:
      break;
  }
  return -1;
}

// static
size_t V4L2Device::GetNumPlanesOfV4L2PixFmt(uint32_t pix_fmt) {
  auto fourcc = Fourcc::FromV4L2PixFmt(pix_fmt);
  if (fourcc && fourcc->IsMultiPlanar()) {
    return VideoFrame::NumPlanes(fourcc->ToVideoPixelFormat());
  }
  return 1u;
}

// static
scoped_refptr<VideoFrame> V4L2Device::VideoFrameFromV4L2Format(
    const struct v4l2_format& format) {
  if (!V4L2_TYPE_IS_MULTIPLANAR(format.type)) {
    MCIL_INFO_PRINT(": buf_type[%d] is not multiplanar", format.type);
    return nullptr;
  }

  const v4l2_pix_format_mplane& pix_mp = format.fmt.pix_mp;
  const uint32_t& pix_fmt = pix_mp.pixelformat;
  const auto video_fourcc = Fourcc::FromV4L2PixFmt(pix_fmt);
  if (!video_fourcc) {
    MCIL_INFO_PRINT(": Failed to convert Fourcc value: %s",
                    FourccToString(pix_fmt).c_str());
    return nullptr;
  }

  Size size(format.fmt.pix_mp.width, format.fmt.pix_mp.height);
  scoped_refptr<VideoFrame> video_frame = VideoFrame::Create(size);

  size_t num_buffers = pix_mp.num_planes;
  video_frame->format = video_fourcc->ToVideoPixelFormat();
  MCIL_DEBUG_PRINT(": format[%d]", video_frame->format);
  const size_t num_color_planes = VideoFrame::NumPlanes(video_frame->format);
  if (num_color_planes == 0) {
    MCIL_INFO_PRINT(": Unsupported video format for NumPlanes(): %d",
                    video_frame->format);
    return nullptr;
  }
  if (num_buffers > num_color_planes) {
    MCIL_INFO_PRINT(": pix_mp.num_planes[%d], should not be \
                    larger than NumPlanes for %d",
                    pix_mp.num_planes, video_frame->format);
    return nullptr;
  }

  video_frame->color_planes.reserve(num_color_planes);
  for (size_t i = 0; i < num_buffers; ++i) {
    const v4l2_plane_pix_format& plane_format = pix_mp.plane_fmt[i];
    video_frame->color_planes.emplace_back(
        static_cast<int32_t>(plane_format.bytesperline), 0u,
        plane_format.sizeimage);
  }
  if (num_color_planes > num_buffers) {
    const int32_t y_stride = video_frame->color_planes[0].stride;
    const size_t y_stride_abs = static_cast<size_t>(y_stride);
    switch (pix_fmt) {
      case V4L2_PIX_FMT_NV12:
        video_frame->color_planes.emplace_back(
            y_stride, y_stride_abs * pix_mp.height,
            y_stride_abs * pix_mp.height / 2);
        break;
      case V4L2_PIX_FMT_YUV420:
      case V4L2_PIX_FMT_YVU420: {
        if (y_stride % 2 != 0 || pix_mp.height % 2 != 0) {
          MCIL_INFO_PRINT(": Plane-Y stride and height should be even; \
              stride: [%d], height:[%d]", y_stride, pix_mp.height);
          return nullptr;
        }
        const int32_t half_stride = y_stride / 2;
        const size_t plane_0_area = y_stride_abs * pix_mp.height;
        const size_t plane_1_area = plane_0_area / 4;
        video_frame->color_planes.emplace_back(
            half_stride, plane_0_area, plane_1_area);
        video_frame->color_planes.emplace_back(
            half_stride, plane_0_area + plane_1_area, plane_1_area);
        break;
      }
      default:
        MCIL_INFO_PRINT(": Cannot derive stride for each plane for pixel \
                        format: %s", FourccToString(pix_fmt).c_str());
        return nullptr;
    }
  }

  video_frame->is_multi_planar = !(num_buffers == 1);
  MCIL_DEBUG_PRINT(": num_planes[%lu], is_multi_planar[%d]",
      video_frame->color_planes.size(), video_frame->is_multi_planar);

  for (size_t i = 0; i < video_frame->color_planes.size(); ++i) {
    MCIL_DEBUG_PRINT(", stride: %d, offset: %lu, size: %lu",
                     video_frame->color_planes[i].stride,
                     video_frame->color_planes[i].offset,
                     video_frame->color_planes[i].size);
  }
  return video_frame;
}

V4L2Device::V4L2Device() {
  MCIL_DEBUG_PRINT(": Ctor");
}

V4L2Device::~V4L2Device() {
  MCIL_DEBUG_PRINT(": Dtor");
}

void V4L2Device::GetSupportedResolution(uint32_t pixelformat,
                                        Size* min_resolution,
                                        Size* max_resolution) {
  max_resolution->width = 0;
  max_resolution->height = 0;

  min_resolution->width = 0;
  min_resolution->height = 0;

  v4l2_frmsizeenum frame_size;
  memset(&frame_size, 0, sizeof(frame_size));
  frame_size.pixel_format = pixelformat;
  for (; Ioctl(VIDIOC_ENUM_FRAMESIZES, &frame_size) == 0; ++frame_size.index) {
    if (frame_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
      if (frame_size.discrete.width >=
              static_cast<uint32_t>(max_resolution->width) &&
          frame_size.discrete.height >=
              static_cast<uint32_t>(max_resolution->height)) {
        max_resolution->width = frame_size.discrete.width;
        max_resolution->height = frame_size.discrete.height;
      }

      if ((!max_resolution->width || !max_resolution->height) ||
          (frame_size.discrete.width <=
               static_cast<uint32_t>(min_resolution->width) &&
           frame_size.discrete.height <=
               static_cast<uint32_t>(min_resolution->height))) {
        min_resolution->width = frame_size.discrete.width;
        min_resolution->height = frame_size.discrete.height;
      }
    } else if (frame_size.type == V4L2_FRMSIZE_TYPE_STEPWISE ||
               frame_size.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
      max_resolution->width = frame_size.stepwise.max_width;
      max_resolution->height = frame_size.stepwise.max_height;
      min_resolution->width = frame_size.stepwise.min_width;
      min_resolution->height = frame_size.stepwise.min_height;
      break;
    }
  }

  if (!max_resolution->width || !max_resolution->height) {
    max_resolution->width = 1920;
    max_resolution->height = 1080;
  }
  if (!min_resolution->width || !min_resolution->height) {
    min_resolution->width = 16;
    min_resolution->height = 16;
  }
}

std::vector<uint32_t> V4L2Device::EnumerateSupportedPixelformats(
    v4l2_buf_type buf_type) {
  MCIL_DEBUG_PRINT(": buf_type[%d]", buf_type);

  std::vector<uint32_t> pixelformats;

  v4l2_fmtdesc fmtdesc;
  memset(&fmtdesc, 0, sizeof(fmtdesc));
  fmtdesc.type = buf_type;

  for (; Ioctl(VIDIOC_ENUM_FMT, &fmtdesc) == 0; ++fmtdesc.index) {
    MCIL_DEBUG_PRINT(": Found %s (0x%x)",
                     fmtdesc.description, fmtdesc.pixelformat);
    pixelformats.push_back(fmtdesc.pixelformat);
  }

  return pixelformats;
}

std::vector<VideoCodecProfile> V4L2Device::V4L2PixFmtToVideoCodecProfiles(
    uint32_t pix_fmt) {
  MCIL_DEBUG_PRINT(": pix_fmt[0x%x]", pix_fmt);

  auto get_supported_profiles = [this](
      VideoCodec codec, std::vector<VideoCodecProfile>* profiles) {
    uint32_t query_id = 0;
    switch (codec) {
      case VIDEO_CODEC_H264:
        query_id = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
        break;
      case VIDEO_CODEC_VP8:
        query_id = V4L2_CID_MPEG_VIDEO_VP8_PROFILE;
        break;
      case VIDEO_CODEC_VP9:
        query_id = V4L2_CID_MPEG_VIDEO_VP9_PROFILE;
        break;
      default:
        return false;
    }

    v4l2_queryctrl query_ctrl;
    memset(&query_ctrl, 0, sizeof(query_ctrl));
    query_ctrl.id = query_id;
    if (Ioctl(VIDIOC_QUERYCTRL, &query_ctrl) != 0) {
      return false;
    }
    v4l2_querymenu query_menu;
    memset(&query_menu, 0, sizeof(query_menu));
    query_menu.id = query_ctrl.id;
    for (query_menu.index = query_ctrl.minimum;
         static_cast<int>(query_menu.index) <= query_ctrl.maximum;
         query_menu.index++) {
      if (Ioctl(VIDIOC_QUERYMENU, &query_menu) == 0) {
        const VideoCodecProfile profile =
            V4L2ProfileToVideoCodecProfile(codec, query_menu.index);
        if (profile != VIDEO_CODEC_PROFILE_UNKNOWN)
          profiles->push_back(profile);
      }
    }
    return true;
  };

  std::vector<VideoCodecProfile> profiles;
  switch (pix_fmt) {
    case V4L2_PIX_FMT_H264:
      if (!get_supported_profiles(VIDEO_CODEC_H264, &profiles)) {
        MCIL_INFO_PRINT(": Driver doesn't support QUERY H264 profiles");
        profiles = {
            H264PROFILE_BASELINE,
            H264PROFILE_MAIN,
            H264PROFILE_HIGH,
        };
      }
      break;
    case V4L2_PIX_FMT_VP8:
      profiles = {VP8PROFILE_ANY};
      break;
    case V4L2_PIX_FMT_VP9:
      if (!get_supported_profiles(VIDEO_CODEC_VP9, &profiles)) {
        MCIL_INFO_PRINT(": Driver doesn't support QUERY VP9 profiles");
        profiles = {VP9PROFILE_PROFILE0};
      }
      break;
    default:
      return {};
  }

  // Erase duplicated profiles.
  std::sort(profiles.begin(), profiles.end());
  profiles.erase(std::unique(profiles.begin(), profiles.end()), profiles.end());
  return profiles;
}

Optional<struct v4l2_event> V4L2Device::DequeueEvent() {
  struct v4l2_event event;
  memset(&event, 0, sizeof(event));

  if (Ioctl(VIDIOC_DQEVENT, &event) != 0) {
    MCIL_DEBUG_PRINT(": Failed to dequeue event");
    return nullopt;
  }

  return event;
}

Optional<struct v4l2_ext_control> V4L2Device::GetCtrl(uint32_t ctrl_id) {
  struct v4l2_ext_control ctrl;
  memset(&ctrl, 0, sizeof(struct v4l2_ext_control));
  struct v4l2_ext_controls ext_ctrls;
  memset(&ext_ctrls, 0, sizeof(ext_ctrls));

  ctrl.id = ctrl_id;
  ext_ctrls.controls = &ctrl;
  ext_ctrls.count = 1;

  if (Ioctl(VIDIOC_G_EXT_CTRLS, &ext_ctrls) != 0) {
    MCIL_INFO_PRINT(": Failed to get control");
    return nullopt;
  }

  return ctrl;
}

bool V4L2Device::SetCtrl(
    uint32_t ctrl_class, uint32_t ctrl_id, int32_t ctrl_val) {
  MCIL_DEBUG_PRINT(" class[%x], id[%x], val[%d]",
                   ctrl_class, ctrl_id, ctrl_val);

  struct v4l2_ext_control ext_ctrl;
  memset(&ext_ctrl, 0, sizeof(ext_ctrl));
  ext_ctrl.id = ctrl_id;
  ext_ctrl.value = ctrl_val;

  struct v4l2_ext_controls ext_ctrls;
  memset(&ext_ctrls, 0, sizeof(ext_ctrls));
  ext_ctrls.ctrl_class = ctrl_class;
  ext_ctrls.count = 1;
  ext_ctrls.controls = &ext_ctrl;

  if (Ioctl(VIDIOC_S_EXT_CTRLS, &ext_ctrls) != 0) {
    MCIL_INFO_PRINT(": Failed to set control id[%x]", ctrl_id);
    return false;
  }

  return true;
}

bool V4L2Device::IsCtrlExposed(uint32_t ctrl_id) {
  struct v4l2_queryctrl query_ctrl;
  memset(&query_ctrl, 0, sizeof(query_ctrl));
  query_ctrl.id = ctrl_id;

  return Ioctl(VIDIOC_QUERYCTRL, &query_ctrl) == 0;
}

bool V4L2Device::SetGOPLength(uint32_t gop_length) {
  MCIL_DEBUG_PRINT(" gop_length[%d]", gop_length);

  if (!SetCtrl(V4L2_CTRL_CLASS_MPEG,
               V4L2_CID_MPEG_VIDEO_GOP_SIZE, gop_length)) {
    if (gop_length == 0) {
      v4l2_query_ext_ctrl queryctrl;
      memset(&queryctrl, 0, sizeof(queryctrl));

      queryctrl.id = V4L2_CTRL_CLASS_MPEG | V4L2_CID_MPEG_VIDEO_GOP_SIZE;
      if (Ioctl(VIDIOC_QUERY_EXT_CTRL, &queryctrl) == 0) {
        MCIL_INFO_PRINT(" Unable to set GOP to 0, instead using max[%lld]",
                        queryctrl.maximum);
        return SetCtrl(V4L2_CTRL_CLASS_MPEG,
                       V4L2_CID_MPEG_VIDEO_GOP_SIZE, queryctrl.maximum);
      }
    }
    return false;
  }
  return true;
}

scoped_refptr<V4L2Queue> V4L2Device::GetQueue(enum v4l2_buf_type buffer_type) {
  MCIL_DEBUG_PRINT(": Called");

  switch (buffer_type) {
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
      break;
    default:
      MCIL_INFO_PRINT(": Unsupported V4L2 queue type [%d]", buffer_type);
      return nullptr;
  }

  auto it = queues_.find(buffer_type);
  if (it != queues_.end())
    return it->second;

  scoped_refptr<V4L2Queue> queue = V4L2Queue::Create(
      this, buffer_type, std::bind(&V4L2Device::OnQueueDestroyed,
                                   this, std::placeholders::_1));
  queues_[buffer_type] = queue.get();
  return queue;
}

SupportedProfiles
V4L2Device::EnumerateSupportedDecodeProfiles(const size_t num_formats,
                                             const uint32_t pixelformats[]) {
  SupportedProfiles profiles;
  const auto& supported_pixelformats =
      EnumerateSupportedPixelformats(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);

  for (uint32_t pixelformat : supported_pixelformats) {
    if (std::find(pixelformats, pixelformats + num_formats, pixelformat) ==
        pixelformats + num_formats)
      continue;

    SupportedProfile profile;
    GetSupportedResolution(pixelformat, &profile.min_resolution,
                           &profile.max_resolution);

    const auto video_codec_profiles =
        V4L2PixFmtToVideoCodecProfiles(pixelformat);

    for (const auto& video_codec_profile : video_codec_profiles) {
      profile.profile = video_codec_profile;
      profiles.push_back(profile);

      MCIL_DEBUG_PRINT(": profile [%d], min[%dx%d], max[%dx%d]", profile.profile,
                       profile.min_resolution.width, profile.min_resolution.width,
                       profile.max_resolution.width, profile.max_resolution.width);
    }
  }

  return profiles;
}

SupportedProfiles V4L2Device::EnumerateSupportedEncodeProfiles() {
  SupportedProfiles profiles;
  const auto& supported_pixelformats =
      EnumerateSupportedPixelformats(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

  for (const auto& pixelformat : supported_pixelformats) {
    SupportedProfile profile;
    Size min_resolution;
    GetSupportedResolution(pixelformat, &min_resolution,
                           &profile.max_resolution);
    const auto video_codec_profiles =
        V4L2PixFmtToVideoCodecProfiles(pixelformat);

    for (const auto& video_codec_profile : video_codec_profiles) {
      profile.profile = video_codec_profile;
      profiles.push_back(profile);

      MCIL_DEBUG_PRINT(": profile [%d], min[%dx%d], max[%dx%d]", profile.profile,
                       profile.min_resolution.width, profile.min_resolution.width,
                       profile.max_resolution.width, profile.max_resolution.width);
    }
  }

  return profiles;
}

void V4L2Device::OnQueueDestroyed(const v4l2_buf_type buf_type) {
  auto it = queues_.find(buf_type);
  queues_.erase(it);
}

}  //  namespace mcil
