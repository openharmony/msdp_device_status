/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "drag_vsync_station.h"

#include <cstdint>

#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "include/util.h"
#include "transaction/rs_interfaces.h"

#undef LOG_TAG
#define LOG_TAG "DragVSyncStation"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

int32_t DragVSyncStation::RequestFrame(int32_t frameType, std::shared_ptr<DragFrameCallback> callback,
        std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    CHKPR(callback, RET_ERR);
    if (frameType < TYPE_FLUSH_DRAG_POSITION || frameType >= REQUEST_TYPE_MAX) {
        FI_HILOGE("Frame callback type is invalid, %{public}d", frameType);
        return RET_ERR;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    int32_t ret = Init(handler);
    if (ret != RET_OK) {
        FI_HILOGE("Init receiver failed");
        return RET_ERR;
    }
    ret = receiver_->RequestNextVSync(frameCallback_);
    if (ret != RET_OK) {
        FI_HILOGE("Request next vSync failed");
        return RET_ERR;
    }
    vSyncCallbacks_.emplace(frameType, callback);
    return RET_OK;
}

void DragVSyncStation::StopVSyncRequest()
{
    FI_HILOGD("StopVSyncRequest in");
    std::lock_guard<std::mutex> lock(mtx_);
    vSyncCallbacks_.erase(vSyncCallbacks_.begin(), vSyncCallbacks_.end());
    if (receiver_ != nullptr) {
        receiver_ = nullptr;
    }
    vSyncPeriod_ = 0;
}

uint64_t DragVSyncStation::GetVSyncPeriod()
{
    FI_HILOGD("GetVSyncPeriod in");
    std::lock_guard<std::mutex> lock(mtx_);
    if (vSyncPeriod_ != 0) {
        return vSyncPeriod_;
    }
    int64_t period = 0;
    int32_t ret = receiver_->GetVSyncPeriod(period);
    if (ret != RET_OK) {
        FI_HILOGE("GetVSyncPeriod failed");
    }
    vSyncPeriod_ = (period <= 0 ? 0 : static_cast<uint64_t>(period));
    return vSyncPeriod_;
}

int32_t DragVSyncStation::Init(std::shared_ptr<AppExecFwk::EventHandler> hander)
{
    if (receiver_ != nullptr) {
        return RET_OK;
    }
    CHKPR(hander, RET_ERR);
    receiver_ = Rosen::RSInterfaces::GetInstance().CreateVSyncReceiver("DragVSyncStation", hander);
    CHKPR(receiver_, RET_ERR);
    int32_t ret = receiver_->Init();
    if (ret != RET_OK) {
        FI_HILOGE("Init receiver, %{public}d", ret);
        return RET_ERR;
    }
    frameCallback_ = {
        .userData_ = this,
        .callback_ = OnVSync,
    };
    FI_HILOGI("Init receiver success");
    return RET_OK;
}

void DragVSyncStation::OnVSyncInner(uint64_t nanoTimestamp)
{
    FI_HILOGD("OnVSyncInner in");
    std::map<int32_t, std::shared_ptr<DragFrameCallback>> vSyncCallbacks;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        vSyncCallbacks.swap(vSyncCallbacks_);
    }
    for (auto &callback : vSyncCallbacks) {
        if (callback.second != nullptr) {
            (*callback.second)(nanoTimestamp);
        }
    }
}

void DragVSyncStation::OnVSync(uint64_t nanoTimestamp, void *client)
{
    auto vSyncClient = static_cast<DragVSyncStation *>(client);
    if (vSyncClient != nullptr) {
        vSyncClient->OnVSyncInner(nanoTimestamp);
    } else {
        FI_HILOGE("VSync client is null");
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS