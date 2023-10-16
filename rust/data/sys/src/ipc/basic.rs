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

//! IPC data definitions of BASIC module.

use std::ffi::{ c_char, CStr, CString };
use std::fmt::{ Display, Formatter, Error };

use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Serialize, Deserialize, IpcResult };

use fusion_utils_rust::{ call_debug_enter, define_enum, FusionResult, FusionErrorCode };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionBasicData"
};

define_enum! {
    BasicParamID {
        AllocSocketPair
    }
}

impl From<BasicParamID> for u32 {
    fn from(id: BasicParamID) -> Self
    {
        match id {
            BasicParamID::AllocSocketPair => { 0u32 },
        }
    }
}

/// Parameters for AllocSocketPair request.
pub struct AllocSocketPairParam {
    /// Represent program name of calling.
    pub program_name: String,
    /// Represent module type of calling.
    pub module_type: i32
}

impl AllocSocketPairParam {
    /// Construct AllocSocketPairParam from raw data.
    ///
    /// # Safety
    /// The 'program_name' must be some valid pointer to null-terminated string.
    ///
    pub unsafe fn from_c(program_name: *const c_char, module_type: i32) -> FusionResult<Self>
    {
        call_debug_enter!("AllocSocketPairParam::from_c");
        let cs = unsafe {
            CStr::from_ptr(program_name)
        };
        match cs.to_str() {
            Ok(sref) => {
                Ok(Self {
                    program_name: sref.to_string(),
                    module_type
                })
            }
            Err(_) => {
                error!(LOG_LABEL, "Can not convert \'program_name\' from CStr to String");
                Err(FusionErrorCode::Fail)
            }
        }
    }
}

impl Serialize for AllocSocketPairParam {
    fn serialize(&self, parcel: &mut BorrowedMsgParcel<'_>) -> IpcResult<()>
    {
        info!(LOG_LABEL, "serialize AllocSocketPairParam");
        self.program_name.serialize(parcel)?;
        self.module_type.serialize(parcel)?;
        Ok(())
    }
}

impl Deserialize for AllocSocketPairParam {
    fn deserialize(parcel: &BorrowedMsgParcel<'_>) -> IpcResult<Self>
    {
        let param = Self {
            program_name: String::deserialize(parcel)?,
            module_type: i32::deserialize(parcel)?,
        };
        Ok(param)
    }
}

impl Display for AllocSocketPairParam {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), Error>
    {
        writeln!(f, "\nAllocSocketPairParam {{")?;
        writeln!(f, "  program_name: {}", self.program_name)?;
        writeln!(f, "  module_type: {}", self.module_type)?;
        writeln!(f, "}}")?;
        Ok(())
    }
}
