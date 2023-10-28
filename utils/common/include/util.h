/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTIL_H
#define UTIL_H

#include <limits>
#include <string>
#include <vector>

#include <sys/types.h>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
int32_t GetPid();
const char* GetProgramName();
int64_t GetMillisTime();

uint64_t GetThisThreadId();

void SetThreadName(const std::string &name);
void GetTimeStamp(std::string &startTime);

template<typename T>
bool AddInt(T op1, T op2, T minValue, T maxValue, T &res)
{
    if (op1 >= 0) {
        if (op2 > maxValue - op1) {
            return false;
        }
    } else {
        if (op2 < minValue - op1) {
            return false;
        }
    }
    res = op1 + op2;
    return true;
}

inline bool AddInt32(int32_t op1, int32_t op2, int32_t &res)
{
    return AddInt(op1, op2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max(), res);
}

inline bool AddInt64(int64_t op1, int64_t op2, int64_t &res)
{
    return AddInt(op1, op2, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), res);
}

template<typename T>
bool MultiplyInt(T op1, T op2, T minVal, T maxVal, T &res)
{
    if (op1 > 0) {
        if (op2 > 0) {
            if (op1 > maxVal / op2) {
                return false;
            }
        } else {
            if (op2 < minVal / op1) {
                return false;
            }
        }
    } else {
        if (op2 > 0) {
            if (op1 < minVal / op2) {
                return false;
            }
        } else {
            if (op1 != 0 && op2 < maxVal / op1) {
                return false;
            }
        }
    }
    res = op1 * op2;
    return true;
}

inline bool MultiplyInt32(int32_t op1, int32_t op2, int32_t& res)
{
    return MultiplyInt(op1, op2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max(), res);
}

inline bool MultiplyInt64(int64_t op1, int64_t op2, int64_t& res)
{
    return MultiplyInt(op1, op2, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), res);
}

size_t StringSplit(const std::string &str, const std::string &sep, std::vector<std::string> &vecList);
std::string GetAnonyString(const std::string &value);
std::string StringPrintf(const char *format, ...);
bool CheckFileExtendName(const std::string &filePath, const std::string &checkExtension);
bool IsValidPath(const std::string &rootDir, const std::string &filePath);
bool IsValidSvgPath(const std::string &filePath);
bool IsValidSvgFile(const std::string &filePath);
bool IsNum(const std::string &str);
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // UTIL_H
