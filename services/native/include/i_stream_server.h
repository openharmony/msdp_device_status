/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef I_STREAM_SERVER_H
#define I_STREAM_SERVER_H

#include "refbase.h"
#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IStreamServer : public virtual RefBase {
public:
    virtual int32_t AddSocketPairInfo(const std::string &programName, int32_t moduleType, int32_t uid,
                                      int32_t pid, int32_t &serverFd, int32_t &toReturnClientFd,
                                      int32_t &tokenType) = 0;
    virtual SessionPtr GetSessionByPid(int32_t pid) const = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_STREAM_SERVER_H