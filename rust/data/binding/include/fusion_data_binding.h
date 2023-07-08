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

#ifndef FUSION_DATA_BINDING_H
#define FUSION_DATA_BINDING_H
#include <cinttypes>

#include "drag_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CShadowInfo {
    int32_t x;
    int32_t y;
};

struct CDragData {
    struct CShadowInfo shadowInfo;
    const uint8_t *buffer;
    size_t bufSize;
    int32_t sourceType;
    int32_t dragNum;
    int32_t pointerId;
    int32_t displayX;
    int32_t displayY;
    int32_t displayId;
    bool hasCanceledAnimation;
};

struct CDragData* CDragDataFree(struct CDragData *cdrag);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // FUSION_DATA_BINDING_H
