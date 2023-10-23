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

//! Helper functions to deal with function id.

#![allow(dead_code)]

use std::ffi::{ c_char, CString };

use hilog_rust::{ info, hilog, HiLogLabel, LogType };

use fusion_data_rust::Intention;
use fusion_utils_rust::{ define_enum, FusionResult, FusionErrorCode };

define_enum! {
    CommonAction {
        Enable,
        Disable,
        Start,
        Stop,
        AddWatch,
        RemoveWatch,
        SetParam,
        GetParam,
        Control
    }
}

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionIpcServiceIdentity"
};

const PARAMBITS: u32 = 12;
const PARAMMASK: u32 = (1u32 << PARAMBITS) - 1u32;
const INTENTIONSHIFT: u32 = PARAMBITS;
const INTENTIONBITS: u32 = 8;
const INTENTIONMASK: u32 = (1u32 << INTENTIONBITS) - 1u32;
const ACTIONSHIFT: u32 = INTENTIONSHIFT + INTENTIONBITS;
const ACTIONBITS: u32 = 4;
const ACTIONMASK: u32 = (1u32 << ACTIONBITS) - 1u32;

pub fn compose_param_id(action: CommonAction, intention: Intention, param: u32) -> u32
{
    info!(LOG_LABEL, "In FusionIpcServiceIdentity::compose_param_id(): enter");
    ((action as u32 & ACTIONMASK) << ACTIONSHIFT) |
    ((intention as u32 & INTENTIONMASK) << INTENTIONSHIFT) |
    (param & PARAMMASK)
}

pub fn split_action(code: u32) -> FusionResult<CommonAction>
{
    info!(LOG_LABEL, "In FusionIpcServiceIdentity::split_action(): enter");
    CommonAction::try_from((code >> ACTIONSHIFT) & ACTIONMASK)
}

pub fn split_intention(code: u32) -> FusionResult<Intention>
{
    info!(LOG_LABEL, "In FusionIpcServiceIdentity::split_intention(): enter");
    Intention::try_from((code >> INTENTIONSHIFT) & INTENTIONMASK)
}

pub fn split_param(code: u32) -> u32
{
    info!(LOG_LABEL, "In FusionIpcServiceIdentity::split_param(): enter");
    code & PARAMMASK
}
