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
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "nocopyable.h"
#include "token_setproc.h"

#include "coordination_sm_test.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
void SetAceessTokenPermission(const std::string &processName, const char** perms, size_t permCount)
{
    if (perms == nullptr || permCount == 0) {
        return;
    }
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permCount,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = processName.c_str(),
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void AddPermission()
{
    const char** perms = new const char* [1];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    SetAceessTokenPermission("CoordinationSMTest", perms, 1);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS