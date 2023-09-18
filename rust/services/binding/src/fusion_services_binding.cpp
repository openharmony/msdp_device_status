/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "fusion_services_binding.h"

#include "devicestatus_define.h"
#include "fi_log.h"

using namespace ::OHOS::Msdp::DeviceStatus;

namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, ::OHOS::Msdp::MSDP_DOMAIN_ID, "fusion_services_binding" };
} // namespace

struct NativeService {
    int32_t refCnt { 0 };
};

struct NativeService* NativeService_New()
{
    CALL_DEBUG_ENTER;
    struct NativeService *service = new NativeService;
    service->refCnt = 1;
    return service;
}

struct NativeService* NativeService_Ref(struct NativeService *service)
{
    CHKPP(service);
    service->refCnt++;
    return service;
}

struct NativeService* NativeService_Unref(struct NativeService *service)
{
    CHKPP(service);
    if (service->refCnt > 0) {
        service->refCnt--;
    }
    if (service->refCnt > 0) {
        return service;
    }
    delete service;
    FI_HILOGI("Device status service instance is deleted");
    return nullptr;
}

void NativeService_OnDump(struct NativeService *service)
{
    CALL_INFO_TRACE;
    CHKPV(service);
}

void NativeService_OnStart(struct NativeService *service)
{
    CALL_INFO_TRACE;
    CHKPV(service);
}

void NativeService_OnStop(struct NativeService *service)
{
    CALL_INFO_TRACE;
    CHKPV(service);
}

int32_t NativeService_AllocSocketFd(struct NativeService *service, const char *programName,
    int32_t moduleType, int32_t *toReturnClientFd, int32_t *tokenType)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}
