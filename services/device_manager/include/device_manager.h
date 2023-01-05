/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <future>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "nocopyable.h"

#include "enumerator.h"
#include "i_context.h"
#include "i_epoll_event_source.h"
#include "i_input_dev_mgr.h"
#include "monitor.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceManager final : public IDeviceManager,
                            public IEpollEventSource {
public:
    DeviceManager();
    ~DeviceManager() = default;
    DISALLOW_COPY_AND_MOVE(DeviceManager);

    int32_t GetFd() const override;
    void Dispatch(const struct epoll_event &ev) override;

    int32_t Init(IContext *context);
    int32_t Enable();
    int32_t Disable();

    int32_t AddDeviceObserver(std::shared_ptr<IDeviceObserver> observer) override;
    void RemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer) override;

    std::shared_ptr<IDevice> GetDevice(int32_t id) const override;

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    virtual bool IsRemote(int32_t id) const override;
    virtual std::vector<std::string> GetCoordinationDhids(int32_t deviceId) const override;
    virtual std::vector<std::string> GetCoordinationDhids(const std::string &dhid) const override;
    virtual std::string GetOriginNetworkId(int32_t id) const override;
    virtual std::string GetOriginNetworkId(const std::string &dhid) const override;
    virtual std::string GetDhid(int32_t deviceId) const override;
    virtual bool HasLocalPointerDevice() const override;
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    class HotplugHandler final : public IInputDevMgr
    {
    public:
        explicit HotplugHandler(DeviceManager &devMgr);
        ~HotplugHandler() = default;

        void AddInputDevice(const std::string &devNode) override;
        void RemoveInputDevice(const std::string &devNode) override;

    private:
        DeviceManager &devMgr_;
    };

private:
    int32_t OnInit(IContext *context);
    int32_t OnEnable();
    int32_t OnDisable();
    int32_t OnEpollDispatch();

    int32_t ParseDeviceId(const std::string &devNode);
    std::shared_ptr<IDevice> AddInputDevice(const std::string &devNode);
    std::shared_ptr<IDevice> RemoveInputDevice(const std::string &devNode);

    std::shared_ptr<IDevice> FindInputDevice(const std::string &devPath);
    void OnInputDeviceAdded(std::shared_ptr<IDevice> dev);
    void OnInputDeviceRemoved(std::shared_ptr<IDevice> dev);

    int32_t OnAddDeviceObserver(std::shared_ptr<IDeviceObserver> observer);
    int32_t OnRemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer);

    int32_t EpollCreate();
    int32_t EpollAdd(IEpollEventSource *source);
    void EpollDel(IEpollEventSource *source);
    void EpollClose();

    std::shared_ptr<IDevice> OnGetDevice(int32_t id) const;
    int32_t RunGetDevice(std::packaged_task<std::shared_ptr<IDevice>(int32_t)> &task, int32_t id) const;

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    bool OnIsRemote(int32_t id) const;
    int32_t RunIsRemote(std::packaged_task<bool(int32_t)> &task, int32_t id) const;

    std::vector<std::string> OnGetCoopDhids(int32_t deviceId) const;
    int32_t RunGetGetCoopDhids(std::packaged_task<std::vector<std::string>(int32_t)> &task,
                               int32_t deviceId) const;

    std::vector<std::string> OnGetCoordinationDhids(const std::string &dhid) const;
    int32_t RunGetCoordinationDhids(std::packaged_task<std::vector<std::string>(const std::string &)> &task,
                                 const std::string &dhid) const;

    std::string OnGetOriginNetId(int32_t id) const;
    int32_t RunGetOriginNetId(std::packaged_task<std::string(int32_t)> &task,
                              int32_t id) const;

    std::string OnGetOriginNetworkId(const std::string &dhid) const;
    int32_t RunGetOriginNetworkId(std::packaged_task<std::string(const std::string &)> &task,
                                  const std::string &dhid) const;

    std::string OnGetDhid(int32_t deviceId) const;
    int32_t RunGetDhid(std::packaged_task<std::string(int32_t)> &task, int32_t deviceId) const;

    bool OnHasLocalPointerDevice() const;
    int32_t RunHasLocalPointerDevice(std::packaged_task<bool()> &task) const;
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    IContext *context_ { nullptr };
    int32_t epollFd_ { -1 };
    Enumerator enumerator_;
    Monitor monitor_;
    HotplugHandler hotplug_;
    std::set<std::shared_ptr<IDeviceObserver>> observers_;
    std::unordered_map<int32_t, std::shared_ptr<IDevice>> devices_;
};

inline int32_t DeviceManager::GetFd() const
{
    return epollFd_;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_MANAGER_H