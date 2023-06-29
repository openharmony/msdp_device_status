/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef FUSION_IPC_SERVER_H
#define FUSION_IPC_SERVER_H
#include <cinttypes>

#ifdef __cplusplus
extern "C" {
#endif

struct FusionIpcServer;

struct FusionIpcServer* fusion_ipc_server_new();

void fusion_ipc_server_delete(struct FusionIpcServer *service);

int32_t fusion_ipc_server_publish(struct FusionIpcServer *service);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // FUSION_IPC_SERVER_H
