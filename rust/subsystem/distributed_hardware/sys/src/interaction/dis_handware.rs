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

#![allow(dead_code)]

use std::ffi::{ c_char, CString };
use crate::fusion_utils_rust::{ call_info_trace };
use fusion_data_rust::FusionResult;
use crate::dm_binding;
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "RustDisHandware"
};
static FI_PKG_NAME: &str = "ohos.msdp.fusioninteraction";

/// struct DisHandware
pub struct DisHandware;

impl DisHandware {
    /// Init device manager.
    pub fn init_device_manager() -> FusionResult<i32> {
        call_info_trace!("DisHandware::init_device_manager");
        let pkg_name = match CString::new(FI_PKG_NAME)?;
        // SAFETY: no `None` here, cause `callback` and  `pkg_name` is valid.
        unsafe {
            if !dm_binding::CInitDeviceManager(pkg_name.as_ptr(), dm_binding::on_remote_died) {
                error!(LOG_LABEL, "Init device manager failed");
                return Err(-1);
            }
            Ok(0)
        }
    }

    /// Register device state
    pub fn register_device_state() -> FusionResult<i32> {
        call_info_trace!("DisHandware::register_device_state");
        let pkg_name = match CString::new(FI_PKG_NAME)?;
        let extra = match CString::new("")?;
        let callbacks = dm_binding::CRegisterDevStateCallback {
            on_device_online: dm_binding::on_device_online,
            on_device_changed: dm_binding::on_device_changed,
            on_device_ready: dm_binding::on_device_ready,
            on_device_offline: dm_binding::on_device_offline,
        };
        // SAFETY: no `None` here, cause `callback` and `pkg_name` and `extra` is valid.
        unsafe {
            if !dm_binding::CRegisterDevState(pkg_name.as_ptr(), extra.as_ptr(), callbacks) {
                error!(LOG_LABEL, "Register devStateCallback failed");
                return Err(-1);
            }
            Ok(0)
        }
    }

    /// UnRegister device state
    pub fn un_register_device_state() -> FusionResult<i32> {
        call_info_trace!("DisHandware::un_register_device_state");
        let pkg_name = match CString::new(FI_PKG_NAME)?;
        let extra = match CString::new("")?;
        // SAFETY: no `None` here, cause `pkg_name` and `extra` is valid.
        if !dm_binding::CUnRegisterDevState(pkg_name.as_ptr(), extra.as_ptr()) {
            error!(LOG_LABEL, "UnRegister devStateCallback failed");
            return Err(-1);
        }
        Ok(0)
    }
}