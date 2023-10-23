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

//! rust adapter sys

#![allow(dead_code)]

use std::{ ffi::{ c_char, CStr, CString }, sync::Arc };
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_utils_rust::{ call_debug_enter, FusionErrorCode, FusionResult, err_log };

use crate::binding::{
    CICrossStateListener,
    CIStringVector,
    UpdateCrossSwitchState,
    SyncCrossSwitchState,
    GetCrossSwitchState,
    RegisterCrossStateListener,
    UnregisterCrossStateListener,
};

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DeviceProfileAdapter",
};

#[repr(C)]
struct CrossStateListener {
    interface: CICrossStateListener,
    callback: Arc<dyn Fn(&str, bool)>,
}

impl CrossStateListener {
    // Structures with C-compatible layouts that safely interconvert the first structure field and structure object.
    // Based on the C17 standard
    extern "C" fn from_interface(listener: *mut CICrossStateListener) -> *mut Self
    {
        listener as *mut Self
    }

    extern "C" fn clone(listener: *mut CICrossStateListener) -> *mut CICrossStateListener
    {
        call_debug_enter!("CrossStateListener::clone");
        let listener_ptr = CrossStateListener::from_interface(listener);
        if listener_ptr.is_null() {
            error!(LOG_LABEL, "listener_ptr is null");
        }
        debug_assert!(!listener_ptr.is_null());
        let listener_box = Box::new(Self {
            interface: CICrossStateListener {
                clone: Some(Self::clone),
                destruct: Some(Self::destruct),
                on_update: Some(Self::on_update),
            },
            // SAFETY: no `None` here, cause `listener_ptr` is valid.
            callback: unsafe { (*listener_ptr).callback.clone() },
        });
        Box::into_raw(listener_box) as *mut CICrossStateListener
    }

    extern "C" fn destruct(listener: *mut CICrossStateListener)
    {
        call_debug_enter!("CrossStateListener::destruct");
        let listener_ptr = CrossStateListener::from_interface(listener);
        if listener_ptr.is_null() {
            error!(LOG_LABEL, "listener_ptr is null");
        }
        debug_assert!(!listener_ptr.is_null());
        // SAFETY: no `None` here, cause `listener_ptr` is valid.
        unsafe { drop(Box::from_raw(listener_ptr)) };
    }

    extern "C" fn on_update(listener: *mut CICrossStateListener, device_id: *const c_char, state: i32)
    {
        call_debug_enter!("CrossStateListener::destruct");
        let listener_ptr = CrossStateListener::from_interface(listener);
        if listener_ptr.is_null() {
            error!(LOG_LABEL, "listener_ptr is null");
        }
        debug_assert!(!device_id.is_null());
        // SAFETY: no `None` here, cause `listener_ptr` is valid.
        unsafe {
            if let Ok(id) = CStr::from_ptr(device_id).to_str() {
                ((*listener_ptr).callback)(id, state != 0);
            } else {
                error!(LOG_LABEL, "Invalid device id");
            }
        }
    }
}

impl<F> From<Arc<F>> for CrossStateListener
where
    F: Fn(&str, bool) + 'static {
    fn from(value: Arc<F>) -> Self
    {
        Self {
            interface: CICrossStateListener {
                clone: Some(Self::clone),
                destruct: None,
                on_update: Some(Self::on_update),
            },
            callback: value,
        }
    }
}

#[repr(C)]
struct StringVector {
    interface: CIStringVector,
    data: Vec<String>,
}

impl StringVector {
    extern "C" fn from_interface(strings: *mut CIStringVector) -> *mut Self
    {
        strings as *mut Self
    }
    
    extern "C" fn clone(strings: *mut CIStringVector) -> *mut CIStringVector
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
        }
        debug_assert!(!strings_ptr.is_null());
        let strings_box = Box::new(Self {
            interface: CIStringVector {
                clone: Some(Self::clone),
                destruct: Some(Self::destruct),
                at: Some(Self::at),
                size: Some(Self::size),
            },
            // SAFETY: no `None` here, cause `strings_ptr` is valid.
            data: unsafe { (*strings_ptr).data.clone() },
        });
        Box::into_raw(strings_box) as *mut CIStringVector
    }

    extern "C" fn destruct(strings: *mut CIStringVector)
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
        }
        debug_assert!(!strings_ptr.is_null());
        // SAFETY: no `None` here, cause `strings_ptr` is valid.
        unsafe { drop(Box::from_raw(strings_ptr)) };
    }

    extern "C" fn at(strings: *mut CIStringVector, index: usize) -> *const c_char
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
        }
        debug_assert!(!strings_ptr.is_null());
        // SAFETY: no `None` here, cause `strings_ptr` is valid.
        unsafe {
            if index < (*strings_ptr).data.len() {
                (*strings_ptr).data[index].as_ptr() as *const c_char
            } else {
                std::ptr::null_mut()
            }
        }
    }

    extern "C" fn size(strings: *mut CIStringVector) -> usize
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
        }
        debug_assert!(!strings_ptr.is_null());
        // SAFETY: no `None` here, cause `strings_ptr` is valid.
        unsafe { (*strings_ptr).data.len() }
    }
}

impl From<&[String]> for StringVector {
    fn from(value: &[String]) -> Self
    {
        Self {
            interface: CIStringVector {
                clone: Some(StringVector::clone),
                destruct: None,
                at: Some(Self::at),
                size: Some(Self::size),
            },
            data: value.to_vec(),
        }
    }
}

/// Implementation of device profile adapter.
#[derive(Default)]
pub struct DeviceProfileAdapter {
    dummy: i32
}

impl DeviceProfileAdapter {
    fn check_return_code(&self, ret: i32) -> FusionResult<()>
    {
        if ret != 0 {
            Err(FusionErrorCode::Fail)
        } else {
            Ok(())
        }
    }

    /// Update the cross switch state with a boolean value.
    /// 
    /// # Arguments
    /// 
    /// * `state` - A boolean value that represents the cross switch state. `true` for on, `false` for off.
    /// 
    /// # Returns
    /// 
    /// Returns `FusionResult<()>` indicating whether the operation is successful. If the update succeeds,
    /// it returns `Ok(())`. If the update fails, it returns the corresponding error information.
    pub fn update_cross_switch_state(&self, state: bool) -> FusionResult<()>
    {
        call_debug_enter!("DeviceProfileAdapter::update_cross_switch_state");
        // SAFETY: no `None` here, cause `state` is valid.
        let ret = unsafe { UpdateCrossSwitchState(state as i32) };
        Ok(err_log!(self.check_return_code(ret), "UpdateCrossSwitchState"))
    }

    /// Synchronize the cross switch state between multiple devices.
    /// 
    /// # Arguments
    /// 
    /// * `state` - A boolean value that represents the cross switch state. `true` for on, `false` for off.
    /// * `device_ids` - An array of device ID strings representing the target devices to synchronize the state.
    /// 
    /// # Returns
    /// 
    /// Returns `FusionResult<()>` indicating whether the operation is successful. If the synchronization succeeds,
    /// it returns `Ok(())`. If the synchronization fails, it returns the corresponding error information.
    pub fn sync_cross_switch_state(&self, state: bool, device_ids: &[String]) -> FusionResult<()>
    {
        call_debug_enter!("DeviceProfileAdapter::sync_cross_switch_state");
        let mut device_ids = StringVector::from(device_ids);
        // SAFETY: no `None` here, cause `state` and `device_ids_ptr` is valid.
        let ret = unsafe {
            let device_ids_ptr: *mut StringVector = &mut device_ids;
            SyncCrossSwitchState(state as i32, device_ids_ptr as *mut CIStringVector)
        };
        Ok(err_log!(self.check_return_code(ret), "SyncCrossSwitchState"))
    }

    /// Get the cross switch state of a specific device.
    /// 
    /// # Arguments
    /// 
    /// * `device_id` - The device ID string representing the target device to get the cross switch state from.
    /// 
    /// # Returns
    /// 
    /// Returns `FusionResult<()>` indicating whether the operation is successful. If the getting succeeds,
    /// it returns `Ok(())`. If the getting fails, it returns the corresponding error information.
    pub fn get_cross_switch_state(&self, device_id: &str) -> FusionResult<()>
    {
        call_debug_enter!("DeviceProfileAdapter::get_cross_switch_state");
        // SAFETY: no `None` here, cause `device_id` is valid.
        let ret = unsafe { GetCrossSwitchState(device_id.as_ptr()) };
        Ok(err_log!(self.check_return_code(ret), "GetCrossSwitchState"))
    }

    /// Register callback for a specific device.
    /// 
    /// # Arguments
    /// 
    /// * `device_id` - The device ID string representing the target device to register the listener for.
    /// * `callback` - The callback function that will be invoked when the cross state changes. It takes two parameters:
    ///                the device ID and a boolean indicating the new cross state.
    /// 
    /// # Returns
    /// 
    /// Returns `FusionResult<()>` indicating whether the operation is successful. If the registration succeeds,
    /// it returns `Ok(())`. If the registration fails, it returns the corresponding error information.
    pub fn register_cross_state_listener<F>(&self, device_id: &str, callback: F) -> FusionResult<()>
    where
        F: Fn(&str, bool) + 'static
    {
        call_debug_enter!("DeviceProfileAdapter::register_cross_state_listener");
        let mut listener = CrossStateListener::from(Arc::new(callback));
        // SAFETY: no `None` here, cause `device_id` and `listener_ptr` is valid.
        let ret = unsafe {
            let listener_ptr: *mut CrossStateListener = &mut listener;
            RegisterCrossStateListener(device_id.as_ptr(), listener_ptr as *mut CICrossStateListener)
        };
        Ok(err_log!(self.check_return_code(ret), "RegisterCrossStateListener"))
    }

    /// Unregister callback for a specific device.
    /// 
    /// # Arguments
    /// 
    /// * `device_id` - The device ID string representing the target device to unregister the listener from.
    /// 
    /// # Returns
    /// 
    /// Returns `FusionResult<()>` indicating whether the operation is successful. If the unregistration succeeds,
    /// it returns `Ok(())`. If the unregistration fails, it returns the corresponding error information.
    pub fn unregister_cross_state_listener(&self, device_id: &str) -> FusionResult<()>
    {
        call_debug_enter!("DeviceProfileAdapter::unregister_cross_state_listener");
        // SAFETY: no `None` here, cause `device_id` is valid.
        let ret = unsafe { UnregisterCrossStateListener(device_id.as_ptr()) };
        Ok(err_log!(self.check_return_code(ret), "UnregisterCrossStateListener"))
    }
}
