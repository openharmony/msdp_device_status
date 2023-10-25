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
    RET_OK,
    EMPTY_LENGTH,
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
    /// Structures with C-compatible layouts that safely interconvert the first structure field and structure instance.
    /// Based on the C17 standard
    /// 
    /// # Note
    ///
    /// This function performs a conversion of the pointer type to `*mut Self` and returns it.
    /// Please note that the pointer `listener` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure that the returned pointer is not null before dereferencing it.
    fn from_interface(listener: *mut CICrossStateListener) -> *mut Self
    {
        listener as *mut Self
    }
    /// Clone a `CICrossStateListener` instance.
    ///
    /// # Note
    ///
    /// Please note that the pointer `listener` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn clone(listener: *mut CICrossStateListener) -> *mut CICrossStateListener
    {
        call_debug_enter!("CrossStateListener::clone");
        let listener_ptr = CrossStateListener::from_interface(listener);
        if listener_ptr.is_null() {
            error!(LOG_LABEL, "listener_ptr is null");
            std::ptr::null_mut()
        } else {
            let listener_box = Box::new(Self {
                interface: CICrossStateListener {
                    clone: Some(Self::clone),
                    destruct: Some(Self::destruct),
                    on_update: Some(Self::on_update),
                },
                // SAFETY: `listener_ptr` is valid, cause has been performed null pointer check.
                callback: unsafe { (*listener_ptr).callback.clone() },
            });
            Box::into_raw(listener_box) as *mut CICrossStateListener
        }
    }
    /// Destruct a `CICrossStateListener` instance.
    /// 
    /// # Note
    ///
    /// Please note that the pointer `listener` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn destruct(listener: *mut CICrossStateListener)
    {
        call_debug_enter!("CrossStateListener::destruct");
        let listener_ptr = CrossStateListener::from_interface(listener);
        if listener_ptr.is_null() {
            error!(LOG_LABEL, "listener_ptr is null");
        } else {
            // SAFETY: `listener_ptr` is valid, cause has been performed null pointer check.
            unsafe { drop(Box::from_raw(listener_ptr)) };
        }
    }
    /// Handle a state update event from a device.
    ///
    /// # Note
    ///
    /// Please note that the pointer `listener` and `device_id` are raw pointers that need to be handled carefully 
    /// to avoid memory safety issues and undefined behavior and ensure `state` is valid(0 for off, 1 for on).
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn on_update(listener: *mut CICrossStateListener, device_id: *const c_char, state: u32)
    {
        call_debug_enter!("CrossStateListener::destruct");
        let listener_ptr = CrossStateListener::from_interface(listener);
        if listener_ptr.is_null() {
            error!(LOG_LABEL, "listener_ptr is null");
        } else {
            // SAFETY: `listener_ptr` is valid, cause has been performed null pointer check.
            // `device_id` and `state` are valid, which are ensured by the caller.
            unsafe {
                if let Ok(id) = CStr::from_ptr(device_id).to_str() {
                    ((*listener_ptr).callback)(id, state != 0);
                } else {
                    error!(LOG_LABEL, "Invalid device id");
                }
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
    /// Structures with C-compatible layouts that safely interconvert the first structure field and structure instance.
    /// Based on the C17 standard
    /// 
    /// # Note
    ///
    /// This function performs a conversion of the pointer type to `*mut Self` and returns it.
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure that the returned pointer is not null before dereferencing it.
    fn from_interface(strings: *mut CIStringVector) -> *mut Self
    {
        strings as *mut Self
    }
    /// Clone a `CIStringVector` instance.
    ///
    /// # Note
    ///
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn clone(strings: *mut CIStringVector) -> *mut CIStringVector
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
            std::ptr::null_mut()
        } else {
            let strings_box = Box::new(Self {
                interface: CIStringVector {
                    clone: Some(Self::clone),
                    destruct: Some(Self::destruct),
                    at: Some(Self::at),
                    size: Some(Self::size),
                },
                // SAFETY: `strings_ptr` is valid, cause has been performed null pointer check.
                data: unsafe { (*strings_ptr).data.clone() },
            });
            Box::into_raw(strings_box) as *mut CIStringVector
        }
    }
    /// Destruct a `CIStringVector` instance.
    /// 
    /// # Note
    ///
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn destruct(strings: *mut CIStringVector)
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
        } else {
            // SAFETY: `strings_ptr` is valid, cause has been performed null pointer check.
            unsafe { drop(Box::from_raw(strings_ptr)) };
        }
    }
    /// Accesse an element at a specific index in a `CIStringVector` instance.
    ///
    /// # Note
    ///
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn at(strings: *mut CIStringVector, index: usize) -> *const c_char
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
            std::ptr::null_mut()
        } else {
            // SAFETY: `strings_ptr` is valid, cause has been performed null pointer check.
            unsafe {
                if index < (*strings_ptr).data.len() {
                    (*strings_ptr).data[index].as_ptr() as *const c_char
                } else {
                    std::ptr::null_mut()
                }
            }
        }
    }
    /// Get the number of elements in a `CIStringVector` instance.
    ///
    /// # Note
    ///
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn size(strings: *mut CIStringVector) -> usize
    {
        let strings_ptr = StringVector::from_interface(strings);
        if strings_ptr.is_null() {
            error!(LOG_LABEL, "strings_ptr is null");
            EMPTY_LENGTH
        } else {
            // SAFETY: `strings_ptr` is valid, cause has been performed null pointer check.
            unsafe { (*strings_ptr).data.len() }
        }
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
        (ret == RET_OK).then_some(()).ok_or(FusionErrorCode::Fail)
    }

    /// Update the cross switch state with a boolean value.
    /// 
    /// # Arguments
    /// 
    /// * `state` - A boolean value that represents the cross switch state. `true` for on, `false` for off.
    ///
    /// # Note
    ///
    /// This function assumes `state` parameter is a valid boolean value (`true` or `false`), 
    /// The caller needs to ensure that `state` is valid.
    /// 
    /// # Returns
    /// 
    /// Returns `FusionResult<()>` indicating whether the operation is successful. If the update succeeds,
    /// it returns `Ok(())`. If the update fails, it returns the corresponding error information.

    pub fn update_cross_switch_state(&self, state: bool) -> FusionResult<()>
    {
        call_debug_enter!("DeviceProfileAdapter::update_cross_switch_state");
        // SAFETY: `state` is valid, which is ensured by the caller.
        let ret = unsafe { UpdateCrossSwitchState(state as u32) };
        Ok(err_log!(self.check_return_code(ret), "UpdateCrossSwitchState"))
    }

    /// Synchronize the cross switch state between multiple devices.
    /// 
    /// # Arguments
    /// 
    /// * `state` - A boolean value that represents the cross switch state. `true` for on, `false` for off.
    /// * `device_ids` - A slice of strings representing the device IDs to sync with.
    /// 
    /// # Note
    ///
    /// This function assumes the validity of `state` (`true` or `false`) and `device_ids` parameters.
    /// The caller needs to ensure that `state` is valid and `device_ids` is a valid and properly initialized 
    /// slice of strings.
    /// 
    /// # Returns
    /// 
    /// Returns `FusionResult<()>` indicating whether the operation is successful. If the synchronization succeeds,
    /// it returns `Ok(())`. If the synchronization fails, it returns the corresponding error information.
    pub fn sync_cross_switch_state(&self, state: bool, device_ids: &[String]) -> FusionResult<()>
    {
        call_debug_enter!("DeviceProfileAdapter::sync_cross_switch_state");
        let mut device_ids = StringVector::from(device_ids);
        // SAFETY: `state` and `device_ids` are valid, which are ensured by the caller.
        let ret = unsafe {
            let device_ids_ptr: *mut StringVector = &mut device_ids;
            SyncCrossSwitchState(state as u32, device_ids_ptr as *mut CIStringVector)
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
        // SAFETY: `device_id` is valid, which is ensured by the caller.
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
        // SAFETY: `device_id` and `listener_ptr` are valid, which are ensured by the caller.
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
        // SAFETY: `device_id` is valid, which is ensured by the caller.
        let ret = unsafe { UnregisterCrossStateListener(device_id.as_ptr()) };
        Ok(err_log!(self.check_return_code(ret), "UnregisterCrossStateListener"))
    }
}
