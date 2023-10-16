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

//! Proxy for general functionality of service.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, CString };

use hilog_rust::{ debug, hilog, HiLogLabel, LogType };
use ipc_rust::{ FileDesc, MsgParcel, Deserialize };

use fusion_data_rust::{ Intention, AllocSocketPairParam, BasicParamID};
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use fusion_ipc_client_rust::FusionIpcClient;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionBasicClient"
};

/// Definition of proxy for general functionality of service.
#[derive(Default)]
pub struct FusionBasicClient(i32);

impl FusionBasicClient {
    /// Request connection of service via socket.
    pub fn alloc_socket_pair(&self, param: &AllocSocketPairParam,
                             ipc_client: &FusionIpcClient) -> FusionResult<(FileDesc, i32)>
    {
        call_debug_enter!("FusionBasicClient::alloc_socket_pair");
        let mut reply_parcel = MsgParcel::new().ok_or(FusionErrorCode::Fail)?;
        let mut borrowed_reply_parcel = reply_parcel.borrowed();

        debug!(LOG_LABEL, "Call ipc_client::start()");
        ipc_client.control(Intention::Basic, u32::from(BasicParamID::AllocSocketPair),
            param, &mut borrowed_reply_parcel)?;

        let fdesc = FileDesc::deserialize(&borrowed_reply_parcel).or(Err(FusionErrorCode::Fail))?;
        let token_type = i32::deserialize(&borrowed_reply_parcel).or(Err(FusionErrorCode::Fail))?;
        Ok((fdesc, token_type))
    }
}
