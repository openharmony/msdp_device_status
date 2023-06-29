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

use fusion_data_rust::{ Intention, DefaultReply, DragData, FusionResult };
use fusion_utils_rust::call_debug_enter;
use fusion_ipc_client_rust::FusionIpcClient;
use ipc_rust::{ MsgParcel, Deserialize };
use std::ffi::{ c_char, CString };
use std::rc::Rc;
use hilog_rust::{ debug, error, hilog, HiLogLabel, LogType };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionDragClient"
};

/// struct DragClient
#[derive(Default)]
pub struct DragClient {
    dummy: i32
}

impl DragClient {
    /// TODO: add documentation.
    pub fn start_drag(&self, drag_data: &DragData, ipc_client: Rc<FusionIpcClient>) -> FusionResult<i32>
    {
        call_debug_enter!("DragClient::start_drag");
        match MsgParcel::new() {
            Some(mut reply_parcel) => {
                let mut borrowed_reply_parcel = reply_parcel.borrowed();
                debug!(LOG_LABEL, "call ipc_client::start()");
                ipc_client.start(Intention::Drag, drag_data, &mut borrowed_reply_parcel)?;

                match DefaultReply::deserialize(&borrowed_reply_parcel) {
                    Ok(x) => {
                        Ok(x.reply)
                    }
                    Err(_) => {
                        error!(LOG_LABEL, "Failed to deserialize DefaultReply");
                        Err(-1)
                    }
                }
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(-1)
            }
        }
    }
}
