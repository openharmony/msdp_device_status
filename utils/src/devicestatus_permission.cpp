/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace Msdp {
bool DevicestatusPermission::CheckCallingPermission(const string &permissionName)
{
    Security::AccessToken::AccessTokenID callingToken = IPCSkeleton::GetCallingTokenID();
    int32_t auth = Security::AccessToken::TypePermissionState::PERMISSION_DENIED;
    auth = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callingToken, permissionName);
    if (auth == Security::AccessToken::TypePermissionState::PERMISSION_GRANTED) {
        return ERR_OK;
    } else {
        DEV_HILOGD(COMMON, "has no permission.permission name = %{public}s", permissionName.c_str());
        return ERR_NG;
    }
}

string DevicestatusPermission::GetAppInfo()
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    uid_t uid = IPCSkeleton::GetCallingUid();
    return Security::Permission::AppIdInfoHelper::CreateAppIdInfo(pid, uid);
}
}  // namespace Msdp
}  // namespace OHOS
