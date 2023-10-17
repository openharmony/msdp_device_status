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

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, CStr, CString };
use std::rc::Rc;
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_utils_rust::{ call_debug_enter, FusionErrorCode, FusionResult };
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
    callback: Rc<dyn Fn(&str, bool)>,
}

impl CrossStateListener {
    extern "C" fn clone(listener: *mut CICrossStateListener) -> *mut CICrossStateListener
    {
        call_debug_enter!("CrossStateListener::clone");
        let listener_ptr = listener as *mut Self;
        debug_assert!(!listener_ptr.is_null());

        let listener_box = Box::new(Self {
            interface: CICrossStateListener {
                clone: Some(Self::clone),
                destruct: Some(Self::destruct),
                on_update: Some(Self::on_update),
            },
            callback: unsafe { (*listener_ptr).callback.clone() },
        });
        Box::into_raw(listener_box) as *mut CICrossStateListener
    }

    extern "C" fn destruct(listener: *mut CICrossStateListener)
    {
        call_debug_enter!("CrossStateListener::destruct");
        let listener_ptr = listener as *mut Self;
        debug_assert!(!listener_ptr.is_null());
        unsafe { drop(Box::from_raw(listener_ptr)) };
    }

    extern "C" fn on_update(listener: *mut CICrossStateListener, device_id: *const c_char, state: i32)
    {
        call_debug_enter!("CrossStateListener::destruct");
        let listener_ptr = listener as *mut Self;
        debug_assert!(!listener_ptr.is_null());
        debug_assert!(!device_id.is_null());
        unsafe {
            if let Ok(id) = CStr::from_ptr(device_id).to_str() {
                ((*listener_ptr).callback)(id, state != 0);
            } else {
                error!(LOG_LABEL, "Invalid device id");
            }
        }
    }
}

impl<F> From<Rc<F>> for CrossStateListener
where
    F: Fn(&str, bool) + 'static {
    fn from(value: Rc<F>) -> Self
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
    extern "C" fn clone(ss: *mut CIStringVector) -> *mut CIStringVector
    {
        let ss_ptr = ss as *mut Self;
        debug_assert!(!ss_ptr.is_null());

        let ss_box = Box::new(Self {
            interface: CIStringVector {
                clone: Some(Self::clone),
                destruct: Some(Self::destruct),
                at: Some(Self::at),
                size: Some(Self::size),
            },
            data: unsafe { (*ss_ptr).data.clone() },
        });
        Box::into_raw(ss_box) as *mut CIStringVector
    }

    extern "C" fn destruct(ss: *mut CIStringVector)
    {
        let ss_ptr = ss as *mut Self;
        debug_assert!(!ss_ptr.is_null());
        unsafe { drop(Box::from_raw(ss_ptr)) };
    }

    extern "C" fn at(ss: *mut CIStringVector, index: usize) -> *const c_char
    {
        let ss_ptr = ss as *mut Self;
        debug_assert!(!ss_ptr.is_null());
        unsafe {
            if index < (*ss_ptr).data.len() {
                (*ss_ptr).data[index].as_ptr() as *const c_char
            } else {
                std::ptr::null_mut()
            }
        }
    }

    extern "C" fn size(ss: *mut CIStringVector) -> usize
    {
        let ss_ptr = ss as *mut Self;
        debug_assert!(!ss_ptr.is_null());
        unsafe { (*ss_ptr).data.len() }
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

/// TODO: add documentation.
#[derive(Default)]
pub struct DeviceProfileAdapter {
    dummy: i32,
}

impl DeviceProfileAdapter {
    /// TODO: add documentation.
    pub fn update_cross_switch_state(&self, state: bool) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::update_cross_switch_state");
        let ret = unsafe { UpdateCrossSwitchState(state as i32) };
        if ret != 0 {
            error!(LOG_LABEL, "UpdateCrossSwitchState fail");
            Err(FusionErrorCode::Fail)
        } else {
            Ok(0)
        }
    }

    /// TODO: add documentation.
    pub fn sync_cross_switch_state(&self, state: bool, device_ids: &[String]) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::sync_cross_switch_state");
        let mut device_ids = StringVector::from(device_ids);
        let ret = unsafe {
            let device_ids_ptr: *mut StringVector = &mut device_ids;
            SyncCrossSwitchState(state as i32, device_ids_ptr as *mut CIStringVector)
        };
        if ret != 0 {
            error!(LOG_LABEL, "SyncCrossSwitchState fail");
            Err(FusionErrorCode::Fail)
        } else {
            Ok(0)
        }
    }

    /// TODO: add documentation.
    pub fn get_cross_switch_state(&self, device_id: &str) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::get_cross_switch_state");
        let ret = unsafe { GetCrossSwitchState(device_id.as_ptr()) };
        if ret == 0 {
            error!(LOG_LABEL, "GetCrossSwitchState failed");
            Err(FusionErrorCode::Fail)
        } else {
            Ok(0)
        }
    }

    /// TODO: add documentation.
    pub fn register_cross_state_listener<F>(&self, device_id: &str, callback: F) -> FusionResult<i32>
    where
        F: Fn(&str, bool) + 'static
    {
        call_debug_enter!("DeviceProfileAdapter::register_cross_state_listener");
        let mut listener = CrossStateListener::from(Rc::new(callback));
        let ret = unsafe {
            let listener_ptr: *mut CrossStateListener = &mut listener;
            RegisterCrossStateListener(device_id.as_ptr(), listener_ptr as *mut CICrossStateListener)
        };
        if ret != 0 {
            error!(LOG_LABEL, "RegisterCrossStateListener fail");
            Err(FusionErrorCode::Fail)
        } else {
            Ok(0)
        }
    }

    /// TODO: add documentation.
    pub fn unregister_cross_state_listener(&self, device_id: &str) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::unregister_cross_state_listener");
        let ret = unsafe { UnregisterCrossStateListener(device_id.as_ptr()) };
        if ret != 0 {
            error!(LOG_LABEL, "UnregisterCrossStateListener fail");
            Err(FusionErrorCode::Fail)
        } else {
            Ok(0)
        }
    }
}
