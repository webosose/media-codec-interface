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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// SPDX-License-Identifier: Apache-2.0

#ifndef SRC_BASE_LOG_H_
#define SRC_BASE_LOG_H_

#include <PmLogLib.h>

#include <cassert>
#include <string>

PmLogContext GetPmLogContext();
std::string BaseFile(const char* file_path);

#define MCIL_LOG_CRITICAL(...) PmLogCritical(GetPmLogContext(), ##__VA_ARGS__)
#define MCIL_LOG_WARNING(...)  PmLogWarning(GetPmLogContext(), ##__VA_ARGS__)

#if 0
#define MCIL_LOG_INFO(FORMAT__, ...) void(0)
#define MCIL_LOG_DEBUG(FORMAT__, ...) void(0)
#define MCIL_LOG_ERROR(FORMAT__, ...) void(0)
#else
#define MCIL_LOG_INFO(FORMAT__, ...) \
    PmLogInfo(GetPmLogContext(), \
    "mcil", 0, "%s:[%d]:%s " FORMAT__, BaseFile(__FILE__).c_str(), __LINE__, __func__,  ##__VA_ARGS__)
#define MCIL_LOG_DEBUG(FORMAT__, ...) \
    PmLogDebug(GetPmLogContext(), \
    "%s:[%d]:%s " FORMAT__, BaseFile(__FILE__).c_str(), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MCIL_LOG_ERROR(FORMAT__, ...) \
    PmLogError(GetPmLogContext(), \
    "mcil", 0, "%s:[%d]:%s " FORMAT__, BaseFile(__FILE__).c_str(), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif

#define MCIL_LOG_OBJ_SET(OBJ__) PmLogContext GetPmLogContext_##OBJ__()
#define MCIL_LOG_OBJ_CRITICAL(OBJ__, ...) \
    PmLogCritical(GetPmLogContext_##OBJ__(), ##__VA_ARGS__)
#define MCIL_LOG_OBJ_ERROR(OBJ__, ...) \
    PmLogError(GetPmLogContext_##OBJ__(), ##__VA_ARGS__)
#define MCIL_LOG_OBJ_WARNING(OBJ__, ...) \
    PmLogWarning(GetPmLogContext_##OBJ__(), ##__VA_ARGS__)
#define MCIL_LOG_OBJ_INFO(OBJ__, ...) \
    PmLogInfo(GetPmLogContext_##OBJ__(), ##__VA_ARGS__)
#define MCIL_LOG_OBJ_DEBUG(OBJ__, FORMAT__, ...) \
    PmLogDebug(GetPmLogContext_##OBJ__(), \
    "%s:[%d]:%s" FORMAT__, BaseFile(__FILE__).c_str(), __LINE__, __FUNCTION__, ##__VA_ARGS__)

/* Info Print */
#define MCIL_INFO_PRINT MCIL_LOG_INFO

/* Debug Print */
#define MCIL_DEBUG_PRINT MCIL_LOG_DEBUG

/* Error Print */
#define MCIL_ERROR_PRINT MCIL_LOG_ERROR

/* Assert print */
#define MCIL_ASSERT(cond) { \
    if (!(cond)) { \
        MCIL_DEBUG_PRINT("%s:[%d]:%s ASSERT FAILED : %s", \
                BaseFile(__FILE__).c_str(), __LINE__, __func__, #cond); \
        assert(0); \
    } \
}

#endif  // SRC_BASE_LOG_H_
