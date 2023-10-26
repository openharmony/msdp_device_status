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

#ifndef BYTRACE_ADAPTER_H
#define BYTRACE_ADAPTER_H

#include <cstring>

#include "hitrace_meter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class BytraceAdapter {
public:
    enum TraceBtn {
        TRACE_STOP = 0,
        TRACE_START = 1
    };
    enum SubscribeType {
        SUBSCRIBE,
        UNSUBSCRIBE
    };
    enum SubscribeObject {
        CLIENT,
        SERVICE
    };

    static void StartBytrace(TraceBtn traceBtn, SubscribeType subscribeType, SubscribeObject subscribeObject);
};
} // namespace DeviceStatus
} // namespace MSDP
} // namespace OHOS
#endif // BYTRACE_ADAPTER_H
