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

//! IPC data definitions of Coordination module.

use std::ffi::{ c_char, CString };

use hilog_rust::{ hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Serialize, Deserialize, IpcResult };

use fusion_utils_rust::call_debug_enter;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionCoordinationData"
};

/// General parameters of request of multi-device cooperation.
#[derive(Default)]
pub struct GeneralCoordinationParam {
    /// ID of one request.
    pub user_data: i32,
}

impl Serialize for GeneralCoordinationParam {
    fn serialize(&self, parcel: &mut BorrowedMsgParcel<'_>) -> IpcResult<()>
    {
        call_debug_enter!("GeneralCoordinationParam::serialize");
        self.user_data.serialize(parcel)?;
        Ok(())
    }
}

impl Deserialize for GeneralCoordinationParam {
    fn deserialize(parcel: &BorrowedMsgParcel<'_>) -> IpcResult<Self>
    {
        call_debug_enter!("GeneralCoordinationParam::deserialize");
        Ok(Self {
            user_data: i32::deserialize(parcel)?,
        })
    }
}

/// Parameters of request to start multi-device cooperation.
pub struct StartCoordinationParam {
    /// ID of one request.
    pub user_data: i32,
    /// The input device that trigger multi-device cooperation.
    pub start_device_id: i32,
    /// The remote device we want to cooperate with.
    pub remote_network_id: String,
}

impl Serialize for StartCoordinationParam {
    fn serialize(&self, parcel: &mut BorrowedMsgParcel<'_>) -> IpcResult<()>
    {
        call_debug_enter!("StartCoordinationParam::serialize");
        self.user_data.serialize(parcel)?;
        self.start_device_id.serialize(parcel)?;
        self.remote_network_id.serialize(parcel)?;
        Ok(())
    }
}

impl Deserialize for StartCoordinationParam {
    fn deserialize(parcel: &BorrowedMsgParcel<'_>) -> IpcResult<Self>
    {
        call_debug_enter!("StartCoordinationParam::deserialize");
        Ok(Self {
            user_data: i32::deserialize(parcel)?,
            start_device_id: i32::deserialize(parcel)?,
            remote_network_id: String::deserialize(parcel)?,
        })
    }
}

/// Parameters of request to stop multi-device cooperation.
pub struct StopCoordinationParam {
    /// ID of one request.
    pub user_data: i32,
    /// Indicate whether to stop remote input.
    pub is_unchained: i32,
}

impl Serialize for StopCoordinationParam {
    fn serialize(&self, parcel: &mut BorrowedMsgParcel<'_>) -> IpcResult<()>
    {
        call_debug_enter!("StopCoordinationParam::serialize");
        self.user_data.serialize(parcel)?;
        self.is_unchained.serialize(parcel)?;
        Ok(())
    }
}

impl Deserialize for StopCoordinationParam {
    fn deserialize(parcel: &BorrowedMsgParcel<'_>) -> IpcResult<Self>
    {
        call_debug_enter!("StopCoordinationParam::deserialize");
        Ok(Self {
            user_data: i32::deserialize(parcel)?,
            is_unchained: i32::deserialize(parcel)?,
        })
    }
}

/// Parameters of request to check switch status of multi-device cooperation
/// of the remote device.
pub struct GetCoordinationStateParam {
    /// ID of one request.
    pub user_data: i32,
    /// The remote device.
    pub device_id: String,
}

impl Serialize for GetCoordinationStateParam {
    fn serialize(&self, parcel: &mut BorrowedMsgParcel<'_>) -> IpcResult<()>
    {
        call_debug_enter!("GetCoordinationStateParam::serialize");
        self.user_data.serialize(parcel)?;
        self.device_id.serialize(parcel)?;
        Ok(())
    }
}

impl Deserialize for GetCoordinationStateParam {
    fn deserialize(parcel: &BorrowedMsgParcel<'_>) -> IpcResult<Self>
    {
        call_debug_enter!("GetCoordinationStateParam::deserialize");
        Ok(Self {
            user_data: i32::deserialize(parcel)?,
            device_id: String::deserialize(parcel)?,
        })
    }
}
