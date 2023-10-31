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

#ifndef VIRTUAL_DEVICE_BUILDER_H
#define VIRTUAL_DEVICE_BUILDER_H

#include <memory>
#include <vector>

#include <linux/uinput.h>

#include <nlohmann/json.hpp>
#include "nocopyable.h"

#include "virtual_device.h"
#include "virtual_device_defines.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using json = nlohmann::json;
class VirtualDeviceBuilder {
public:
    VirtualDeviceBuilder(const std::string &name, uint16_t bustype, uint16_t vendor, uint16_t product);
    VirtualDeviceBuilder(const std::string &name, std::shared_ptr<VirtualDevice> vDev);
    virtual ~VirtualDeviceBuilder();
    DISALLOW_COPY_AND_MOVE(VirtualDeviceBuilder);
    bool SetUp();
    void Close();

protected:
    static void Daemonize();
    static void Unmount(const char *name, const char *id);
    static void WaitFor(const char *path, const char *name);
    static void WaitFor(const char *name, int32_t timeout);
    static int32_t ReadFile(const char *path, json &model);
    static int32_t ScanFor(std::function<bool(std::shared_ptr<VirtualDevice>)> pred,
        std::vector<std::shared_ptr<VirtualDevice>> &vDevs);
    static std::shared_ptr<VirtualDevice> Select(std::vector<std::shared_ptr<VirtualDevice>> &vDevs, const char *name);
    void SetResolution(const ResolutionInfo &resolutionInfo);
    void SetAbsValue(const AbsInfo &absInfo);
    virtual const std::vector<uint32_t> &GetAbs() const;
    virtual const std::vector<uint32_t> &GetEventTypes() const;
    virtual const std::vector<uint32_t> &GetKeys() const;
    virtual const std::vector<uint32_t> &GetLeds() const;
    virtual const std::vector<uint32_t> &GetMiscellaneous() const;
    virtual const std::vector<uint32_t> &GetProperties() const;
    virtual const std::vector<uint32_t> &GetRelBits() const;
    virtual const std::vector<uint32_t> &GetRepeats() const;
    virtual const std::vector<uint32_t> &GetSwitches() const;

protected:
    std::vector<uinput_abs_setup> absInit_;
    std::vector<uint32_t> abs_;
    std::vector<uint32_t> relBits_;
    std::vector<uint32_t> switches_;
    std::vector<uint32_t> repeats_;
    std::vector<uint32_t> eventTypes_;
    std::vector<uint32_t> keys_;
    std::vector<uint32_t> properties_;
    std::vector<uint32_t> leds_;
    std::vector<uint32_t> miscellaneous_;

private:
    void SetPhys();
    void SetSupportedEvents();
    void SetAbsResolution();
    void SetIdentity();
    void CopyProperties(const std::string &name, std::shared_ptr<VirtualDevice> vDev);
    void CopyIdentity(const std::string &name, std::shared_ptr<VirtualDevice> vDev);
    void CopyAbsInfo(std::shared_ptr<VirtualDevice> vDev);
    void CopyEvents(std::shared_ptr<VirtualDevice> vDev);
    static void ConcatenationName(std::string &sLine);
    static bool ExecuteUnmount(const char *id, const char *name, const std::string &direntName);

private:
    int32_t fd_ { -1 };
    struct uinput_user_dev uinputDev_ {};
    struct uinput_abs_setup uinputAbs_ {};
    std::shared_ptr<VirtualDevice> vDev_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // VIRTUAL_DEVICE_BUILDER_H