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

#include "devicestatus_service.h"

#include <vector>
#include <ipc_skeleton.h>
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "devicestatus_permission.h"
#include "devicestatus_common.h"
#include "devicestatus_dumper.h"
#include "hisysevent.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Msdp {
using namespace OHOS::HiviewDFX;
namespace {
auto ms = DelayedSpSingleton<DevicestatusService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(ms.GetRefPtr());
}
DevicestatusService::DevicestatusService() : SystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, true)
{
    DEV_HILOGD(SERVICE, "Add SystemAbility");
}

DevicestatusService::~DevicestatusService() {}

void DevicestatusService::OnDump()
{
    DEV_HILOGI(SERVICE, "OnDump");
}

void DevicestatusService::OnStart()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (ready_) {
        DEV_HILOGE(SERVICE, "OnStart is ready, nothing to do");
        return;
    }

    if (!Init()) {
        DEV_HILOGE(SERVICE, "OnStart call init fail");
        return;
    }
    if (!Publish(DelayedSpSingleton<DevicestatusService>::GetInstance())) {
        DEV_HILOGE(SERVICE, "OnStart register to system ability manager failed");
        return;
    }
    ready_ = true;
    DEV_HILOGI(SERVICE, "OnStart and add system ability success");
}

void DevicestatusService::OnStop()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (!ready_) {
        return;
    }
    ready_ = false;

    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "devicestatusManager_ is null");
        return;
    }
    devicestatusManager_->UnloadAlgorithm();
    DEV_HILOGI(SERVICE, "unload algorithm library exit");
}

int DevicestatusService::Dump(int fd, const std::vector<std::u16string>& args)
{
    DEV_HILOGI(SERVICE, "dump DeviceStatusServiceInfo");
    if (fd < 0) {
        DEV_HILOGE(SERVICE, "fd is invalid");
        return RET_ERR;
    }
    DevicestatusDumper &deviceStatusDumper = DevicestatusDumper::GetInstance();
    if (args.empty()) {
        DEV_HILOGE(SERVICE, "param cannot be empty");
        dprintf(fd, "param cannot be empty\n");
        deviceStatusDumper.DumpHelpInfo(fd);
        return RET_ERR;
    }
    std::vector<std::string> argList = { "" };
    std::transform(args.begin(), args.end(), std::back_inserter(argList),
        [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    DevicestatusDataUtils::DevicestatusType type;
    std::vector<DevicestatusDataUtils::DevicestatusData> datas;
    for (type = DevicestatusDataUtils::TYPE_STILL;
        type <= DevicestatusDataUtils::TYPE_LID_OPEN;
        type = (DevicestatusDataUtils::DevicestatusType)(type+1)) {
        DevicestatusDataUtils::DevicestatusData data = GetCache(type);
        if (data.value != DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) {
            datas.emplace_back(data);
        }
    }
    deviceStatusDumper.ParseCommand(fd, argList, datas);
    return RET_OK;
}


bool DevicestatusService::Init()
{
    DEV_HILOGI(SERVICE, "Enter");

    if (!devicestatusManager_) {
        devicestatusManager_ = std::make_shared<DevicestatusManager>(ms);
    }
    if (!devicestatusManager_->Init()) {
        DEV_HILOGE(SERVICE, "OnStart init fail");
        return false;
    }

    return true;
}

void DevicestatusService::Subscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "UnSubscribe func is nullptr");
        return;
    }

    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = GetCallingUid();
    appInfo->pid = GetCallingPid();
    appInfo->tokenId = GetCallingTokenID();
    devicestatusManager_->GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = type;
    appInfo->callback = callback;
    DevicestatusDumper::GetInstance().SaveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceSubcribeStart");
    devicestatusManager_->Subscribe(type, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportMsdpSysEvent(type, true);
}

void DevicestatusService::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "UnSubscribe func is nullptr");
        return;
    }

    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = GetCallingUid();
    appInfo->pid = GetCallingPid();
    appInfo->tokenId = GetCallingTokenID();
    devicestatusManager_->GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = type;
    appInfo->callback = callback;
    DevicestatusDumper::GetInstance().RemoveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceUnSubcribeStart");
    devicestatusManager_->UnSubscribe(type, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportMsdpSysEvent(type, false);
}

DevicestatusDataUtils::DevicestatusData DevicestatusService::GetCache(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DevicestatusDataUtils::DevicestatusData data = {type, DevicestatusDataUtils::DevicestatusValue::VALUE_EXIT};
        data.value = DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID;
        DEV_HILOGI(SERVICE, "GetLatestDevicestatusData func is nullptr,return default!");
        return data;
    }
    return devicestatusManager_->GetLatestDevicestatusData(type);
}

void DevicestatusService::ReportMsdpSysEvent(const DevicestatusDataUtils::DevicestatusType& type, bool enable)
{
    auto uid = this->GetCallingUid();
    auto callerToken = this->GetCallingTokenID();
    std::string packageName("");
    devicestatusManager_->GetPackageName(callerToken, packageName);
    std::string message;
    if (enable) {
        HiSysEvent::Write(HiSysEvent::Domain::MSDP, "SUBSCRIBE", HiSysEvent::EventType::STATISTIC,
            "UID", uid, "PKGNAME", packageName, "TYPE", type);
        return;
    }
    HiSysEvent::Write(HiSysEvent::Domain::MSDP, "UNSUBSCRIBE", HiSysEvent::EventType::STATISTIC,
        "UID", uid, "PKGNAME", packageName, "TYPE", type);
}
} // namespace Msdp
} // namespace OHOS
