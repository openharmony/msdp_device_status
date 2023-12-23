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

#include "cooperate_context.h"

#include "devicestatus_define.h"
#include "dsoftbus_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateContext" };
} // namespace

Context::Context(IContext *env)
    : env_(env), evMgr_(env)
{
    devMgr_ = std::make_shared<DeviceManager>(env);
    dinput_ = std::make_shared<DInputAdapter>(env);
}

void Context::Enable()
{
    CALL_DEBUG_ENTER;
    EnableDevMgr();
    EnableDSoftbus();
}

void Context::Disable()
{
    CALL_DEBUG_ENTER;
    DisableDSoftbus();
    DisableDevMgr();
}

int32_t Context::EnableDevMgr()
{
    devMgr_->AttachSender(sender_);
    devMgr_->Start();
    return RET_OK;
}

void Context::DisableDevMgr()
{
    devMgr_->Stop();
}

int32_t Context::EnableDSoftbus()
{
    return DSoftbusAdapter::GetInstance()->Enable();
}

void Context::DisableDSoftbus()
{
    DSoftbusAdapter::GetInstance()->Disable();
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS