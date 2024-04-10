/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dinput_adapter_impl.h"

#include <algorithm>

#include "distributed_input_kit.h"

#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "DInputAdapterImpl"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t DEFAULT_DELAY_TIME { 4000 };
constexpr int32_t RETRY_TIME { 2 };
} // namespace

using namespace DistributedHardware::DistributedInput;

DInputAdapterImpl::DInputAdapterImpl(IContext *env)
    : env_(env)
{}

bool DInputAdapterImpl::IsNeedFilterOut(const std::string &networkId, BusinessEvent &&event)
{
    CALL_DEBUG_ENTER;
    DistributedHardware::DistributedInput::BusinessEvent ev {
        .pressedKeys = std::move(event.pressedKeys),
        .keyCode = event.keyCode,
        .keyAction = event.keyAction,
    };
    return DistributedInputKit::IsNeedFilterOut(networkId, ev);
}

int32_t DInputAdapterImpl::StartRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    CALL_DEBUG_ENTER;
    sptr<IStartStopDInputsCallback> cb = sptr<StartDInputCallbackSink>::MakeSptr(shared_from_this());
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::StartDInputCallbackSink, callback);
    return DistributedInputKit::StartRemoteInput(remoteNetworkId, originNetworkId, inputDeviceDhids, cb);
}

int32_t DInputAdapterImpl::StopRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    CALL_DEBUG_ENTER;
    sptr<IStartStopDInputsCallback> cb = sptr<StopDInputCallbackSink>::MakeSptr(shared_from_this());
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::StopDInputCallbackSink, callback);
    return DistributedInputKit::StopRemoteInput(remoteNetworkId, originNetworkId, inputDeviceDhids, cb);
}

int32_t DInputAdapterImpl::StopRemoteInput(const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    CALL_DEBUG_ENTER;
    sptr<IStartStopDInputsCallback> cb = sptr<StopDInputCallbackDHIds>::MakeSptr(shared_from_this());
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::StopDInputCallbackDHIds, callback);
    return DistributedInputKit::StopRemoteInput(originNetworkId, inputDeviceDhids, cb);
}

int32_t DInputAdapterImpl::PrepareRemoteInput(const std::string &remoteNetworkId,
    const std::string &originNetworkId, DInputCallback callback)
{
    CALL_DEBUG_ENTER;
    sptr<IPrepareDInputCallback> cb = sptr<PrepareStartDInputCallbackSink>::MakeSptr(shared_from_this());
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::PrepareStartDInputCallbackSink, callback);
    return DistributedInputKit::PrepareRemoteInput(remoteNetworkId, originNetworkId, cb);
}

int32_t DInputAdapterImpl::UnPrepareRemoteInput(const std::string &remoteNetworkId,
    const std::string &originNetworkId, DInputCallback callback)
{
    CALL_DEBUG_ENTER;
    sptr<IUnprepareDInputCallback> cb = sptr<UnPrepareStopDInputCallbackSink>::MakeSptr(shared_from_this());
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::UnPrepareStopDInputCallbackSink, callback);
    return DistributedInputKit::UnprepareRemoteInput(remoteNetworkId, originNetworkId, cb);
}

int32_t DInputAdapterImpl::PrepareRemoteInput(const std::string &networkId, DInputCallback callback)
{
    CALL_DEBUG_ENTER;
    sptr<IPrepareDInputCallback> cb = sptr<PrepareDInputCallback>::MakeSptr(shared_from_this());
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::PrepareDInputCallback, callback);
    return DistributedInputKit::PrepareRemoteInput(networkId, cb);
}

int32_t DInputAdapterImpl::UnPrepareRemoteInput(const std::string &networkId, DInputCallback callback)
{
    CALL_DEBUG_ENTER;
    sptr<IUnprepareDInputCallback> cb = sptr<UnprepareDInputCallback>::MakeSptr(shared_from_this());
    CHKPR(cb, ERROR_NULL_POINTER);
    SaveCallback(CallbackType::UnprepareDInputCallback, callback);
    return DistributedInputKit::UnprepareRemoteInput(networkId, cb);
}

int32_t DInputAdapterImpl::RegisterSessionStateCb(std::function<void(uint32_t)> callback)
{
    CALL_DEBUG_ENTER;
    sptr<SessionStateCallback> cb = sptr<SessionStateCallback>::MakeSptr(callback);
    CHKPR(callback, ERROR_NULL_POINTER);
    return DistributedInputKit::RegisterSessionStateCb(cb);
}

int32_t DInputAdapterImpl::UnregisterSessionStateCb()
{
    CALL_DEBUG_ENTER;
    return DistributedInputKit::UnregisterSessionStateCb();
}

void DInputAdapterImpl::SaveCallback(CallbackType type, DInputCallback callback)
{
    std::lock_guard<std::mutex> guard(adapterLock_);
    CHKPV(callback);
    callbacks_[type] = callback;
    AddTimer(type);
}

void DInputAdapterImpl::AddTimer(const CallbackType &type)
{
    FI_HILOGD("AddTimer type:%{public}d", type);
    CHKPV(env_);
    int32_t timerId = env_->GetTimerManager().AddTimer(DEFAULT_DELAY_TIME, RETRY_TIME, [this, type]() {
        if ((callbacks_.find(type) == callbacks_.end()) || (watchings_.find(type) == watchings_.end())) {
            FI_HILOGE("Callback or watching is not exist");
            return;
        }
        if (watchings_[type].times == 0) {
            FI_HILOGI("It will be retry to call callback next time");
            watchings_[type].times++;
            return;
        }
        callbacks_[type](false);
        callbacks_.erase(type);
    });
    if (timerId < 0) {
        FI_HILOGE("Add timer failed, timeId:%{public}d", timerId);
        return;
    }
    watchings_[type].timerId = timerId;
    watchings_[type].times = 0;
}

void DInputAdapterImpl::RemoveTimer(const CallbackType &type)
{
    FI_HILOGD("Remove timer type:%{public}d", type);
    if (watchings_.find(type) != watchings_.end()) {
        CHKPV(env_);
        env_->GetTimerManager().RemoveTimer(watchings_[type].timerId);
        watchings_.erase(type);
    }
}

void DInputAdapterImpl::ProcessDInputCallback(CallbackType type, int32_t status)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(adapterLock_);
    RemoveTimer(type);
    auto it = callbacks_.find(type);
    if (it == callbacks_.end()) {
        FI_HILOGI("Dinput callback not exist");
        return;
    }
    it->second(status == RET_OK);
    callbacks_.erase(it);
}

DInputAdapterImpl::StopDInputCallbackDHIds::StopDInputCallbackDHIds(std::shared_ptr<DInputAdapterImpl> dinput)
    : dinput_(dinput)
{}

void DInputAdapterImpl::StopDInputCallbackDHIds::OnResultDhids(const std::string &devId, const int32_t &status)
{
    std::shared_ptr<DInputAdapterImpl> dinput = dinput_.lock();
    CHKPV(dinput);
    dinput->ProcessDInputCallback(CallbackType::StopDInputCallbackDHIds, status);
}

DInputAdapterImpl::StartDInputCallbackSink::StartDInputCallbackSink(std::shared_ptr<DInputAdapterImpl> dinput)
    : dinput_(dinput)
{}

void DInputAdapterImpl::StartDInputCallbackSink::OnResultDhids(const std::string &devId, const int32_t &status)
{
    std::shared_ptr<DInputAdapterImpl> dinput = dinput_.lock();
    CHKPV(dinput);
    dinput->ProcessDInputCallback(CallbackType::StartDInputCallbackSink, status);
}

DInputAdapterImpl::StopDInputCallbackSink::StopDInputCallbackSink(std::shared_ptr<DInputAdapterImpl> dinput)
    : dinput_(dinput)
{}

void DInputAdapterImpl::StopDInputCallbackSink::OnResultDhids(const std::string &devId, const int32_t &status)
{
    std::shared_ptr<DInputAdapterImpl> dinput = dinput_.lock();
    CHKPV(dinput);
    dinput->ProcessDInputCallback(CallbackType::StopDInputCallbackSink, status);
}

DInputAdapterImpl::PrepareDInputCallback::PrepareDInputCallback(std::shared_ptr<DInputAdapterImpl> dinput)
    : dinput_(dinput)
{}

void DInputAdapterImpl::PrepareDInputCallback::OnResult(const std::string &devId, const int32_t &status)
{
    std::shared_ptr<DInputAdapterImpl> dinput = dinput_.lock();
    CHKPV(dinput);
    dinput->ProcessDInputCallback(CallbackType::PrepareDInputCallback, status);
}

DInputAdapterImpl::UnprepareDInputCallback::UnprepareDInputCallback(std::shared_ptr<DInputAdapterImpl> dinput)
    : dinput_(dinput)
{}

void DInputAdapterImpl::UnprepareDInputCallback::OnResult(const std::string &devId, const int32_t &status)
{
    std::shared_ptr<DInputAdapterImpl> dinput = dinput_.lock();
    CHKPV(dinput);
    dinput->ProcessDInputCallback(CallbackType::UnprepareDInputCallback, status);
}

DInputAdapterImpl::PrepareStartDInputCallbackSink::PrepareStartDInputCallbackSink(
    std::shared_ptr<DInputAdapterImpl> dinput)
    : dinput_(dinput)
{}

void DInputAdapterImpl::PrepareStartDInputCallbackSink::OnResult(const std::string &devId, const int32_t &status)
{
    std::shared_ptr<DInputAdapterImpl> dinput = dinput_.lock();
    CHKPV(dinput);
    dinput->ProcessDInputCallback(CallbackType::PrepareStartDInputCallbackSink, status);
}

DInputAdapterImpl::UnPrepareStopDInputCallbackSink::UnPrepareStopDInputCallbackSink(
    std::shared_ptr<DInputAdapterImpl> dinput)
    : dinput_(dinput)
{}

void DInputAdapterImpl::UnPrepareStopDInputCallbackSink::OnResult(const std::string &devId, const int32_t &status)
{
    std::shared_ptr<DInputAdapterImpl> dinput = dinput_.lock();
    CHKPV(dinput);
    dinput->ProcessDInputCallback(CallbackType::UnPrepareStopDInputCallbackSink, status);
}

DInputAdapterImpl::SessionStateCallback::SessionStateCallback(std::function<void(uint32_t)> callback)
    : callback_(callback) {}

void DInputAdapterImpl::SessionStateCallback::OnResult(const std::string &devId, const uint32_t status)
{
    CHKPV(callback_);
    callback_(status);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
