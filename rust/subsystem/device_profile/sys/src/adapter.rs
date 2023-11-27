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
    CIString,
    CIStringVector,
    UpdateCrossSwitchState,
    SyncCrossSwitchState,
    GetCrossSwitchState,
    RegisterCrossStateListener,
    UnregisterCrossStateListener,
    GetLocalNetworkId,
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
    /// This function performs a conversion of a raw pointer to a mutable reference of `Self` type.
    /// Please note that the pointer `listener` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    fn from_interface(listener: *mut CICrossStateListener) -> Option<&'static mut Self>
    {
        let listener_ptr = listener as *mut Self;
        // SAFETY: `listener_ptr` is valid, because `as_mut` has null pointer checking.
        unsafe {
            listener_ptr.as_mut()
        }
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
        if let Some(listener_mut) = CrossStateListener::from_interface(listener) {
            let listener_box = Box::new(Self {
                interface: CICrossStateListener {
                    clone: Some(Self::clone),
                    destruct: Some(Self::destruct),
                    on_update: Some(Self::on_update),
                },
                callback: listener_mut.callback.clone(),
            });
            Box::into_raw(listener_box) as *mut CICrossStateListener
        } else {
            error!(LOG_LABEL, "Failed to clone a CICrossStateListener instance");
            std::ptr::null_mut()
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
        if let Some(listener_mut) = CrossStateListener::from_interface(listener) {
            // SAFETY: `listener_mut` is valid, becauce has been matched to `Some`.
            unsafe { drop(Box::from_raw(listener_mut as *mut CrossStateListener)) };
        } else {
            error!(LOG_LABEL, "Failed to destruct a CICrossStateListener instance");
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
        call_debug_enter!("CrossStateListener::on_update");
        if let Some(listener_mut) = CrossStateListener::from_interface(listener) {
            // SAFETY: `listener_mut` is valid, cause has been performed check.
            // `device_id` and `state` are valid, which are ensured by the caller.
            unsafe {
                if let Ok(id) = CStr::from_ptr(device_id).to_str() {
                    (listener_mut.callback)(id, state != 0);
                } else {
                    error!(LOG_LABEL, "Invalid device id");
                }
            }
        } else {
            error!(LOG_LABEL, "Failed to handle a state update event from a device");
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
    /// This function performs a conversion of a raw pointer to a mutable reference of `Self` type.
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure that the returned pointer is not null before dereferencing it.
    fn from_interface(strings: *mut CIStringVector) -> Option<&'static mut Self>
    {
        let strings_ptr = strings as *mut Self;
        // SAFETY: `strings_ptr` is valid, because `as_mut` has null pointer checking.
        unsafe {
            strings_ptr.as_mut()
        }
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
        if let Some(strings_mut) = StringVector::from_interface(strings) {
            let strings_box = Box::new(Self {
                interface: CIStringVector {
                    clone: Some(Self::clone),
                    destruct: Some(Self::destruct),
                    get: Some(Self::get),
                    get_size: Some(Self::get_size),
                },
                data: strings_mut.data.clone(),
            });
            Box::into_raw(strings_box) as *mut CIStringVector
        } else {
            error!(LOG_LABEL, "Failed to clone a CIStringVector instance");
            std::ptr::null_mut()
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
        if let Some(strings_mut) = StringVector::from_interface(strings) {
            // SAFETY: `strings_mut` is valid, becauce has been matched to `Some`.
            unsafe { drop(Box::from_raw(strings_mut as *mut StringVector)) };
        } else {
            error!(LOG_LABEL, "Failed to destruct a CIStringVector instance");
        }
    }
    /// Accesse an element at a specific index in a `CIStringVector` instance.
    ///
    /// # Note
    ///
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn get(strings: *mut CIStringVector, index: usize) -> *const c_char
    {
        if let Some(strings_mut) = StringVector::from_interface(strings) {
            if index < strings_mut.data.len() {
                strings_mut.data[index].as_ptr() as *const c_char
            } else {
                error!(LOG_LABEL, "index is out of bounds");
                std::ptr::null()
            }
        } else {
            error!(LOG_LABEL, "Failed to accesse CIStringVector instance");
            std::ptr::null()
        }
    }
    /// Get the number of elements in a `CIStringVector` instance.
    ///
    /// # Note
    ///
    /// Please note that the pointer `strings` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn get_size(strings: *mut CIStringVector) -> usize
    {
        if let Some(strings_mut) = StringVector::from_interface(strings) {
            strings_mut.data.len()
        } else {
            error!(LOG_LABEL, "Failed to get the number of CIStringVector instance");
            EMPTY_LENGTH
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
                get: Some(Self::get),
                get_size: Some(Self::get_size),
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

struct CStringGuard {
    data: *mut CIString,
}

impl CStringGuard {
    /// Retrieves the string data from the CIString object.
    /// 
    /// Returns:
    /// - Some(String): If the data is successfully retrieved and converted to a string, it is wrapped in `Some`.
    /// - None: If the `data` or `get_data` pointer is null, or an error occurs during retrieval or conversion.
    pub fn data(&self) -> Option<String>
    {
        if self.data.is_null() {
            error!(LOG_LABEL, "data is null");
            return None;
        }
        // SAFETY: `data` is valid, because null pointer check has been performed.
        let data = unsafe { (*self.data).data };
        if let Some(data) = data {
            // SAFETY: `data` is valid, becauce has been matched to `Some`.
            let res = unsafe { CStr::from_ptr(data(self.data)).to_str() };
            res.ok().map(|id| id.to_string()).or_else(|| {
                error!(LOG_LABEL, "Failed to convert CStr to str");
                None
            })
        } else {
            error!(LOG_LABEL, "data is null");
            None
        }
    }
}

impl From<*mut CIString> for CStringGuard {
    /// Constructs a `CStringGuard` object and assigns the given raw pointer `value` to the `data` field.
    /// 
    /// # Note
    ///
    /// Please note that the pointer `value` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
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
        // SAFETY: `data` is valid, because null pointer check has been performed.
        unsafe {
            if let Some(destruct) = (*self.data).destruct {
                destruct(self.data);
            }
        }
    }
}

/// Retrieves the local network ID using FFI.
//
/// Returns:
/// - Some(String): If the local network ID is successfully retrieved, it is wrapped in `Some`.
/// - None: If the local network ID is not available or an error occurs during retrieval.
pub fn get_local_network_id() -> Option<String>
{
    // SAFETY: The `CStringGuard::from` and `data` methods are all valid, which are ensured by the caller.
    unsafe { CStringGuard::from(GetLocalNetworkId()).data() }
}
