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

//! Drag client implementation.

#![allow(dead_code)]
#![allow(unused_variables)]

extern crate fusion_data_rust;
extern crate fusion_utils_rust;
extern crate ipc_rust;

use fusion_data_rust::{ Intention, AllocSocketPairParam, BasicParamID, FusionResult };
use fusion_utils_rust::call_debug_enter;
use fusion_ipc_client_rust::FusionIpcClient;
use ipc_rust::{ FileDesc, MsgParcel, Deserialize };
use std::ffi::{ c_char, CString };
use std::rc::Rc;
use hilog_rust::{ debug, error, info, hilog, HiLogLabel, LogType };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionBasicClient"
};

/// struct FusionBasicClient
#[derive(Default)]
pub struct FusionBasicClient {
    dummy: i32
}

impl FusionBasicClient {
    /// TODO: add documentation.
    pub fn alloc_socket_pair(&self, param: &AllocSocketPairParam, ipc_client: Rc<FusionIpcClient>)
        -> FusionResult<(FileDesc, i32)>
    {
        call_debug_enter!("FusionBasicClient::alloc_socket_pair");
        match MsgParcel::new() {
            Some(mut reply_parcel) => {
                let mut borrowed_reply_parcel = reply_parcel.borrowed();
                debug!(LOG_LABEL, "in DragClient::start_drag(): call ipc_client::start()");
                ipc_client.control(Intention::Basic, u32::from(BasicParamID::AllocSocketPair),
                    param, &mut borrowed_reply_parcel)?;

                if let Ok(fdesc) = FileDesc::deserialize(&borrowed_reply_parcel) {
                    if let Ok(token_type) = i32::deserialize(&borrowed_reply_parcel) {
                        info!(LOG_LABEL, "Deserialization succeeded");
                        return Ok((fdesc, token_type));
                    }
                }
                error!(LOG_LABEL, "Failed to deserialize reply");
                Err(-1)
            }
            None => {
                error!(LOG_LABEL, "in DragClient::start_drag(): can not instantiate MsgParcel");
                Err(-1)
            }
        }
    }
}
