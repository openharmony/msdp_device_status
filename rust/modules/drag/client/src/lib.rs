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

//! Proxy for `Drag` service.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, CString };
use fusion_data_rust::{ Intention, DefaultReply, DragData };
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use fusion_ipc_client_rust::FusionIpcClient;
use ipc_rust::{ MsgParcel, Deserialize };
use hilog_rust::{ debug, error, hilog, HiLogLabel, LogType };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionDragClient"
};

/// Definition of proxy for `Drag` service.
#[derive(Default)]
pub struct DragClient(i32);

impl DragClient {
    /// Request service to change to [`DRAG`] mode.
    pub fn start_drag(&self, drag_data: &DragData, ipc_client: &FusionIpcClient) -> FusionResult<i32>
    {
        call_debug_enter!("DragClient::start_drag");
        match MsgParcel::new() {
            Some(mut reply_parcel) => {
                let mut borrowed_reply_parcel = reply_parcel.borrowed();
                debug!(LOG_LABEL, "Call ipc_client::start()");
                ipc_client.start(Intention::Drag, drag_data, &mut borrowed_reply_parcel)?;

                match DefaultReply::deserialize(&borrowed_reply_parcel) {
                    Ok(x) => {
                        Ok(x.reply)
                    }
                    Err(_) => {
                        error!(LOG_LABEL, "Failed to deserialize DefaultReply");
                        Err(FusionErrorCode::Fail)
                    }
                }
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }
}
