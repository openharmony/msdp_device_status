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

//! Coordination Server implementation.

#![allow(dead_code)]
#![allow(unused_variables)]

extern crate fusion_data_rust;
extern crate fusion_utils_rust;
extern crate fusion_plugin_manager_rust;
extern crate hilog_rust;
extern crate ipc_rust;

mod coordination;

use std::ffi::{ c_char, CString };
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Deserialize };
use fusion_data_rust::{ IPlugin, CallingContext, GeneralCoordinationParam, StartCoordinationParam,
    StopCoordinationParam, GetCoordinationStateParam};
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use fusion_plugin_manager_rust::export_plugin;
use coordination::Coordination;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionCoordinationServer"
};

/// struct FusionCoordinationServer
#[derive(Default)]
pub struct FusionCoordinationServer(Coordination);

// impl FusionCoordinationServer {
//     /// TODO: add documentation.
//     pub fn new(ops: &FusionDragOperations) -> Self {
//         Self(ops.clone())
//     }
// }

impl IPlugin for FusionCoordinationServer {
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::enable");
        match GeneralCoordinationParam::deserialize(data) {
            Ok(param) => {
                self.0.enable(context, &param)
            }
            Err(_) => {
                error!(LOG_LABEL, "GeneralCoordinationParam::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::disable");
        match GeneralCoordinationParam::deserialize(data) {
            Ok(param) => {
                self.0.disable(context, &param)
            }
            Err(_) => {
                error!(LOG_LABEL, "GeneralCoordinationParam::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::start");
        match StartCoordinationParam::deserialize(data) {
            Ok(param) => {
                self.0.start(context, &param)
            }
            Err(_) => {
                error!(LOG_LABEL, "StartCoordinationParam::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::stop");
        match StopCoordinationParam::deserialize(data) {
            Ok(param) => {
                self.0.stop(context, &param)
            }
            Err(_) => {
                error!(LOG_LABEL, "StopCoordinationParam::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::add_watch");
        match GeneralCoordinationParam::deserialize(data) {
            Ok(param) => {
                self.0.register_listener(context, &param)
            }
            Err(_) => {
                error!(LOG_LABEL, "GeneralCoordinationParam::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::remove_watch");
        match GeneralCoordinationParam::deserialize(data) {
            Ok(param) => {
                self.0.unregister_listener(context, &param)
            }
            Err(_) => {
                error!(LOG_LABEL, "GeneralCoordinationParam::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::set_param");
        Err(FusionErrorCode::Fail)
    }

    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::get_param");
        match GetCoordinationStateParam::deserialize(data) {
            Ok(param) => {
                self.0.get_state(context, &param)
            }
            Err(_) => {
                error!(LOG_LABEL, "GetCoordinationStateParam::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::control");
        Err(FusionErrorCode::Fail)
    }
}

export_plugin!(FusionCoordinationServer);
