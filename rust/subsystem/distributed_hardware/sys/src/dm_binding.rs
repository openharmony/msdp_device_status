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

//! rust distributed hardware sys.

#![allow(dead_code)]

use std::ffi::{ c_char, CString };
use crate::fusion_utils_rust::{ call_info_trace };
use crate::DmDeviceInfo;
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "RustDmBinding"
};

/// enum CDmAuthForm
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub enum CDmAuthForm {
    /// Invalid type.
    INVALIDTYPE = -1,
    /// Peer to peer.
    PEERTOPEER = 0,
    /// Identical account.
    IDENTICALACCOUNT = 1,
    /// Across account.
    ACROSSACCOUNT = 2,
}

/// struct CDmDeviceInfo
#[repr(C)]
pub struct CDmDeviceInfo {
    /// The device id property of the `CDmDeviceInfo`.
    pub device_id: *const c_char,
    /// The device name property of the `CDmDeviceInfo`.
    pub device_name: *const c_char,
    /// The device type id property of the `CDmDeviceInfo`.
    pub device_type_id: u16,
    /// The network id property of the `CDmDeviceInfo`.
    pub network_id: *const c_char,
    /// The range property of the `CDmDeviceInfo`.
    pub range: i32,
    /// The auth form property of the `CDmDeviceInfo`.
    pub auth_form: CDmAuthForm,
}

/// struct CRegisterDevStateCallback
#[repr(C)]
pub struct CRegisterDevStateCallback {
    /// The `on_device_online` form property of the `CRegisterDevStateCallback`.
    pub on_device_online: OnRegisterDevState,
    /// The `on_device_changed` form property of the `CRegisterDevStateCallback`.
    pub on_device_changed: OnRegisterDevState,
    /// The `on_device_ready` form property of the `CRegisterDevStateCallback`.
    pub on_device_ready: OnRegisterDevState,
    /// The `on_device_offline` form property of the `CRegisterDevStateCallback`.
    pub on_device_offline: OnRegisterDevState,
}

// Callback function type for OnDeviceInit() from native, this
// callback will be called when device manager init.
pub type OnDeviceInit = unsafe extern "C" fn ();

/// Callback when the remote died.
///
/// # Safety
///
/// The function pointer passed to the c-side is legal.
pub unsafe extern "C" fn on_remote_died() {
    call_info_trace!("dm_binding::on_remote_died");
}

// Callback function type for OnRegisterDevState() from native, this
// callback will be called when register device state.
pub type OnRegisterDevState = unsafe extern "C" fn (
    device_info: *const CDmDeviceInfo
);

/// Callback when the device online.
///
/// # Safety
///
/// The function pointer passed to the c-side is legal.
pub unsafe extern "C" fn on_device_online(device_info_ptr: *const CDmDeviceInfo) {
    call_info_trace!("dm_binding::on_device_online");
    let _device_info = match DmDeviceInfo::from_raw(device_info_ptr) {
        Some(dev_info) => dev_info,
        None => {
            error!(LOG_LABEL, "Crate DmDeviceInfo failed");
            return;
        }
    };
}

/// Callback when the device changed.
///
/// # Safety
///
/// The function pointer passed to the c-side is legal.
pub unsafe extern "C" fn on_device_changed(_device_info_ptr: *const CDmDeviceInfo) {
    call_info_trace!("dm_binding::on_device_changed");
}

/// Callback when the device ready.
///
/// # Safety
///
/// The function pointer passed to the c-side is legal.
pub unsafe extern "C" fn on_device_ready(_device_info_ptr: *const CDmDeviceInfo) {
    call_info_trace!("dm_binding::on_device_ready");
}

/// Callback when the device offline.
///
/// # Safety
///
/// The function pointer passed to the c-side is legal.
pub unsafe extern "C" fn on_device_offline(_device_info_ptr: *const CDmDeviceInfo) {
    call_info_trace!("dm_binding::on_device_offline");
}

extern "C" {
    pub fn CInitDeviceManager(pkg_name: *const c_char, on_device_init: OnDeviceInit) -> bool;
    pub fn CRegisterDevState(
        pkg_name: *const c_char,
        extra: *const c_char,
        on_register_dev_state: CRegisterDevStateCallback,
    ) -> bool;
}