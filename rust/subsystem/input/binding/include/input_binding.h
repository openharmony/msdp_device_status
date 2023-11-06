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

#ifndef INPUT_BINDING_H
#define INPUT_BINDING_H

#ifdef __cplusplus
extern "C" {
#endif

struct CPointerEvent;
struct CKeyEvent;
struct CAxisEvent;
struct CPointerStyle;
struct CExtraData;

int32_t CGetPointerId(const CPointerEvent* cPointerEvent);
int32_t CGetPointerAction(const CPointerEvent* cPointerEvent);
int32_t CGetTargetWindowId(const CPointerEvent* cPointerEvent);
int32_t CGetSourceType(const CPointerEvent* cPointerEvent);
int32_t CGetTargetDisplayId(const CPointerEvent* cPointerEvent);
int32_t CGetDisplayX(const CPointerEvent* cPointerEvent);
int32_t CGetDisplayY(const CPointerEvent* cPointerEvent);
void CPointerEventAddFlag(const CPointerEvent* cPointerEvent);
void CKeyEventAddFlag(const CKeyEvent* cKeyEvent);
int32_t CGetDeviceId(const CPointerEvent* cPointerEvent);
int32_t CGetKeyCode(const CKeyEvent* cKeyEvent);

int32_t CAddMonitor(void (*callback)(CPointerEvent *));
int32_t CGetWindowPid(const CPointerEvent* cPointerEvent);
int32_t CGetPointerStyle(CPointerStyle* cPointerStyle);
void CAppendExtraData(CExtraData cExtraData);
int32_t CSetPointerVisible(bool visible);
int32_t CEnableInputDevice(bool enable);
int32_t CRemoveInputEventFilter(int filterId);
void CRemoveMonitor(int monitorId);
void CRemoveInterceptor(int interceptorId);
void CSetPointerLocation(int physicalX, int physicalY);

void CDestroyPointerEvent(CPointerEvent* cPointerEvent);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // INPUT_BINDING_H