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

#ifndef DEVICESTATUS_PERMISSION_H
#define DEVICESTATUS_PERMISSION_H

#include <string>

namespace OHOS {
namespace Msdp {
class DevicestatusPermission {
public:
    /* check caller's permission by finding pid uid by system */
    static bool CheckCallingPermission(const std::string &permissionName);

    /* construct appIdInfo string */
    static std::string FindAppIdInfo();
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_PERMISSION_H