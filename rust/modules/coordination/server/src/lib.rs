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

//! Service of multi-device cooperation.

#![allow(dead_code)]
#![allow(unused_variables)]

mod coordination;

use std::ffi::{ c_char, CString };

use hilog_rust::{ hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Deserialize };

use fusion_data_rust::{ IPlugin, CallingContext, GeneralCoordinationParam, StartCoordinationParam,
    StopCoordinationParam, GetCoordinationStateParam };
use fusion_plugin_manager_rust::export_plugin;
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };

use coordination::Coordination;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionCoordinationServer"
};

/// Module-level interface of multi-device cooperation.
#[derive(Default)]
struct FusionCoordinationServer(Coordination);

impl IPlugin for FusionCoordinationServer {
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::enable");
        let param = GeneralCoordinationParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;
        self.0.enable(context, &param)
    }

    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::disable");
        let param = GeneralCoordinationParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;
        self.0.disable(context, &param)
    }

    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::start");
        let param = StartCoordinationParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;
        self.0.start(context, &param)
    }

    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::stop");
        let param = StopCoordinationParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;
        self.0.stop(context, &param)
    }

    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::add_watch");
        let param = GeneralCoordinationParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;
        self.0.register_listener(context, &param)
    }

    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::remove_watch");
        let param = GeneralCoordinationParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;
        self.0.unregister_listener(context, &param)
    }

    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::set_param");
        Err(FusionErrorCode::Fail)
    }

    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::get_param");
        let param = GetCoordinationStateParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;
        self.0.get_state(context, &param)
    }

    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionCoordinationServer::control");
        Err(FusionErrorCode::Fail)
    }
}

export_plugin!(FusionCoordinationServer);
