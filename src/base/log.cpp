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

#include "log.h"

PmLogContext GetPmLogContext() {
  static PmLogContext mcil_log_context = 0;

  if (0 == mcil_log_context) {
    PmLogGetContext("mcil", &mcil_log_context);
  }

  return mcil_log_context;
}

std::string BaseFile(const char* file_path) {
  std::string filename = std::string(file_path);
  size_t last_slash_pos = filename.find_last_of("/");
  return filename.substr(last_slash_pos + 1);
}
