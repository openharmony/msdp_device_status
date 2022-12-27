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
#ifndef MMI_FD_LISTENER_H
#define MMI_FD_LISTENER_H

#include "file_descriptor_listener.h"

#include "i_client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class FdListener final : public AppExecFwk::FileDescriptorListener {
public:
    explicit FdListener(IClientPtr client);
    DISALLOW_COPY_AND_MOVE(FdListener);
    ~FdListener() override = default;

    void OnReadable(int32_t fd) override;
    void OnShutdown(int32_t fd) override;
    void OnException(int32_t fd) override;

private:
    IClientPtr iClient_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // MMI_FD_LISTENER_H