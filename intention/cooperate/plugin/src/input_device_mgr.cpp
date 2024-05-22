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

#include "input_device_mgr.h"

#include "parcel.h"

#include "devicestatus_define.h"
#include "raw_data_packer.h"
#include "softbus_message_id.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

constexpr int32_t INVALID_DEVICE_ID { -1 };

InputDeviceMgr::InputDeviceMgr(IContext *context) : env_(context) {
    rawDataHandlers_ = {
        { static_cast<int32_t>(SoftbusMessageId::INPUT_DEVICE_INFO),
            &InputDeviceMgr::OnRemoteInputDeviceInfo },
        { static_cast<int32_t>(SoftbusMessageId::INPUT_DEVICE_HOT_PLUG),
            &InputDeviceMgr::OnRemoteHotPlugInfo },
    };
}

void InputDeviceMgr::OnSessionOpened(const DSoftbusSessionOpened &notice)
{
    CALL_INFO_TRACE;
    if (env_->GetDeviceManager().HasKeyboard()) {
        auto keyboards = env_->GetDeviceManager().GetKeyboard();
        NotifyInputDeviceToRemote(notice.networkId, keyboards);
        return;
    }
    FI_HILOGI("No keyboard existed in current device");
}

void InputDeviceMgr::OnSessionClosed(const DSoftbusSessionClosed &notice)
{
    CALL_INFO_TRACE;
    // 把下线设备的外设信息全部刷新，同时通知到多模
    RemoveRemoteInputDevice(notice.networkId);
}

void InputDeviceMgr::AttachSender(Channel<CooperateEvent>::Sender sender)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    sender_ = sender;
}

bool InputDeviceMgr::OnRawData(const std::string &networkId, const void *data, uint32_t dataLen)
{
    // 实现对原始数据的解析.
    // 考虑对软总线数据 MessageId 的解析， 和跨端拖拽的区分，是否需要将 SoftbusMessageId 声明在蓝区
    SoftbusMessageId messageId;
    if (RawDataPacker::GetSoftbusMessageId(data, dataLen, messageId) != RET_OK) {
        FI_HILOGE("GetSoftbusMessageId from %{public}s failed", Utility::Anonymize(networkId).c_str());
        return false;
    }
    Parcel parcel;
    if (RawDataPacker::WriteRawDataToParcel(data, dataLen, parcel) != RET_OK) {
        FI_HILOGE("WriteRawDataToParcel from %{public}s failed", Utility::Anonymize(networkId).c_str());
        return false;
    }
    return OnParcel(networkId, messageId, parcel);
}

bool InputDeviceMgr::OnParcel(const std::string &networkId, SoftbusMessageId messageId, Parcel &parcel)
{
    CALL_DEBUG_ENTER;
    int32_t messageId = static_cast<int32_t>(messageId);
    auto it = rawDataHandlers_.find(messageId);
    if (it != rawDataHandlers_.end()) {
        (this->*(it->second))(networkId, parcel);
        return true;
    }
    FI_HILOGD("Unsupported softbusMessageId: %{public}d from %{public}s", messageId,
        Utility::Anonymize(networkId).c_str());
    return false;
}

void InputDeviceMgr::OnRemoteInputDeviceInfo(const std::string &networkId, Parcel &parcel)
{
    // 反序列化成设备信息，
    // 刷新 remoteInputDevice
    // 调用多模产生热插拔事件

}

void InputDeviceMgr::OnRemoteHotPlugInfo(const std::string &networkId, Parcel &parcel)
{
    // 反序列化成设备信息，
    // 刷新 remoteInputDevice
    // 调用多模产生热插拔事件
    
}

void InputDeviceMgr::OnLocalHotPlugEvent(const InputHotplugEvent &notice)
{
    CALL_INFO_TRACE;
    NotifyHotPlugToRemote(notice);
}

void InputDeviceMgr::NotifyInputDeviceToRemote(const std::string &networkId, const std::vector<std::shared_ptr<IDevice>> &keyboards)
{
    CALL_INFO_TRACE;
    Parcel parcel;
    if (!SerializeDeviceInfo(keyboards, parcel)) {
        FI_HILOGE("SerializeDeviceInfo failed");
        return;
    }
    if (env_->GetDsoftbus().SendParcel(networkId, parcel) != RET_OK) {
        FI_HILOGE("SendParcel failed");
        return RET_ERR;
    }
    return RET_OK;
}

void InputDeviceMgr::NotifyHotPlugToRemote(const DSoftbusNotifyDeviceInfo &notice)
{
    // 序列化，发送到对端
    // 通知穿入端设备，notice.deviceId 对应的外设已经下线，更新自己维护的列表
    // CALL_INFO_TRACE;
    // Parcel parcel;
    // if (!SerializeDeviceInfo(keyboards, parcel)) {
    //     FI_HILOGE("SerializeDeviceInfo failed");
    //     return;
    // }
    // if (env_->GetDsoftbus().SendParcel(networkId, parcel) != RET_OK) {
    //     FI_HILOGE("SendParcel failed");
    //     return RET_ERR;
    // }
    // return RET_OK;
}

void InputDeviceMgr::DumpInputDeviceInfo(const std::string &networkId)
{
    // 打印指定设备的外设信息
    if (remoteDeviceInfo_.find(networkId) == remoteDeviceInfo_.end()) {
        FI_HILOGE("No input deviceInfo of networkId:%{public}s", Utility::Anonymize(networkId).c_str());
    }
    for (const auto &elem : remoteDeviceInfo_[networkId]) {
        FI_HILOGI("Device info of each device");
    }
}

void InputDeviceMgr::RemoveRemoteInputDevice(const std::string &networkId)
{
    CALL_INFO_TRACE;
    for (const auto &elem : remoteDeviceInfo_[networkId]) {
        // 调用多模接口进行设备下线
    }
    remoteDeviceInfo_.erase(networkId);
}

void InputDeviceMgr::UpdateRemoteInputDeviceInfo()
{
    
}

bool InputDeviceMgr::SerializeDeviceInfo(const std::vector<IDevice> &devices, Parcel &parcel)
{
    CALL_INFO_TRACE;
    WRITEUINT32(parcel, SoftbusMessageId::INPUT_DEVICE_INFO, false);
    int32_t deviceNum = devices.size();
    WRITEINT32(parcel, deviceNum, false);
    for (const auto elem : devices) {
        CHKPR(elem, false);
        if (!Device::Marshalling(elem, parcel)) {
            FI_HILOGE("Marshalling deviceInfo failed");
            return false;
        }
    }
    return true;
}

bool InputDeviceMgr::DeSerializeDeviceInfo(Parcel &parcel, std::vector<IDevice> &devices)
{
    CALL_INFO_TRACE;
    uint32_t messageId { -1 };
    READUINT32(parcel, messageId, false);
    int32_t deviceNum { -1 };
    READINT32(parcel, deviceNum, false);
    if (deviceNum > 10 || deviceNum <= 0) {
        FI_HILOGE("Invalid deviceNum");
        return false;
    }
    for (int32_t i = 0; i < deviceNum; i++) {
        devices.push_back(std:make_shared<Device>(INVALID_DEVICE_ID));
        if (!Device::UnMarshalling(parcel, devices[i])) {
            FI_HILOGE("Marshalling deviceInfo failed");
            return false;   
        }
    }
    return true;
}

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
