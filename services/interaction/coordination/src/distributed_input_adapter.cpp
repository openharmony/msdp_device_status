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

#include "distributed_input_adapter.h"

#include <algorithm>
#include <map>
#include <mutex>

#include "coordination_event_manager.h"
#include "coordination_hisysevent.h"
#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "DistributedInputAdapter"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace DistributedHardware::DistributedInput;
namespace {
constexpr int32_t RETRY_TIME { 2 };
constexpr int32_t DEFAULT_DELAY_TIME { 4000 };
} // namespace
DistributedInputAdapter::DistributedInputAdapter() = default;

DistributedInputAdapter::~DistributedInputAdapter()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(adapterLock_);
    callbacks_.clear();
}

bool DistributedInputAdapter::IsNeedFilterOut(const std::string &networkId, const BusinessEvent &event)
{
    CALL_INFO_TRACE;
    return DistributedInputKit::IsNeedFilterOut(networkId, event);
}

int32_t DistributedInputAdapter::StartRemoteInput(const std::string &remoteNetworkId,
    const std::string &originNetworkId, const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    CALL_INFO_TRACE;
    sptr<IStartStopDInputsCallback> cb = new (std::nothrow) StartDInputCallbackSink();
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::StartDInputCallbackSink, callback);
    int32_t ret = DistributedInputKit::StartRemoteInput(remoteNetworkId, originNetworkId, inputDeviceDhids, cb);
    if (ret != RET_OK) {
        CoordinationDFX::WriteInputFunc(CoorType::INPUT_START_REMOTE_INPUT, "remoteNetworkId",
            remoteNetworkId.substr(0, SUB_LEN), "originNetworkId", originNetworkId.substr(0, SUB_LEN));
    }
    return ret;
}

int32_t DistributedInputAdapter::StopRemoteInput(const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    CALL_INFO_TRACE;
    sptr<IStartStopDInputsCallback> cb = new (std::nothrow) StopDInputCallbackDHIds();
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::StopDInputCallbackDHIds, callback);
    int32_t ret = DistributedInputKit::StopRemoteInput(originNetworkId, inputDeviceDhids, cb);
    if (ret != RET_OK) {
        CoordinationDFX::WriteInputFunc(CoorType::INPUT_STOP_REMOTE_T, "originNetworkId",
            originNetworkId.substr(0, SUB_LEN));
    }
    return ret;
}

int32_t DistributedInputAdapter::StopRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    CALL_INFO_TRACE;
    sptr<IStartStopDInputsCallback> cb = new (std::nothrow) StopDInputCallbackSink();
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::StopDInputCallbackSink, callback);
    int32_t ret = DistributedInputKit::StopRemoteInput(remoteNetworkId, originNetworkId, inputDeviceDhids, cb);
    if (ret != RET_OK) {
        CoordinationDFX::WriteInputFunc(CoorType::INPUT_STOP_REMOTE_O, "remoteNetworkId",
            remoteNetworkId.substr(0, SUB_LEN), "originNetworkId", originNetworkId.substr(0, SUB_LEN));
    }
    return ret;
}

int32_t DistributedInputAdapter::PrepareRemoteInput(const std::string &remoteNetworkId,
                                                    const std::string &originNetworkId, DInputCallback callback)
{
    CALL_INFO_TRACE;
    sptr<IPrepareDInputCallback> cb = new (std::nothrow) PrepareStartDInputCallbackSink();
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::PrepareStartDInputCallbackSink, callback);
    int32_t ret = DistributedInputKit::PrepareRemoteInput(remoteNetworkId, originNetworkId, cb);
    if (ret != RET_OK) {
        CoordinationDFX::WriteInputFunc(CoorType::INPUT_PRE_REMOTE_T, "remoteNetworkId",
            remoteNetworkId.substr(0, SUB_LEN), "originNetworkId", originNetworkId.substr(0, SUB_LEN));
    }
    return ret;
}

int32_t DistributedInputAdapter::UnPrepareRemoteInput(const std::string &remoteNetworkId,
                                                      const std::string &originNetworkId, DInputCallback callback)
{
    CALL_INFO_TRACE;
    sptr<IUnprepareDInputCallback> cb = new (std::nothrow) UnPrepareStopDInputCallbackSink();
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::UnPrepareStopDInputCallbackSink, callback);
    int32_t ret = DistributedInputKit::UnprepareRemoteInput(remoteNetworkId, originNetworkId, cb);
    if (ret != RET_OK) {
        CoordinationDFX::WriteInputFunc(CoorType::INPUT_UNPRE_REMOTE_T, "remoteNetworkId",
            remoteNetworkId.substr(0, SUB_LEN), "originNetworkId", originNetworkId.substr(0, SUB_LEN));
    }
    return ret;
}

int32_t DistributedInputAdapter::PrepareRemoteInput(const std::string &networkId, DInputCallback callback)
{
    CALL_INFO_TRACE;
    sptr<IPrepareDInputCallback> cb = new (std::nothrow) PrepareStartDInputCallback();
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::PrepareStartDInputCallback, callback);
    int32_t ret = DistributedInputKit::PrepareRemoteInput(networkId, cb);
    if (ret != RET_OK) {
        CoordinationDFX::WriteInputFunc(CoorType::INPUT_PRE_REMOTE_O, "remoteNetworkId", networkId.substr(0, SUB_LEN));
    }
    return ret;
}

int32_t DistributedInputAdapter::UnPrepareRemoteInput(const std::string &networkId, DInputCallback callback)
{
    CALL_INFO_TRACE;
    sptr<IUnprepareDInputCallback> cb = new (std::nothrow) UnPrepareStopDInputCallback();
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::UnPrepareStopDInputCallback, callback);
    int32_t ret = DistributedInputKit::UnprepareRemoteInput(networkId, cb);
    if (ret != RET_OK) {
        CoordinationDFX::WriteInputFunc(CoorType::INPUT_UNPRE_REMOTE_O, "remoteNetworkId",
            networkId.substr(0, SUB_LEN));
    }
    return ret;
}

int32_t DistributedInputAdapter::RegisterSessionStateCb(std::function<void(uint32_t)> callback)
{
    CALL_INFO_TRACE;
    sptr<SessionStateCallback> callBack = new (std::nothrow) SessionStateCallback(callback);
    CHKPR(callback, ERROR_NULL_POINTER);
    return DistributedInputKit::RegisterSessionStateCb(callBack);
}

int32_t DistributedInputAdapter::UnregisterSessionStateCb()
{
    CALL_INFO_TRACE;
    return DistributedInputKit::UnregisterSessionStateCb();
}

void DistributedInputAdapter::SaveCallback(CallbackType cbType, DInputCallback callback)
{
    std::lock_guard<std::mutex> guard(adapterLock_);
    CHKPV(callback);
    callbacks_[cbType] = callback;
    AddTimer(cbType);
}

void DistributedInputAdapter::AddTimer(const CallbackType &type)
{
    FI_HILOGD("AddTimer callback type:%{public}d", type);
    auto context = COOR_EVENT_MGR->GetIContext();
    CHKPV(context);
    int32_t timerId = context->GetTimerManager().AddTimer(DEFAULT_DELAY_TIME, RETRY_TIME, [this, type]() {
        if ((callbacks_.find(type) == callbacks_.end()) || (watchings_.find(type) == watchings_.end())) {
            FI_HILOGE("Callback or watchings is not exist");
            return;
        }
        if (watchings_[type].times == 0) {
            FI_HILOGI("The callback will be retried next time");
            watchings_[type].times++;
            return;
        }
        callbacks_[type](false);
        callbacks_.erase(type);
    });
    if (timerId < 0) {
        FI_HILOGE("Failed to add timer, timeId:%{public}d", timerId);
        return;
    }
    watchings_[type].timerId = timerId;
    watchings_[type].times = 0;
}

void DistributedInputAdapter::RemoveTimer(const CallbackType &type)
{
    FI_HILOGD("Remove timer callback type:%{public}d", type);
    if (watchings_.find(type) != watchings_.end()) {
        auto context = COOR_EVENT_MGR->GetIContext();
        CHKPV(context);
        context->GetTimerManager().RemoveTimer(watchings_[type].timerId);
        watchings_.erase(type);
    }
}

void DistributedInputAdapter::ProcessDInputCallback(CallbackType cbType, int32_t status)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(adapterLock_);
    RemoveTimer(cbType);
    auto it = callbacks_.find(cbType);
    if (it == callbacks_.end()) {
        FI_HILOGI("Dinput callback not exist");
        return;
    }
    it->second(status == RET_OK);
    callbacks_.erase(it);
}

void DistributedInputAdapter::StartDInputCallback::OnResult(const std::string &devID, const uint32_t &inputTypes,
                                                            const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::StartDInputCallback, status);
}

void DistributedInputAdapter::StopDInputCallback::OnResult(const std::string &devID, const uint32_t &inputTypes,
                                                           const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::StopDInputCallback, status);
}

void DistributedInputAdapter::StartDInputCallbackDHIds::OnResultDhids(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::StartDInputCallbackDHIds, status);
}

void DistributedInputAdapter::StopDInputCallbackDHIds::OnResultDhids(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::StopDInputCallbackDHIds, status);
}

void DistributedInputAdapter::StartDInputCallbackSink::OnResultDhids(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::StartDInputCallbackSink, status);
}

void DistributedInputAdapter::StopDInputCallbackSink::OnResultDhids(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::StopDInputCallbackSink, status);
}

void DistributedInputAdapter::PrepareStartDInputCallback::OnResult(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::PrepareStartDInputCallback, status);
}

void DistributedInputAdapter::UnPrepareStopDInputCallback::OnResult(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::UnPrepareStopDInputCallback, status);
}

void DistributedInputAdapter::PrepareStartDInputCallbackSink::OnResult(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::PrepareStartDInputCallbackSink, status);
}

void DistributedInputAdapter::UnPrepareStopDInputCallbackSink::OnResult(const std::string &devID, const int32_t &status)
{
    D_INPUT_ADAPTER->ProcessDInputCallback(CallbackType::UnPrepareStopDInputCallbackSink, status);
}

void DistributedInputAdapter::SessionStateCallback::OnResult(const std::string &devID, const uint32_t status)
{
    CHKPV(callback_);
    callback_(status);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
