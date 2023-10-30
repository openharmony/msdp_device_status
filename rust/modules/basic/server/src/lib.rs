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

//! General functionalities.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, CString };
use std::fs::File;
use std::os::fd::{ FromRawFd, RawFd };

use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Deserialize, FileDesc, Serialize };

use fusion_data_rust::{ IPlugin, AllocSocketPairParam, BasicParamID, CallingContext };
use fusion_services_rust::FusionService;
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionBasicServer"
};

/// Module-level interface of general functionalities.
#[derive(Default)]
pub struct FusionBasicServer(i32);

impl FusionBasicServer {
    fn alloc_socket_pair(&self, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>
    {
        let call_param = AllocSocketPairParam::deserialize(data).or(Err(FusionErrorCode::Fail))?;

        if let Some(proxy) = FusionService::get_instance() {
            let mut client_fd: RawFd = 0;
            let mut token_type: i32 = 0;

            proxy.alloc_socket_fd(call_param.program_name.as_str(),
                call_param.module_type, &mut client_fd, &mut token_type)?;

            let f = unsafe { File::from_raw_fd(client_fd) };
            FileDesc::new(f).serialize(reply).or(Err(FusionErrorCode::Fail))?;
            token_type.serialize(reply).or(Err(FusionErrorCode::Fail))?;
            Ok(())
        } else {
            error!(LOG_LABEL, "No proxy");
            Err(FusionErrorCode::Fail)
        }
    }
}

impl IPlugin for FusionBasicServer {
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::enable");
        Ok(())
    }

    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::disable");
        Ok(())
    }

    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::start");
        error!(LOG_LABEL, "FusionBasicServer::start");
        Err(FusionErrorCode::Fail)
    }

    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::stop");
        Ok(())
    }

    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::add_watch");
        Ok(())
    }

    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::remove_watch");
        Ok(())
    }

    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::set_param");
        Ok(())
    }

    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::get_param");
        Ok(())
    }

    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionBasicServer::control");
        match BasicParamID::try_from(id) {
            Ok(param) => {
                match param {
                    BasicParamID::AllocSocketPair => {
                        info!(LOG_LABEL, "Alloc socket pair");
                        self.alloc_socket_pair(data, reply)
                    }
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "Invalid param id: {}", id);
                Err(FusionErrorCode::Fail)
            }
        }
    }
}
