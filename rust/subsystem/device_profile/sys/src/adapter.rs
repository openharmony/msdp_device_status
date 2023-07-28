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

use std::cell::RefCell;
use std::ffi::{ c_char, CStr, CString };
use std::rc::Rc;
use std::sync::Mutex;
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_data_rust::{ FusionResult };
use fusion_utils_rust::call_debug_enter;
use crate::binding::{
    CICrossStateListener,
    RegisterCrossStateListener,
};

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DeviceProfileAdapter",
};

struct CrossStateListener {
    interface: CICrossStateListener,
    callback: Rc<dyn Fn(&str, bool)>,
}

impl CrossStateListener {
    extern "C" fn clone(listener: *mut CICrossStateListener) -> *mut CICrossStateListener
    {
        let listener_ptr = listener as *mut Self;
        if let Some(listener_ref) = unsafe { listener_ptr.as_ref() } {
            let listener_box = Box::new(Self {
                interface: CICrossStateListener {
                    clone: Some(Self::clone),
                    destruct: Some(Self::destruct),
                    on_update: Some(Self::on_update),
                },
                callback: listener_ref.callback.clone(),
            });
            Box::into_raw(listener_box) as *mut CICrossStateListener
        } else {
            std::ptr::null_mut()
        }
    }

    extern "C" fn destruct(listener: *mut CICrossStateListener)
    {
        let listener_ptr = listener as *mut Self;
        if !listener_ptr.is_null() {
            unsafe { drop(Box::from_raw(listener_ptr)) };
        }
    }

    extern "C" fn on_update(listener: *mut CICrossStateListener, device_id: *const c_char, state: i32)
    {
        if device_id.is_null() {
            error!(LOG_LABEL, "Device id is null");
            return;
        }
        let listener_ptr = listener as *mut Self;
        unsafe {
            if let Some(listener_ref) = listener_ptr.as_ref() {
                if let Ok(id) = CStr::from_ptr(device_id).to_str() {
                    (listener_ref.callback)(id, state != 0);
                } else {
                    error!(LOG_LABEL, "Invalid device id");
                }
            } else {
                error!(LOG_LABEL, "Listener is null");
            }
        }
    }
}

impl<F> From<Rc<F>> for CrossStateListener
where
    F: Fn(&str, bool) + 'static {
    fn from(value: Rc<F>) -> Self {
        Self {
            interface: CICrossStateListener {
                clone: Some(Self::clone),
                destruct: Some(Self::destruct),
                on_update: Some(Self::on_update),
            },
            callback: value,
        }
    }
}

struct DeviceProfileAdapterImpl {
    dummy: i32,
}

impl DeviceProfileAdapterImpl {
    /// TODO: add documentation.
    pub fn update_cross_switch_state(&self, state: i32) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::update_cross_switch_state");
        Err(-1)
    }

    pub fn sync_cross_switch_state(&self, state: i32, device_ids: &[String]) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::sync_cross_switch_state");
        Err(-1)
    }

    pub fn get_cross_switch_state(&self, device_id: &str) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::get_cross_switch_state");
        Err(-1)
    }

    pub fn register_cross_state_listener<F>(&mut self, device_id: &str, callback: F) -> FusionResult<i32>
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
            error!(LOG_LABEL, "Fail to register cross state listener");
            Err(-1)
        } else {
            Ok(0)
        }
    }

    pub fn unregister_cross_state_listener(&self, device_id: &str) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::unregister_cross_state_listener");
        Err(-1)
    }
}

/// TODO: add documentation.
pub struct DeviceProfileAdapter {
    adapter_impl: Mutex<RefCell<DeviceProfileAdapterImpl>>,
}

impl DeviceProfileAdapter {
    /// TODO: add documentation.
    pub fn update_cross_switch_state(&self, state: i32) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::update_cross_switch_state");
        Err(-1)
    }

    /// TODO: add documentation.
    pub fn sync_cross_switch_state(&self, state: i32, device_ids: &[String]) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::sync_cross_switch_state");
        Err(-1)
    }

    /// TODO: add documentation.
    pub fn get_cross_switch_state(&self, device_id: &str) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::get_cross_switch_state");
        Err(-1)
    }

    /// TODO: add documentation.
    pub fn register_cross_state_listener<F>(&self, device_id: &str, callback: F) -> FusionResult<i32>
    where
        F: Fn(&str, bool) + 'static
    {
        call_debug_enter!("DeviceProfileAdapter::register_cross_state_listener");
        match self.adapter_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().register_cross_state_listener(device_id, callback)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {}", err);
                Err(-1)
            }
        }
    }

    /// TODO: add documentation.
    pub fn unregister_cross_state_listener(&self, device_id: &str) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfileAdapter::unregister_cross_state_listener");
        Err(-1)
    }
}
