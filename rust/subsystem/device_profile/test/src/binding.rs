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

//! TODO: add documentation.

use std::ffi::{ c_char, CStr, CString };
use hilog_rust::{ error, hilog, HiLogLabel, LogType };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DeviceProfileTestBinding",
};

type CIStringClone = extern "C" fn (*mut CIString) -> *mut CIString;
type CIStringDestruct = extern "C" fn (*mut CIString);
type CIStringGetData = extern "C" fn (*mut CIString) -> *const c_char;

#[repr(C)]
pub struct CIString {
    pub clone: Option<CIStringClone>,
    pub destruct: Option<CIStringDestruct>,
    pub data: Option<CIStringGetData>,
}

struct CStringGuard {
    data: *mut CIString,
}

impl CStringGuard {
    pub unsafe fn data(&self) -> Option<String>
    {
        if self.data.is_null() {
            error!(LOG_LABEL, "data is null");
            None
        } else if let Some(get_data) = (*self.data).data {
            match CStr::from_ptr(get_data(self.data)).to_str() {
                Ok(id) => { Some(id.to_string()) }
                Err(err) => {
                    error!(LOG_LABEL, "error: {}", err);
                    None
                }
            }
        } else {
            error!(LOG_LABEL, "get_data is null");
            None
        }
    }
}

impl From<*mut CIString> for CStringGuard {
    fn from(value: *mut CIString) -> Self
    {
        Self { data: value }
    }
}

impl Drop for CStringGuard {
    fn drop(&mut self)
    {
        if self.data.is_null() {
            error!(LOG_LABEL, "data is null");
            return;
        }
        unsafe {
            if let Some(destruct) =  (*self.data).destruct {
                destruct(self.data);
            }
        }
    }
}

extern "C" {
    pub fn GetAccessToken();
    fn GetLocalNetworkId() -> *mut CIString;
}

pub fn get_local_network_id() -> Option<String>
{
    unsafe { CStringGuard::from(GetLocalNetworkId()).data() }
}
