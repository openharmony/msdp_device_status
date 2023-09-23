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

#ifndef FUSION_SERVICES_BINDING_H
#define FUSION_SERVICES_BINDING_H
#include <cinttypes>

#ifdef __cplusplus
extern "C" {
#endif

struct NativeService;

struct NativeService* NativeServiceNew(void);
struct NativeService* NativeServiceRef(struct NativeService *service);
struct NativeService* NativeServiceUnref(struct NativeService *service);

void NativeServiceOnDump(struct NativeService *service);
void NativeServiceOnStart(struct NativeService *service);
void NativeServiceOnStop(struct NativeService *service);

int32_t NativeServiceAllocSocketFd(struct NativeService *service, const char *programName,
    int32_t moduleType, int32_t *toReturnClientFd, int32_t *tokenType);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // FUSION_SERVICES_BINDING_H
