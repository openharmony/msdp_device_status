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

use hilog_rust::{ error, hilog, HiLogLabel, LogType };

use fusion_utils_rust::call_info_trace;

use crate::DmDeviceInfo;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "RustDmBinding"
};

/// C representation of [`DmAuthForm`].
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

/// C representation of [`DmDeviceInfo`].
#[repr(C)]
pub struct CDmDeviceInfo {
    /// Device Id of the device.
    pub device_id: *const c_char,
    /// Device name of the device.
    pub device_name: *const c_char,
    /// Device type of the device.
    pub device_type_id: u16,
    /// NetworkId of the device.
    pub network_id: *const c_char,
    /// The distance of discovered device.
    pub range: i32,
    /// Device authentication form.
    pub auth_form: CDmAuthForm,
}

/// Used to hold callbacks.
#[repr(C)]
pub struct CRegisterDevStateCallback {
    /// Device online callback of the device.
    pub on_device_online: OnRegisterDevState,
    /// Device changed callback of the device.
    pub on_device_changed: OnRegisterDevState,
    /// Device ready callback of the device.
    pub on_device_ready: OnRegisterDevState,
    /// Device offline callback of the device.
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

    CDestroyDmDeviceInfo(device_info_ptr);
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
    pub fn CUnRegisterDevState(pkg_name: *const c_char, extra: *const c_char) -> bool;
    pub fn CDestroyDmDeviceInfo(device_info: *const CDmDeviceInfo);
}