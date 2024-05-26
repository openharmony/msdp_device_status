/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef KEY_DEVICE_INFO
#define KEY_DEVICE_INFO

#include <string>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

struct KeyDeviceInfo {
    int32_t deviceId { -1 };
    std::string dhid;
    std::string name;
    std::string unique;
    std::string networkId;
    bool isKeyboard { false };
    bool isPointerDevice { false };
};

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // KEY_DEVICE_INFO