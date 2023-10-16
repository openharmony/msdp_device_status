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

use crate::{ CDmDeviceInfo, CDmAuthForm };
use std::ffi::CStr;

/// DmDeviceInfo packed the native CDmDeviceInfo.
pub struct DmDeviceInfo(*const CDmDeviceInfo);

impl DmDeviceInfo {
    /// Create a PointerEvent object.
    pub fn new(device_info: *const CDmDeviceInfo) -> Self {
        Self(device_info)
    }

    /// Extract a raw `CDmDeviceInfo` pointer from this wrapper.
    /// 
    /// # Safety
    /// 
    /// Rust `DmDeviceInfo` always hold a valid native `CDmDeviceInfo`.
    pub unsafe fn as_inner(&self) -> *const CDmDeviceInfo {
        self.0
    }

    /// Create an `DmDeviceInfo` wrapper object from a raw `CDmDeviceInfo` pointer.
    pub fn from_raw(device_info: *const CDmDeviceInfo) -> Option<Self> {
        if device_info.is_null() {
            return None;
        }
        Some(Self(device_info))
    }
}

impl DmDeviceInfo {
    /// Get the device id attribute of CDmDeviceInfo.
    pub fn device_id(&self) -> String {
        // SAFETY:
        // Rust `DmDeviceInfo` always hold a valid native `CDmDeviceInfo`.
        let device_id = unsafe {CStr::from_ptr((*self.as_inner()).device_id)};
        device_id.to_string_lossy().into_owned()
    }

    /// Get the device name attribute of CDmDeviceInfo.
    pub fn device_name(&self) -> String {
        // SAFETY:
        // Rust `DmDeviceInfo` always hold a valid native `CDmDeviceInfo`.
        let device_name = unsafe {CStr::from_ptr((*self.as_inner()).device_name)};
        device_name.to_string_lossy().into_owned()
    }

    /// Get the device type id attribute of CDmDeviceInfo.
    pub fn device_type_id(&self) -> u16 {
        // SAFETY:
        // Rust `DmDeviceInfo` always hold a valid native `CDmDeviceInfo`.
        unsafe {
            (*self.as_inner()).device_type_id
        }
    }

    /// Get the network id attribute of CDmDeviceInfo.
    pub fn network_id(&self) -> String {
        // SAFETY:
        // Rust `DmDeviceInfo` always hold a valid native `CDmDeviceInfo`.
        let network_id = unsafe {CStr::from_ptr((*self.as_inner()).network_id)};
        network_id.to_string_lossy().into_owned()
    }

    /// Get the range attribute of CDmDeviceInfo.
    pub fn range(&self) -> i32 {
        // SAFETY:
        // Rust `DmDeviceInfo` always hold a valid native `CDmDeviceInfo`.
        unsafe {
            (*self.as_inner()).range
        }
    }

    /// Get the auth form attribute of CDmDeviceInfo.
    pub fn auth_form(&self) -> CDmAuthForm {
        // SAFETY:
        // Rust `DmDeviceInfo` always hold a valid native `CDmDeviceInfo`.
        unsafe {
            (*self.as_inner()).auth_form
        }
    }
}