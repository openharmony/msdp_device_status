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

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "PluginManager" };
}

using CreatePlugin = IPlugin* (*)(IContext *context);
using DestroyPlugin = void (*)(IPlugin *);

PluginManager::Plugin::Plugin(IContext *context, void *handle)
    : context_(context), handle_(handle)
{}

PluginManager::Plugin::Plugin(Plugin &&other)
    : context_(other.context_), handle_(other.handle_), instance_(other.instance_)
{
    other.context_ = nullptr;
    other.handle_ = nullptr;
    other.instance_ = nullptr;
}

PluginManager::Plugin::~Plugin()
{
    if (handle_ != nullptr) {
        if (instance_ != nullptr) {
            DestroyPlugin destroy =
                reinterpret_cast<DestroyPlugin>(dlsym(handle_, "DestroyInstance"));
            if (destroy != nullptr) {
                destroy(instance_);
            }
        }
        dlclose(handle_);
    }
}

PluginManager::Plugin& PluginManager::Plugin::operator=(Plugin &&other)
{
    context_ = other.context_;
    handle_ = other.handle_;
    instance_ = other.instance_;
    other.context_ = nullptr;
    other.handle_ = nullptr;
    other.instance_ = nullptr;
    return *this;
}

IPlugin* PluginManager::Plugin::GetInstance()
{
    if (instance_ != nullptr) {
        return instance_;
    }
    CHKPP(handle_);
    CreatePlugin func = reinterpret_cast<CreatePlugin>(dlsym(handle_, "CreateInstance"));
    if (func == nullptr) {
        FI_HILOGE("dlsym msg:%{public}s", dlerror());
        return nullptr;
    }
    instance_ = func(context_);
    return instance_;
}

PluginManager::PluginManager(IContext *context)
    : context_(context)
{}

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

void PluginManager::LoadLibrary(Intention intention)
{
    std::string libPath;

    switch (intention) {
        case Intention::DRAG: {
            libPath = "/system/lib/libintention_drag.z.so";
            break;
        }
        case Intention::COOPERATE: {
            libPath = "/system/lib/libintention_cooperate.z.so";
            break;
        }
        default: {
            FI_HILOGW("Intention is invalid");
            return;
        }
    }

    void *handle = ::dlopen(libPath.c_str(), RTLD_NOW);
    if (handle == nullptr) {
        FI_HILOGE("Open plugin failed, plugin name:%{public}s, msg:%{public}s", libPath.c_str(), dlerror());
        return;
    }
    libs_.emplace(intention, Plugin(context_, handle));
}

IPlugin* PluginManager::DoLoadPlugin(Intention intention)
{
    auto iter = libs_.find(intention);
    if (iter == libs_.end()) {
        LoadLibrary(intention);
        if (iter = libs_.find(intention); iter == libs_.end()) {
            return nullptr;
        }
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
