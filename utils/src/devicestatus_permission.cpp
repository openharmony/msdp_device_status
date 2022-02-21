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

namespace OHOS {
namespace Msdp {
bool DevicestatusPermission::CheckCallingPermission(const string &permissionName)
{
    string appIdInfo = GetAppInfo();
    int auth = Security::Permission::PermissionKit::CheckPermission(permissionName, appIdInfo);
    return auth == Security::Permission::PermissionKit::GRANTED;
}

string DevicestatusPermission::GetAppInfo()
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    uid_t uid = IPCSkeleton::GetCallingUid();
    return Security::Permission::AppIdInfoHelper::CreateAppIdInfo(pid, uid);
}
}  // namespace Msdp
}  // namespace OHOS
