/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef DEVICESTATUS_MOUDULE_TEST_H
#define DEVICESTATUS_MOUDULE_TEST_H

#include <gtest/gtest.h>

#include "devicestatus_callback_stub.h"
#include "stationary_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusModuleTest : public testing::Test {
public:

    class DeviceStatusModuleTestCallback : public DeviceStatusCallbackStub {
    public:
        DeviceStatusModuleTestCallback() {};
        virtual ~DeviceStatusModuleTestCallback() {};
        virtual void OnDeviceStatusChanged(const Data &devicestatusData) override;
    };
    static Type g_moduleTest;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_MOUDULE_TEST_H
