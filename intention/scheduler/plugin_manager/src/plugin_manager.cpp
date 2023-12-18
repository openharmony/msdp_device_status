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

#include "plugin_manager.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "PluginManager" };
}

void PluginManager::Init(IContext *context)
{
    context_ = context;
}

ICooperate* PluginManager::LoadCooperate()
{
    CALL_DEBUG_ENTER;
    CHKPP(context_);
    if (cooperate_ == nullptr) {
        cooperate_ = LoadLibrary<ICooperate>(context_, "/system/lib/libintention_cooperate.z.so");
    }
    return (cooperate_ != nullptr ? cooperate_->GetInstance() : nullptr);
}

void PluginManager::UnloadCooperate()
{
    CALL_DEBUG_ENTER;
    cooperate_.reset();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
