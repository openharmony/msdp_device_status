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

#include <dlfcn.h>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "PluginManager" };
}

PluginManager::Plugin::Plugin(IContext *context, void *handle)
    : context_(context), handle_(handle)
{}

PluginManager::Plugin::~Plugin()
{
    if (instance_ != nullptr) {
        auto destroy = reinterpret_cast<DestroyInstance>(dlsym(handle_, "DestroyInstance"));
        if (destroy != nullptr) {
            destroy(instance_);
        }
    }
    dlclose(handle_);
}

IPlugin* PluginManager::Plugin::GetInstance()
{
    if (instance_ != nullptr) {
        return instance_;
    }
    auto func = reinterpret_cast<CreateInstance>(dlsym(handle_, "CreateInstance"));
    if (func == nullptr) {
        FI_HILOGE("dlsym msg:%{public}s", dlerror());
        return nullptr;
    }
    instance_ = func(context_);
    return instance_;
}

void PluginManager::Init(IContext *context)
{
    context_ = context;
}

IPlugin* PluginManager::LoadPlugin(Intention intention)
{
    switch (intention) {
        case Intention::DRAG:
        case Intention::COOPERATE: {
            return DoLoadPlugin(intention);
        }
        default: {
            FI_HILOGW("Intention is invalid");
            break;
        }
    }
    return nullptr;
}

int32_t PluginManager::LoadLibrary(Intention intention)
{
    std::string libPath;

    switch (intention) {
        case Intention::DRAG: {
            libPath = "/system/lib/libintention_drag.z.so";
        }
        case Intention::COOPERATE: {
            libPath = "/system/lib/libintention_cooperate.z.so";
        }
        default: {
            FI_HILOGW("Intention is invalid");
            return RET_ERR;
        }
    }

    void *handle = ::dlopen(libPath.c_str(), RTLD_NOW);
    if (handle == nullptr) {
        FI_HILOGE("Open plugin failed, plugin name:%{public}s, msg:%{public}s", libPath.c_str(), dlerror());
        return RET_ERR;
    }
    libs_.emplace(intention, Plugin(context_, handle));
    return RET_OK;
}

IPlugin* PluginManager::DoLoadPlugin(Intention intention)
{
    if (libs_.find(intention) == libs_.end()) {
        if (LoadLibrary(intention) != RET_OK) {
            return nullptr;
        }
    }
    auto iter = libs_.find(intention);
    if (iter == libs_.end()) {
        return nullptr;
    }
    return iter->second.GetInstance();
}

void PluginManager::UnloadPlugin(Intention intention)
{
    if (auto iter = libs_.find(intention); iter != libs_.end()) {
        libs_.erase(iter);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
