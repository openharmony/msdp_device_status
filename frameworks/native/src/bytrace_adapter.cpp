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

#include "bytrace_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
const std::string clientSubscribe { "ClientSubscribe" };
const std::string clientUnsubscribe { "ClientUnsubscribe" };
const std::string serviceSubscribe { "ServiceSubscribe" };
const std::string serviceUnsubscribe { "ServiceUnsubscribe" };
} // namespace
void BytraceAdapter::StartBytrace(TraceBtn traceBtn, SubscribeType isSubscribe, SubscribeObject subscribeObject)
{
    if (isSubscribe) {
        if (traceBtn == TRACE_START) {
            if (subscribeObject == CLIENT) {
                StartTrace(HITRACE_TAG_MSDP, clientSubscribe);
                HITRACE_METER_NAME(HITRACE_TAG_MSDP, "client start subsvribe");
            } else {
                StartTrace(HITRACE_TAG_MSDP, serviceSubscribe);
                HITRACE_METER_NAME(HITRACE_TAG_MSDP, "service start subsvribe");
            }
        } else {
            FinishTrace(HITRACE_TAG_MSDP);
        }
    } else {
        if (traceBtn == TRACE_START) {
            if (subscribeObject == CLIENT) {
                StartTrace(HITRACE_TAG_MSDP, clientUnsubscribe);
                HITRACE_METER_NAME(HITRACE_TAG_MSDP, "client start unSubsvribe");
            } else {
                StartTrace(HITRACE_TAG_MSDP, serviceUnsubscribe);
                HITRACE_METER_NAME(HITRACE_TAG_MSDP, "service start unSubsvribe");
            }
        } else {
            FinishTrace(HITRACE_TAG_MSDP);
        }
    }
}
} // namespace DeviceStatus
} // namespace MSDP
} // namespace OHOS
