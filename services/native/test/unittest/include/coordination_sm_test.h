/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COORDINATION_SM_TEST_H
#define COORDINATION_SM_TEST_H
#include "coordination_sm.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

void ClearCoordiantionSM()
{
    COOR_SM->preparedNetworkId_ = { "", "" } ;
    COOR_SM->startDeviceDhid_ = "";
    COOR_SM->remoteNetworkId_ = "";
    COOR_SM->sinkNetworkId_ = "";
    COOR_SM->isUnchained_ = false;
    COOR_SM->currentState_ = CoordinationState::STATE_FREE;
    COOR_SM->initCallback_ = nullptr;
    COOR_SM->stateCallback_ = nullptr;
    COOR_SM->isStarting_ = false ;
    COOR_SM->isStopping_ = false ;
    COOR_SM->mouseLocation_ = std::make_pair(0, 0);
    COOR_SM->lastPointerEvent_ = nullptr;
    COOR_SM->displayX_ = -1 ;
    COOR_SM->displayY_ = -1 ;
    COOR_SM->interceptorId_ = -1 ;
    COOR_SM->monitorId_ = -1 ;
    COOR_SM->filterId_ = -1 ;
    COOR_SM->remoteNetworkIdCallback_ = nullptr;
    COOR_SM->mouseLocationCallback_ = nullptr;
    COOR_SM->notifyDragCancelCallback_ = nullptr;
    COOR_SM->runner_ = nullptr;
    COOR_SM->onlineDevice_.clear();
    COOR_SM->stateChangedCallbacks_.clear();
    COOR_SM->coordinationStates_.clear();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_SM_TEST_H