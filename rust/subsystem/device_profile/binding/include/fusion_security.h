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

#ifndef FUSION_SECURITY_H
#define FUSION_SECURITY_H

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CIString {
    CIString* (*clone)(CIString*);
    void (*destruct)(CIString*);
    const char *(*data)(CIString*);
};

void GetAccessToken();
CIString* GetLocalNetworkId();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // FUSION_SECURITY_H
