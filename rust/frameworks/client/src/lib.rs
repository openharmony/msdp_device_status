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

//! fusion IPC client.

#![allow(dead_code)]
#![allow(unused_variables)]

extern crate hilog_rust;
extern crate ipc_rust;
extern crate fusion_utils_rust;
extern crate fusion_ipc_client_rust;
extern crate fusion_data_rust;
extern crate fusion_basic_client_rust;
extern crate fusion_drag_client_rust;
extern crate fusion_coordination_client_rust;

mod frameworks;

use std::ffi::{ c_char, CStr, CString };
use std::os::fd::AsRawFd;
use hilog_rust::{ error, info, hilog, HiLogLabel, LogType };
use fusion_utils_rust::call_debug_enter;
use fusion_data_rust::{ AllocSocketPairParam, CDragData, DragData };
use frameworks::FusionFrameworks;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "fusion_client"
};

/// fusion_start_drag()
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_alloc_socket_fd(program_name: *const c_char, module_type: i32,
    client_fd: *mut i32, token_type: *mut i32) -> i32
{
    call_debug_enter!("fusion_alloc_socket_fd");
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
        }
        None => {
            error!(LOG_LABEL, "Fail dereferencing mutable FusionFrameworks instance");
            return -1;
        }
    }

    let param_result = unsafe {
        if program_name.is_null() {
            error!(LOG_LABEL, "program_name is null");
            return -1;
        } else {
            AllocSocketPairParam::from_c(program_name, module_type)
        }
    };
    let param = match param_result {
        Ok(param) => {
            param
        }
        Err(err) => {
            error!(LOG_LABEL, "Fail parsing AllocSocketPairParam");
            return err;
        }
    };
    match FusionFrameworks::get_instance() {
        Some(fw) => {
            info!(LOG_LABEL, "Call alloc_socket_pair()");
            match fw.alloc_socket_pair(&param) {
                Ok((fdesc, t)) => {
                    *client_fd = fdesc.as_raw_fd();
                    *token_type = t;
                    0
                }
                Err(_) => {
                    error!(LOG_LABEL, "alloc_socket_pair() fail");
                    -1
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Fail dereferencing FusionFrameworks instance");
            -1
        }
    }
}

/// fusion_start_drag()
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_start_drag(c_drag_data: *mut CDragData) -> i32
{
    c_drag_data.as_mut().map_or(-1, |c_drag_data_ref| {
        info!(LOG_LABEL, "enter fusion_start_drag()");
        match FusionFrameworks::get_mut_instance() {
            Some(fw_mut) => {
                fw_mut.set_ipc_connect();
            }
            None => {
                error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
                return -1;
            }
        }

        let drag_data = DragData::from_c(c_drag_data_ref);
        match FusionFrameworks::get_instance() {
            Some(fw) => {
                info!(LOG_LABEL, "Call start_drag()");
                match fw.start_drag(&drag_data) {
                    Ok(_) => {
                        0
                    }
                    Err(err) => {
                        error!(LOG_LABEL, "Error happened when starting drag");
                        err
                    }
                }
            }
            None => {
                error!(LOG_LABEL, "Can not dereference FusionFrameworks instance");
                -1
            }
        }
    })
}

/// TODO: add documentation.
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_register_coordination_listener() -> i32
{
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
            match fw_mut.register_coordination_listener() {
                Ok(_) => {
                    0
                }
                Err(err) => {
                    error!(LOG_LABEL, "Fail to register coordination listener: {}", @public(err));
                    err
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
            -1
        }
    }
}

/// TODO: add documentation.
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_unregister_coordination_listener() -> i32
{
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
            match fw_mut.unregister_coordination_listener() {
                Ok(_) => {
                    0
                }
                Err(err) => {
                    error!(LOG_LABEL, "Fail to unregister coordination listener: {}", @public(err));
                    err
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
            -1
        }
    }
}

/// TODO: add documentation.
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_enable_coordination(user_data: i32) -> i32
{
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
            match fw_mut.enable_coordination(user_data) {
                Ok(_) => {
                    0
                }
                Err(err) => {
                    error!(LOG_LABEL, "Error in enable coordination");
                    err
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
            -1
        }
    }
}

/// TODO: add documentation.
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_disable_coordination(user_data: i32) -> i32
{
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
            match fw_mut.disable_coordination(user_data) {
                Ok(_) => {
                    0
                }
                Err(err) => {
                    error!(LOG_LABEL, "Error in enable coordination");
                    err
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
            -1
        }
    }
}

/// TODO: add documentation.
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_start_coordination(user_data: i32,
    remote_network_id: *const c_char, start_device_id: i32) -> i32
{
    if remote_network_id.is_null() {
        error!(LOG_LABEL, "remote_network_id is null");
        return -1;
    }
    let remote_network_id: String = match CStr::from_ptr(remote_network_id).to_str() {
        Ok(id) => {
            id.to_string()
        }
        Err(_) => {
            error!(LOG_LABEL, "Invalid network id");
            return -1;
        }
    };
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
            match fw_mut.start_coordination(user_data, remote_network_id, start_device_id) {
                Ok(_) => {
                    0
                }
                Err(err) => {
                    error!(LOG_LABEL, "Error happened when starting coordination");
                    err
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
            -1
        }
    }
}

/// TODO: add documentation.
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_stop_coordination(user_data: i32, is_unchained: i32) -> i32
{
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
            match fw_mut.stop_coordination(user_data, is_unchained) {
                Ok(_) => {
                    0
                }
                Err(err) => {
                    error!(LOG_LABEL, "Fail to stop coordination");
                    err
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
            -1
        }
    }
}

/// TODO: add documentation.
/// # Safety
#[no_mangle]
pub unsafe extern "C" fn fusion_get_coordination_state(user_data: i32, device_id: *const c_char) -> i32
{
    if device_id.is_null() {
        error!(LOG_LABEL, "device_id is null");
        return -1;
    }
    let device_id: String = match CStr::from_ptr(device_id).to_str() {
        Ok(id) => {
            id.to_string()
        }
        Err(_) => {
            error!(LOG_LABEL, "Invalid device id");
            return -1;
        }
    };
    match FusionFrameworks::get_mut_instance() {
        Some(fw_mut) => {
            fw_mut.set_ipc_connect();
            match fw_mut.get_coordination_state(user_data, device_id) {
                Ok(_) => {
                    0
                }
                Err(err) => {
                    error!(LOG_LABEL, "Fail to get coordination state: {}", @public(err));
                    err
                }
            }
        }
        None => {
            error!(LOG_LABEL, "Can not dereference mutable FusionFrameworks instance");
            -1
        }
    }
}
