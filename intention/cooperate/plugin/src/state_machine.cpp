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

#include "state_machine.h"

#include "application_state_observer_stub.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "cooperate_events.h"
#include "cooperate_free.h"
#include "cooperate_in.h"
#include "cooperate_out.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "StateMachine"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

StateMachine::AppStateObserver::AppStateObserver(Channel<CooperateEvent>::Sender sender, int32_t clientPid)
    : sender_(sender), clientPid_(clientPid) {}

void StateMachine::AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    FI_HILOGI("\'%{public}s\' died, pid:%{public}d", processData.bundleName.c_str(), processData.pid);
    if (processData.pid == clientPid_) {
        sender_.Send(CooperateEvent(CooperateEventType::APP_CLOSED));
        FI_HILOGI("Report to handler");
    }
}

void StateMachine::AppStateObserver::UpdateClientPid(int32_t clientPid)
{
    clientPid_ = clientPid;
}

StateMachine::StateMachine(IContext *env)
    : env_(env)
{
    states_[COOPERATE_STATE_FREE] = std::make_shared<CooperateFree>(*this, env);
    states_[COOPERATE_STATE_OUT] = std::make_shared<CooperateOut>(*this, env);
    states_[COOPERATE_STATE_IN] = std::make_shared<CooperateIn>(*this, env);

    AddHandler(CooperateEventType::ADD_OBSERVER, &StateMachine::AddObserver);
    AddHandler(CooperateEventType::REMOVE_OBSERVER, &StateMachine::RemoveObserver);
    AddHandler(CooperateEventType::REGISTER_LISTENER, &StateMachine::RegisterListener);
    AddHandler(CooperateEventType::UNREGISTER_LISTENER, &StateMachine::UnregisterListener);
    AddHandler(CooperateEventType::REGISTER_HOTAREA_LISTENER, &StateMachine::RegisterHotAreaListener);
    AddHandler(CooperateEventType::UNREGISTER_HOTAREA_LISTENER, &StateMachine::UnregisterHotAreaListener);
    AddHandler(CooperateEventType::ENABLE, &StateMachine::EnableCooperate);
    AddHandler(CooperateEventType::DISABLE, &StateMachine::DisableCooperate);
    AddHandler(CooperateEventType::START, &StateMachine::StartCooperate);
    AddHandler(CooperateEventType::GET_COOPERATE_STATE, &StateMachine::GetCooperateState);
    AddHandler(CooperateEventType::REGISTER_EVENT_LISTENER, &StateMachine::RegisterEventListener);
    AddHandler(CooperateEventType::UNREGISTER_EVENT_LISTENER, &StateMachine::UnregisterEventListener);
    AddHandler(CooperateEventType::DDM_BOARD_ONLINE, &StateMachine::OnBoardOnline);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &StateMachine::OnBoardOffline);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED, &StateMachine::OnProfileChanged);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &StateMachine::OnPointerEvent);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &StateMachine::OnSoftbusSessionClosed);
    AddHandler(CooperateEventType::DSOFTBUS_SUBSCRIBE_MOUSE_LOCATION, &StateMachine::OnSoftbusSubscribeMouseLocation);
    AddHandler(CooperateEventType::DSOFTBUS_UNSUBSCRIBE_MOUSE_LOCATION,
        &StateMachine::OnSoftbusUnSubscribeMouseLocation);
    AddHandler(CooperateEventType::DSOFTBUS_REPLY_SUBSCRIBE_MOUSE_LOCATION,
        &StateMachine::OnSoftbusReplySubscribeMouseLocation);
    AddHandler(CooperateEventType::DSOFTBUS_REPLY_UNSUBSCRIBE_MOUSE_LOCATION,
        &StateMachine::OnSoftbusReplyUnSubscribeMouseLocation);
    AddHandler(CooperateEventType::DSOFTBUS_MOUSE_LOCATION, &StateMachine::OnSoftbusMouseLocation);
}

void StateMachine::OnEvent(Context &context, const CooperateEvent &event)
{
    if (auto iter = handlers_.find(event.type); iter != handlers_.end()) {
        iter->second(context, event);
    } else {
        Transfer(context, event);
    }
}

void StateMachine::TransiteTo(Context &context, CooperateState state)
{
    if ((state >= COOPERATE_STATE_FREE) &&
        (state < N_COOPERATE_STATES) &&
        (state != current_)) {
        states_[current_]->OnLeaveState(context);
        current_ = state;
        states_[current_]->OnEnterState(context);
    }
}

void StateMachine::AddHandler(CooperateEventType event,
    void (StateMachine::*handler)(Context&, const CooperateEvent&))
{
    handlers_.emplace(event, std::bind(handler, this, std::placeholders::_1, std::placeholders::_2));
}

void StateMachine::OnQuit(Context &context)
{
    CALL_DEBUG_ENTER;
    RemoveWatches(context);
    RemoveMonitor(context);
}

void StateMachine::AddObserver(Context &context, const CooperateEvent &event)
{
    AddObserverEvent notice = std::get<AddObserverEvent>(event.event);
    context.AddObserver(notice.observer);
}

void StateMachine::RemoveObserver(Context &context, const CooperateEvent &event)
{
    RemoveObserverEvent notice = std::get<RemoveObserverEvent>(event.event);
    context.RemoveObserver(notice.observer);
}

void StateMachine::RegisterListener(Context &context, const CooperateEvent &event)
{
    RegisterListenerEvent notice = std::get<RegisterListenerEvent>(event.event);
    context.eventMgr_.RegisterListener(notice);
}

void StateMachine::UnregisterListener(Context &context, const CooperateEvent &event)
{
    UnregisterListenerEvent notice = std::get<UnregisterListenerEvent>(event.event);
    context.eventMgr_.UnregisterListener(notice);
}

void StateMachine::RegisterHotAreaListener(Context &context, const CooperateEvent &event)
{
    RegisterHotareaListenerEvent notice = std::get<RegisterHotareaListenerEvent>(event.event);
    context.hotArea_.AddListener(notice);
}

void StateMachine::UnregisterHotAreaListener(Context &context, const CooperateEvent &event)
{
    UnregisterHotareaListenerEvent notice = std::get<UnregisterHotareaListenerEvent>(event.event);
    context.hotArea_.RemoveListener(notice);
}

void StateMachine::EnableCooperate(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    EnableCooperateEvent enableEvent = std::get<EnableCooperateEvent>(event.event);
    context.EnableCooperate(enableEvent);
    context.eventMgr_.EnableCooperate(enableEvent);
    context.hotArea_.EnableCooperate(enableEvent);
    AddSessionObserver(context, enableEvent);
    AddMonitor(context);
    Transfer(context, event);
}

void StateMachine::DisableCooperate(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DisableCooperateEvent disableEvent = std::get<DisableCooperateEvent>(event.event);
    context.DisableCooperate(disableEvent);
    context.eventMgr_.DisableCooperate(disableEvent);
    RemoveSessionObserver(context, disableEvent);
    RemoveMonitor(context);
    Transfer(context, event);
}

void StateMachine::StartCooperate(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StartCooperateEvent startEvent = std::get<StartCooperateEvent>(event.event);
    UpdateApplicationStateObserver(startEvent.pid);
    if (!context.IsAllowCooperate()) {
        FI_HILOGI("Not allow cooperate");
        startEvent.errCode->set_value(COMMON_NOT_ALLOWED_DISTRIBUTED);
        return;
    }
    startEvent.errCode->set_value(RET_OK);
    Transfer(context, event);
}

void StateMachine::GetCooperateState(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    GetCooperateStateEvent stateEvent = std::get<GetCooperateStateEvent>(event.event);
    UpdateApplicationStateObserver(stateEvent.pid);
    bool switchStatus { false };
    auto udId = env_->GetDP().GetUdIdByNetworkId(stateEvent.networkId);
    if (env_->GetDP().GetCrossingSwitchState(udId, switchStatus) != RET_OK) {
        FI_HILOGE("GetCrossingSwitchState for udId:%{public}s failed", Utility::Anonymize(udId).c_str());
        return;
    }
    auto session = env_->GetSocketSessionManager().FindSessionByPid(stateEvent.pid);
    CHKPV(session);
    NetPacket pkt(MessageId::COORDINATION_GET_STATE);
    pkt << stateEvent.userData << switchStatus;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!session->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
    }
}

void StateMachine::RegisterEventListener(Context &context, const CooperateEvent &event)
{
    RegisterEventListenerEvent notice = std::get<RegisterEventListenerEvent>(event.event);
    context.mouseLocation_.AddListener(notice);
}

void StateMachine::UnregisterEventListener(Context &context, const CooperateEvent &event)
{
    UnregisterEventListenerEvent notice = std::get<UnregisterEventListenerEvent>(event.event);
    context.mouseLocation_.RemoveListener(notice);
}

void StateMachine::OnBoardOnline(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DDMBoardOnlineEvent onlineEvent = std::get<DDMBoardOnlineEvent>(event.event);

    auto ret = onlineBoards_.insert(onlineEvent.networkId);
    if (ret.second) {
        FI_HILOGD("Watch \'%{public}s\'", Utility::Anonymize(onlineEvent.networkId).c_str());
        env_->GetDP().AddWatch(onlineEvent.networkId);
        Transfer(context, event);
    }
}

void StateMachine::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DDMBoardOfflineEvent offlineEvent = std::get<DDMBoardOfflineEvent>(event.event);

    if (auto iter = onlineBoards_.find(offlineEvent.networkId); iter != onlineBoards_.end()) {
        onlineBoards_.erase(iter);
        FI_HILOGD("Remove watch \'%{public}s\'", Utility::Anonymize(offlineEvent.networkId).c_str());
        env_->GetDP().RemoveWatch(offlineEvent.networkId);
        context.CloseDistributedFileConnection(std::string());
        Transfer(context, event);
    }
}

void StateMachine::OnProfileChanged(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);
    context.eventMgr_.OnProfileChanged(notice);
    Transfer(context, event);
}

void StateMachine::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    InputPointerEvent pointerEvent = std::get<InputPointerEvent>(event.event);
    Coordinate cursorPos = context.CursorPosition();
    context.OnPointerEvent(pointerEvent);
    pointerEvent.position = cursorPos;
    Transfer(context, CooperateEvent { CooperateEventType::INPUT_POINTER_EVENT, pointerEvent });
}

void StateMachine::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);
    context.eventMgr_.OnSoftbusSessionClosed(notice);
    Transfer(context, event);
}

void StateMachine::OnSoftbusSubscribeMouseLocation(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusSubscribeMouseLocation notice = std::get<DSoftbusSubscribeMouseLocation>(event.event);
    context.mouseLocation_.OnSubscribeMouseLocation(notice);
}

void StateMachine::OnSoftbusUnSubscribeMouseLocation(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusUnSubscribeMouseLocation notice = std::get<DSoftbusUnSubscribeMouseLocation>(event.event);
    context.mouseLocation_.OnUnSubscribeMouseLocation(notice);
}

void StateMachine::OnSoftbusReplySubscribeMouseLocation(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusReplySubscribeMouseLocation notice = std::get<DSoftbusReplySubscribeMouseLocation>(event.event);
    context.mouseLocation_.OnReplySubscribeMouseLocation(notice);
}

void StateMachine::OnSoftbusReplyUnSubscribeMouseLocation(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusReplyUnSubscribeMouseLocation notice = std::get<DSoftbusReplyUnSubscribeMouseLocation>(event.event);
    context.mouseLocation_.OnReplyUnSubscribeMouseLocation(notice);
}

void StateMachine::OnSoftbusMouseLocation(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DSoftbusSyncMouseLocation notice = std::get<DSoftbusSyncMouseLocation>(event.event);
    context.mouseLocation_.OnRemoteMouseLocation(notice);
}

void StateMachine::Transfer(Context &context, const CooperateEvent &event)
{
    states_[current_]->OnEvent(context, event);
}

sptr<AppExecFwk::IAppMgr> StateMachine::GetAppMgr()
{
    CALL_INFO_TRACE;
    auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHKPP(saMgr);
    auto appMgrObj = saMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
    CHKPP(appMgrObj);
    return iface_cast<AppExecFwk::IAppMgr>(appMgrObj);
}

int32_t StateMachine::RegisterApplicationStateObserver(Channel<CooperateEvent>::Sender sender,
    const EnableCooperateEvent &event)
{
    CALL_INFO_TRACE;
    auto bundleName = GetPackageName(event.tokenId);
    clientBundleNames_.push_back(bundleName);
    FI_HILOGI("Register application %{public}s state observer", bundleName.c_str());
    auto appMgr = GetAppMgr();
    CHKPR(appMgr, RET_ERR);
    appStateObserver_ = sptr<AppStateObserver>::MakeSptr(sender, event.pid);
    auto err = appMgr->RegisterApplicationStateObserver(appStateObserver_, clientBundleNames_);
    if (err != RET_OK) {
        appStateObserver_.clear();
        FI_HILOGE("IAppMgr::RegisterApplicationStateObserver fail, error:%{public}d", err);
        return RET_ERR;
    }
    return RET_OK;
}

void StateMachine::UnregisterApplicationStateObserver()
{
    CALL_INFO_TRACE;
    CHKPV(appStateObserver_);
    auto appMgr = GetAppMgr();
    CHKPV(appMgr);
    FI_HILOGI("Unregister application associateassistant state observer");
    auto err = appMgr->UnregisterApplicationStateObserver(appStateObserver_);
    if (err != RET_OK) {
        FI_HILOGE("IAppMgr::UnregisterApplicationStateObserver fail, error:%{public}d", err);
    }
    appStateObserver_.clear();
}

void StateMachine::UpdateApplicationStateObserver(int32_t clientPid)
{
    CALL_INFO_TRACE;
    CHKPV(appStateObserver_);
    appStateObserver_->UpdateClientPid(clientPid);
}

void StateMachine::AddSessionObserver(Context &context, const EnableCooperateEvent &event)
{
    CALL_INFO_TRACE;
    RegisterApplicationStateObserver(context.Sender(), event);
}

std::string StateMachine::GetPackageName(Security::AccessToken::AccessTokenID tokenId)
{
    CALL_INFO_TRACE;
    std::string bundleName {"Default"};
    int32_t tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (tokenType) {
        case Security::AccessToken::ATokenTypeEnum::TOKEN_HAP: {
            Security::AccessToken::HapTokenInfo hapInfo;
            if (Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != RET_OK) {
                FI_HILOGE("Get hap token info failed");
            } else {
                bundleName = hapInfo.bundleName;
            }
            break;
        }
        case Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE:
        case Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL: {
            Security::AccessToken::NativeTokenInfo tokenInfo;
            if (Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != RET_OK) {
                FI_HILOGE("Get native token info failed");
            } else {
                bundleName = tokenInfo.processName;
            }
            break;
        }
        default: {
            FI_HILOGW("token type not match");
            break;
        }
    }
    return bundleName;
}

void StateMachine::RemoveSessionObserver(Context &context, const DisableCooperateEvent &event)
{
    UnregisterApplicationStateObserver();
}

void StateMachine::AddMonitor(Context &context)
{
    CALL_INFO_TRACE;
    if (monitorId_ >= 0) {
        return;
    }
    CHKPV(env_);
    monitorId_ = env_->GetInput().AddMonitor(
        [sender = context.Sender(), &hotArea = context.hotArea_, &mouseLocation = context.mouseLocation_] (
            std::shared_ptr<MMI::PointerEvent> pointerEvent) mutable {
            hotArea.ProcessData(pointerEvent);
            mouseLocation.ProcessData(pointerEvent);

            MMI::PointerEvent::PointerItem pointerItem;
            if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
                FI_HILOGE("Corrupted pointer event");
                return;
            }
            sender.Send(CooperateEvent(
                CooperateEventType::INPUT_POINTER_EVENT,
                InputPointerEvent {
                    .deviceId = pointerEvent->GetDeviceId(),
                    .pointerAction = pointerEvent->GetPointerAction(),
                    .sourceType = pointerEvent->GetSourceType(),
                    .position = Coordinate {
                        .x = pointerItem.GetDisplayX(),
                        .y = pointerItem.GetDisplayY(),
                    }
                }));
        });
    if (monitorId_ < 0) {
        FI_HILOGE("MMI::Add Monitor fail");
    }
}

void StateMachine::RemoveMonitor(Context &context)
{
    CALL_INFO_TRACE;
    if (monitorId_ < 0) {
        return;
    }
    env_->GetInput().RemoveMonitor(monitorId_);
    monitorId_ = -1;
}

void StateMachine::RemoveWatches(Context &context)
{
    CALL_INFO_TRACE;
    for (auto iter = onlineBoards_.begin();
         iter != onlineBoards_.end(); iter = onlineBoards_.begin()) {
        FI_HILOGD("Remove watch \'%{public}s\'", Utility::Anonymize(*iter).c_str());
        env_->GetDP().RemoveWatch(*iter);
        onlineBoards_.erase(iter);
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
