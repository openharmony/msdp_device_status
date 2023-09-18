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

extern crate fusion_data_rust;
extern crate fusion_services_rust;
extern crate fusion_utils_rust;
extern crate hilog_rust;
extern crate ipc_rust;

use std::ffi::{ c_char, CString };
use std::fs::File;
use std::os::fd::{ FromRawFd, RawFd };
use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Deserialize, FileDesc, Serialize };
use fusion_data_rust::{ IPlugin, AllocSocketPairParam, BasicParamID, CallingContext, FusionResult };
use fusion_services_rust::{ FusionService };
use fusion_utils_rust::{ call_debug_enter };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionBasicServer"
};

/// Module-level interface of general functionalities.
#[derive(Default)]
pub struct FusionBasicServer {
    dummy: i32
}

impl FusionBasicServer {
    fn alloc_socket_pair(&self, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>
    {
        let call_param = AllocSocketPairParam::deserialize(data).map_err(|_err| { -1 })?;

        if let Some(proxy) = FusionService::get_instance() {
            let mut client_fd: RawFd = 0;
            let mut token_type: i32 = 0;

            proxy.alloc_socket_fd(call_param.program_name.as_str(),
                call_param.module_type, &mut client_fd, &mut token_type)?;

            let f = unsafe { File::from_raw_fd(client_fd) };
            FileDesc::new(f).serialize(reply).map_err(|_err| { -1 })?;
            token_type.serialize(reply).map_err(|_err| { -1 })?;
            Ok(0)
        } else {
            error!(LOG_LABEL, "No proxy");
            Err(-1)
        }
    }
}

impl IPlugin for FusionBasicServer {
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::enable");
        Ok(0)
    }

    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::disable");
        Ok(0)
    }

    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::start");
        error!(LOG_LABEL, "FusionBasicServer::start");
        Err(-1)
    }

    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::stop");
        Ok(0)
    }

    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::add_watch");
        Ok(0)
    }

    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::remove_watch");
        Ok(0)
    }

    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::set_param");
        Ok(0)
    }

    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::get_param");
        Ok(0)
    }

    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionBasicServer::control");
        match BasicParamID::try_from(id) {
            Ok(param) => {
                match param {
                    BasicParamID::AllocSocketPair => {
                        info!(LOG_LABEL, "alloc socket pair");
                        self.alloc_socket_pair(data, reply)
                    }
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "Invalid param id: {}", id);
                Err(-1)
            }
        }
    }
}
