// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "generic_v4l2_device.h"

#include <libdrm/drm_fourcc.h>
#include <poll.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>

#include "base/log.h"
#include "v4l2/v4l2_queue.h"

#ifdef MCIL_DEBUG_PRINT
//#undef MCIL_DEBUG_PRINT
#endif
//#define MCIL_DEBUG_PRINT MCIL_INFO_PRINT

namespace mcil {

namespace {

uint32_t V4L2PixFmtToDrmFormat(uint32_t format) {
  switch (format) {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV12M:
      return DRM_FORMAT_NV12;

    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YUV420M:
      return DRM_FORMAT_YUV420;

    case V4L2_PIX_FMT_YVU420:
      return DRM_FORMAT_YVU420;

    case V4L2_PIX_FMT_RGB32:
      return DRM_FORMAT_ARGB8888;

    default:
      break;
  }
  MCIL_INFO_PRINT(": Unrecognized: %s", FourccToString(format).c_str());
  return 0;
}

} // namespace

GenericV4L2Device::GenericV4L2Device(DeviceType device_type)
  : V4L2Device(device_type) {
  MCIL_DEBUG_PRINT(": Ctor");
}

GenericV4L2Device::~GenericV4L2Device() {
  MCIL_DEBUG_PRINT(": Dtor");
  CloseDevice();
}

bool GenericV4L2Device::Initialize() {
  MCIL_DEBUG_PRINT(": called");
  return true;
}

bool GenericV4L2Device::Open(DeviceType type, uint32_t v4l2_pixfmt) {
  std::string path = GetDevicePathFor(type, v4l2_pixfmt);
  if (path.empty()) {
    MCIL_INFO_PRINT(": No devices supporting: %s",
                    FourccToString(v4l2_pixfmt).c_str());
    return false;
  }

  if (!OpenDevice(path, type)) {
    MCIL_INFO_PRINT(": Failed opening: %s", path.c_str());
    return false;
  }

  device_poll_interrupt_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (device_poll_interrupt_fd_ <= 0) {
    MCIL_INFO_PRINT(": Failed creating a poll interrupt fd");
    return false;
  }

  MCIL_DEBUG_PRINT(": Success path: %s", path.c_str());
  return true;
}

int GenericV4L2Device::Ioctl(int request, void* arg) {
  if (device_fd_ == -1) {
    MCIL_INFO_PRINT(": Invalid device_fd_[%d]", device_fd_);
    return 0;
  }

  return HANDLE_EINTR(ioctl(device_fd_, request, arg));
}

bool GenericV4L2Device::Poll(bool poll_device, bool* event_pending) {
  struct pollfd pollfds[2];
  nfds_t nfds;
  int pollfd = -1;

  pollfds[0].fd = device_poll_interrupt_fd_;
  pollfds[0].events = POLLIN | POLLERR;
  pollfds[0].revents = 0;
  nfds = 1;

  if (poll_device) {
    MCIL_DEBUG_PRINT(": adding device fd to poll() set");
    pollfds[nfds].fd = device_fd_;
    pollfds[nfds].events = POLLIN | POLLOUT | POLLERR | POLLPRI;
    pollfds[nfds].revents = 0;
    pollfd = nfds;
    nfds++;
  }

  if (HANDLE_EINTR(poll(pollfds, nfds, -1)) == -1) {
    MCIL_DEBUG_PRINT(": poll() failed");
    return false;
  }

  *event_pending = (pollfd != -1 && pollfds[pollfd].revents & POLLPRI);
  return true;
}

bool GenericV4L2Device::SetDevicePollInterrupt() {
  const uint64_t buf = 1;
  if (HANDLE_EINTR(write(device_poll_interrupt_fd_, &buf, sizeof(buf))) ==
      -1) {
    MCIL_INFO_PRINT(": write() failed");
    return false;
  }
  return true;
}

bool GenericV4L2Device::ClearDevicePollInterrupt() {
  uint64_t buf;
  if (HANDLE_EINTR(read(device_poll_interrupt_fd_, &buf, sizeof(buf))) ==
      -1) {
    if (errno == EAGAIN) {
      return true;
    } else {
      MCIL_INFO_PRINT(": read() failed");
      return false;
    }
  }
  return true;
}

void* GenericV4L2Device::Mmap(void* addr, unsigned int len, int prot,
                              int flags, unsigned int offset) {
  return mmap(addr, len, prot, flags, device_fd_, offset);
}

void GenericV4L2Device::Munmap(void* addr, unsigned int len) {
  munmap(addr, len);
}

std::vector<int32_t> GenericV4L2Device::GetDmabufsForV4L2Buffer(
    int index,
    size_t num_planes,
    enum v4l2_buf_type buffer_type) {
  std::vector<int32_t> dmabuf_fds;
  for (size_t i = 0; i < num_planes; ++i) {
    struct v4l2_exportbuffer expbuf;
    memset(&expbuf, 0, sizeof(expbuf));
    expbuf.type = buffer_type;
    expbuf.index = index;
    expbuf.plane = i;
    expbuf.flags = O_CLOEXEC;
    if (Ioctl(VIDIOC_EXPBUF, &expbuf) != 0) {
      dmabuf_fds.clear();
      break;
    }

    MCIL_DEBUG_PRINT(": expbuf.fd [%d]", expbuf.fd);
    dmabuf_fds.push_back(expbuf.fd);
  }

  return dmabuf_fds;
}

bool GenericV4L2Device::CanCreateEGLImageFrom(const Fourcc fourcc) const {
  MCIL_DEBUG_PRINT(": fourcc[%s]", fourcc.ToString().c_str());

  static uint32_t kEGLImageDrmFmtsSupported[] = {
    DRM_FORMAT_ARGB8888,
    DRM_FORMAT_NV12,
    DRM_FORMAT_YVU420,
  };

  return std::find(
             kEGLImageDrmFmtsSupported,
             kEGLImageDrmFmtsSupported + ARRAY_SIZE(kEGLImageDrmFmtsSupported),
             V4L2PixFmtToDrmFormat(fourcc.ToV4L2PixFmt())) !=
         kEGLImageDrmFmtsSupported + ARRAY_SIZE(kEGLImageDrmFmtsSupported);
}

unsigned int GenericV4L2Device::GetTextureTarget() const {
  return GL_TEXTURE_EXTERNAL_OES;
}

SupportedProfiles GenericV4L2Device::GetSupportedDecodeProfiles(
      const size_t num_formats,
      const uint32_t pixelformats[]) {
  SupportedProfiles supported_profiles;

  DeviceType type = V4L2_DECODER;
  const auto& devices = GetDevicesForType(type);
  for (const auto& device : devices) {
    if (!OpenDevice(device.first, type)) {
      MCIL_INFO_PRINT(": Failed opening: %s", device.first.c_str());
      continue;
    }

    const auto& profiles =
        EnumerateSupportedDecodeProfiles(num_formats, pixelformats);
    supported_profiles.insert(supported_profiles.end(), profiles.begin(),
                              profiles.end());
    CloseDevice();
  }

  for (const auto profile : supported_profiles) {
    MCIL_DEBUG_PRINT(": profile[%d] min[%dx%d] max[%dx%d]", profile.profile,
                     profile.min_resolution.width, profile.min_resolution.height,
                     profile.max_resolution.width, profile.max_resolution.height);
  }
  return supported_profiles;
}

SupportedProfiles GenericV4L2Device::GetSupportedEncodeProfiles() {
  SupportedProfiles supported_profiles;

  DeviceType type = V4L2_ENCODER;
  const auto& devices = GetDevicesForType(type);
  for (const auto& device : devices) {
    if (!OpenDevice(device.first, type)) {
      MCIL_INFO_PRINT(": Failed opening: %s", device.first.c_str());
      continue;
    }

    const auto& profiles = EnumerateSupportedEncodeProfiles();
    supported_profiles.insert(supported_profiles.end(), profiles.begin(),
                              profiles.end());
    CloseDevice();
  }

  for (const auto profile : supported_profiles) {
    MCIL_DEBUG_PRINT(": profile[%d] min[%dx%d] max[%dx%d]", profile.profile,
                    profile.min_resolution.width, profile.min_resolution.height,
                    profile.max_resolution.width, profile.max_resolution.height);
  }
  return supported_profiles;
}

void GenericV4L2Device::EnumerateDevicesForType(DeviceType type) {
  MCIL_DEBUG_PRINT(": type[%d]", type);

  /* On Raspberry Pi 4, video device files are "video1*" */
  static const std::string kDecoderDevicePattern = "/dev/video10";
  static const std::string kEncoderDevicePattern = "/dev/video11";
  static const std::string kImageProcessorDevicePattern = "/dev/video12";
  static const std::string kJpegDecoderDevicePattern = "/dev/jpeg-dec";

  std::string device_pattern;
  v4l2_buf_type buf_type;
  switch (type) {
    case V4L2_DECODER:
      device_pattern = kDecoderDevicePattern;
      buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
      break;
    case V4L2_ENCODER:
      device_pattern = kEncoderDevicePattern;
      buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      break;
    case IMAGE_PROCESSOR:
      device_pattern = kImageProcessorDevicePattern;
      buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
      break;
    case JPEG_DECODER:
      device_pattern = kJpegDecoderDevicePattern;
      buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
      break;
    default:
      break;
  }

  std::vector<std::string> candidate_paths;

  // TODO(posciak): Remove this legacy unnumbered device once
  // all platforms are updated to use numbered devices.
  candidate_paths.push_back(device_pattern);

  Devices devices;
  for (const auto& path : candidate_paths) {
    if (!OpenDevice(path, type)) {
      MCIL_INFO_PRINT(": Failed opening: %s", path.c_str());
      continue;
    }

    const auto& supported_pixelformats =
        EnumerateSupportedPixelformats(buf_type);
    if (!supported_pixelformats.empty()) {
      MCIL_DEBUG_PRINT(": Found device: %s", path.c_str());
      devices.push_back(std::make_pair(path, supported_pixelformats));
    }

    CloseDevice();
  }

  devices_by_type_[type] = devices;
}

std::vector<uint32_t>
GenericV4L2Device::PreferredInputFormat(DeviceType type) const {
  if (type == V4L2_ENCODER)
    return { V4L2_PIX_FMT_YUV420, V4L2_PIX_FMT_NV12M, V4L2_PIX_FMT_NV12 };
  return {};
}

bool GenericV4L2Device::OpenDevice(const std::string& path, DeviceType type) {
  MCIL_DEBUG_PRINT(": path = %s", path.c_str());

  if (device_fd_ > 0) {
    MCIL_INFO_PRINT(": device_fd_[%d] already Open", device_fd_);
    return true;
  }

  device_fd_ =
      HANDLE_EINTR(open(path.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC));
  if (device_fd_ <= 0) {
    MCIL_INFO_PRINT(": type[%d], path: %s Failed", type, path.c_str());
    return false;
  }

  MCIL_DEBUG_PRINT(": type[%d], path: %s Success FD [%d]",
                   type, path.c_str(), device_fd_);
  return true;
}

void GenericV4L2Device::CloseDevice() {
  if (device_fd_ != -1) {
    int ret = IGNORE_EINTR(close(device_fd_));
    MCIL_DEBUG_PRINT(": Success FD [%d]", device_fd_);
    device_fd_ = -1;
  }

  if (device_poll_interrupt_fd_ != -1) {
    close(device_poll_interrupt_fd_);
    device_poll_interrupt_fd_ = -1;
  }
}

std::string
GenericV4L2Device::GetDevicePathFor(DeviceType type, uint32_t pixfmt) {
  const Devices& devices = GetDevicesForType(type);
  for (const auto& device : devices) {
    if (std::find(device.second.begin(), device.second.end(), pixfmt) !=
        device.second.end())
      return device.first;
  }

  return std::string();
}

const GenericV4L2Device::Devices&
GenericV4L2Device::GetDevicesForType(DeviceType type) {
  if (devices_by_type_.count(type) == 0)
    EnumerateDevicesForType(type);
  return devices_by_type_[type];
}

}  //  namespace mcil
