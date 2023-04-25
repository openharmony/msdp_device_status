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

#include "across_device_drag.h"

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "coordination_softbus_adapter.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION
#include "devicestatus_define.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "AcrossDeviceDrag" };
} // namespace

AcrossDeviceDrag::AcrossDeviceDrag()
{
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    CooSoftbusAdapter->RegisterRecvFunc(CoordinationSoftbusAdapter::DRAGING_DATA,
        std::bind(&AcrossDeviceDrag::RecvDragingData, this, std::placeholders::_1, std::placeholders::_2));
    CooSoftbusAdapter->RegisterRecvFunc(CoordinationSoftbusAdapter::STOPDRAG_DATA,
        std::bind(&AcrossDeviceDrag::RecvStopDragData, this, std::placeholders::_1, std::placeholders::_2));
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t AcrossDeviceDrag::Init(IContext *context)
{
    CALL_DEBUG_ENTER;
    CHKPR(context, RET_ERR);
    context_ = context;
    return RET_OK;
}

void AcrossDeviceDrag::RecvDragingData(void* data, uint32_t dataLen)
{
    CALL_DEBUG_ENTER;
    CHKPV(data);
    if (dataLen == 0) {
        FI_HILOGE("Recv data len is 0");
        return;
    }
}

void AcrossDeviceDrag::RecvStopDragData(void* data, uint32_t dataLen)
{
    CALL_DEBUG_ENTER;
    CHKPV(data);
    if (dataLen == 0) {
        FI_HILOGE("Recv data len is 0");
        return;
    }
}

void AcrossDeviceDrag::SendDragingData()
{
    CALL_DEBUG_ENTER;
}

void AcrossDeviceDrag::SendStopDragData()
{
    CALL_DEBUG_ENTER;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS