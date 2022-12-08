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

#include "device.h"

#include <regex>

#include <openssl/sha.h>

#include "input_manager.h"

#include "fi_log.h"
#include "napi_constants.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "Device" };
const std::string INPUT_VIRTUAL_DEVICE_NAME { "DistributedInput " };
const std::string SPLIT_SYMBOL { "|" };
const std::string DH_ID_PREFIX { "Input_" };
} // namespace

Device::Device(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev)
    : inputDev_(inputDev)
{
    Populate();
}

void Device::Populate()
{
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    if (IsRemote()) {
        networkId_ = MakeNetworkId(GetPhys());
    }
    dhid_ = GenerateDescriptor();
#endif // OHOS_BUILD_ENABLE_COORDINATION

    if (IsKeyboard()) {
        int32_t ret = InputMgr->GetKeyboardType(GetId(),
            std::bind(&Device::onKeyboardTypeObtained, this, std::placeholders::_1));
        if (ret != 0) {
            FI_HILOGE("GetKeyboardType failed");
        }
    }
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION

bool Device::IsRemote() const
{
    return (GetName().find(INPUT_VIRTUAL_DEVICE_NAME) != std::string::npos);
}

std::string Device::MakeNetworkId(const std::string &phys) const
{
    std::vector<std::string> strList;
    StringSplit(phys, SPLIT_SYMBOL, strList);
    if (strList.size() == 3) {
        return strList[1];
    }
    return EMPTYSTR;
}

std::string Device::GenerateDescriptor() const
{
    const std::string phys = GetPhys();
    std::string descriptor;
    if (IsRemote() && !phys.empty()) {
        FI_HILOGI("physicalPath:%{public}s", phys.c_str());
        std::vector<std::string> strList;
        StringSplit(phys.c_str(), SPLIT_SYMBOL, strList);
        if (strList.size() == 3) {
            descriptor = strList[2];
        }
        return descriptor;
    }

    int32_t vendor = GetVendor();
    const std::string name = GetName();
    const std::string uniq = GetUniq();
    int32_t product = GetProduct();
    std::string rawDescriptor = StringPrintf(":%04x:%04x:", vendor, product);

    // add handling for USB devices to not unique kbs that show up twice
    if (!uniq.empty()) {
        rawDescriptor += "uniqueId:" + uniq;
    }
    if (!phys.empty()) {
        rawDescriptor += "physicalPath:" + phys;
    }
    if (!name.empty()) {
        rawDescriptor += "name:" + std::regex_replace(name, std::regex(" "), "");
    }
    descriptor = DH_ID_PREFIX + Sha256(rawDescriptor);
    FI_HILOGI("Created descriptor raw: %{public}s", rawDescriptor.c_str());
    return descriptor;
}

std::string Device::Sha256(const std::string &in) const
{
    unsigned char out[SHA256_DIGEST_LENGTH * 2 + 1] = {0};
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, in.data(), in.size());
    SHA256_Final(&out[SHA256_DIGEST_LENGTH], &ctx);
    // here we translate sha256 hash to hexadecimal. each 8-bit char will be presented by two characters([0-9a-f])
    constexpr int32_t WIDTH = 4;
    constexpr unsigned char MASK = 0x0F;
    const char* hexCode = "0123456789abcdef";
    constexpr int32_t DOUBLE_TIMES = 2;
    for (int32_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char value = out[SHA256_DIGEST_LENGTH + i];
        // uint8_t is 2 digits in hexadecimal.
        out[i * DOUBLE_TIMES] = hexCode[(value >> WIDTH) & MASK];
        out[i * DOUBLE_TIMES + 1] = hexCode[value & MASK];
    }
    out[SHA256_DIGEST_LENGTH * DOUBLE_TIMES] = 0;
    return reinterpret_cast<char*>(out);
}

#endif // OHOS_BUILD_ENABLE_COORDINATION

void Device::onKeyboardTypeObtained(int32_t keyboardType)
{
    if (keyboardType >= ::OHOS::MMI::KEYBOARD_TYPE_NONE &&
        keyboardType < ::OHOS::MMI::KEYBOARD_TYPE_MAX) {
        keyboardType_ = static_cast<::OHOS::MMI::KeyboardType>(keyboardType);
    } else {
        keyboardType_ = ::OHOS::MMI::KEYBOARD_TYPE_NONE;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS