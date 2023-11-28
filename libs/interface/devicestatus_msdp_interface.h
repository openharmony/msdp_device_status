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

#ifndef DEVICESTATUS_MSDP_INTERFACE_H
#define DEVICESTATUS_MSDP_INTERFACE_H

#include <memory>

#include <errors.h>

#include "stationary_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IMsdp {
public:
    IMsdp() = default;
    virtual ~IMsdp() = default;
    class MsdpAlgoCallback {
    public:
        MsdpAlgoCallback() = default;
        virtual ~MsdpAlgoCallback() = default;
        virtual void OnResult(const Data &data) = 0;
    };

    virtual ErrCode RegisterCallback(std::shared_ptr<IMsdp::MsdpAlgoCallback> callback) = 0;
    virtual ErrCode UnregisterCallback() = 0;
    virtual ErrCode Enable(Type type) = 0;
    virtual ErrCode Disable(Type type) = 0;
    virtual ErrCode DisableCount(Type type) = 0;
};

struct MsdpAlgoHandle {
    void* handle;
    IMsdp* (*create)();
    void* (*destroy)(IMsdp*);
    IMsdp* pAlgorithm;
    MsdpAlgoHandle() : handle(nullptr), create(nullptr), destroy(nullptr), pAlgorithm(nullptr) {}
    ~MsdpAlgoHandle() {}
    void Clear()
    {
        handle = nullptr;
        create = nullptr;
        destroy = nullptr;
        pAlgorithm = nullptr;
    }
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_MSDP_INTERFACE_H
