/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#include "devicestatus_permission.h"

#include "ipc_skeleton.h"
#include "accesstoken_kit.h"

#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "DeviceStatusPermission"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

bool DeviceStatusPermission::CheckCallingPermission(const std::string &permissionName)
{
    Security::AccessToken::AccessTokenID callingToken = IPCSkeleton::GetCallingTokenID();
    int32_t auth = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callingToken, permissionName);
    if (auth != Security::AccessToken::TypePermissionState::PERMISSION_GRANTED) {
        FI_HILOGD("Has no permission.permission name = %{public}s", permissionName.c_str());
        return ERR_NG;
    }
    return ERR_OK;
}

std::string DeviceStatusPermission::GetAppInfo()
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    uid_t uid = IPCSkeleton::GetCallingUid();
    return Security::Permission::AppIdInfoHelper::CreateAppIdInfo(pid, uid);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
