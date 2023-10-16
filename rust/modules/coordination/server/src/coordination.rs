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

//! Implementation of multi-device cooperation.

#![allow(dead_code)]

use std::ffi::{ c_char, CString };

use hilog_rust::{ hilog, HiLogLabel, LogType };

use fusion_data_rust::{ GeneralCoordinationParam, StartCoordinationParam, CallingContext,
    StopCoordinationParam, GetCoordinationStateParam };
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "Coordination"
};

#[derive(Default)]
pub struct Coordination(i32);

impl Coordination {
    pub fn enable(&self, context: &CallingContext,
        param: &GeneralCoordinationParam) -> FusionResult<()>
    {
        call_debug_enter!("Coordination::enable");
        Err(FusionErrorCode::Fail)
    }

    pub fn disable(&self, context: &CallingContext,
        param: &GeneralCoordinationParam) -> FusionResult<()>
    {
        call_debug_enter!("Coordination::disable");
        Err(FusionErrorCode::Fail)
    }

    pub fn start(&self, context: &CallingContext,
        param: &StartCoordinationParam) -> FusionResult<()>
    {
        call_debug_enter!("Coordination::start");
        Err(FusionErrorCode::Fail)
    }

    pub fn stop(&self, context: &CallingContext,
        param: &StopCoordinationParam) -> FusionResult<()>
    {
        call_debug_enter!("Coordination::stop");
        Err(FusionErrorCode::Fail)
    }

    pub fn get_state(&self, context: &CallingContext,
        param: &GetCoordinationStateParam) -> FusionResult<()>
    {
        call_debug_enter!("Coordination::get_state");
        Err(FusionErrorCode::Fail)
    }

    pub fn register_listener(&self, context: &CallingContext,
        param: &GeneralCoordinationParam) -> FusionResult<()>
    {
        call_debug_enter!("Coordination::register_listener");
        Err(FusionErrorCode::Fail)
    }

    pub fn unregister_listener(&self, context: &CallingContext,
        param: &GeneralCoordinationParam) -> FusionResult<()>
    {
        call_debug_enter!("Coordination::unregister_listener");
        Err(FusionErrorCode::Fail)
    }
}
