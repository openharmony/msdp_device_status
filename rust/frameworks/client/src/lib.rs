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

mod frameworks;

use std::ffi::{ c_char, CStr, CString };
use std::os::fd::AsRawFd;
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_utils_rust::call_debug_enter;
use fusion_data_rust::{ AllocSocketPairParam, CDragData, DragData };
use frameworks::FusionFrameworks;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "fusion_client"
};

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_alloc_socket_fd(program_name: *const c_char, module_type: i32,
    client_fd: *mut i32, token_type: *mut i32) -> i32
{
    call_debug_enter!("fusion_alloc_socket_fd");
    if program_name.is_null() {
        error!(LOG_LABEL, "program_name is null");
        return -1;
    };
    let param = match AllocSocketPairParam::from_c(program_name, module_type) {
        Ok(param) => { param }
        Err(err) => {
            error!(LOG_LABEL, "Failed parsing AllocSocketPairParam");
            return i32::from(err);
        }
    };
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.alloc_socket_pair(&param).map_or_else(
        |err| { i32::from(err) },
        |(fdesc, t)| {
            *client_fd = fdesc.as_raw_fd();
            *token_type = t;
            0
        }
    )
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_start_drag(c_drag_data: *mut CDragData) -> i32
{
    c_drag_data.as_mut().map_or(-1, |c_drag_data_ref| {
        let drag_data = DragData::from_c(c_drag_data_ref);
        let fw = FusionFrameworks::get_instance().unwrap();
        fw.start_drag(&drag_data).map_or_else(|err| { i32::from(err) }, |_| { 0 })
    })
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_register_coordination_listener() -> i32
{
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.register_coordination_listener().map_or_else(|err| { i32::from(err) }, |_| { 0 })
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_unregister_coordination_listener() -> i32
{
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.unregister_coordination_listener().map_or_else(|err| { i32::from(err) }, |_| { 0 })
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_enable_coordination(user_data: i32) -> i32
{
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.enable_coordination(user_data).map_or_else(|err| { i32::from(err) }, |_| { 0 })
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_disable_coordination(user_data: i32) -> i32
{
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.disable_coordination(user_data).map_or_else(|err| { i32::from(err) }, |_| { 0 })
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_start_coordination(user_data: i32,
    remote_network_id: *const c_char, start_device_id: i32) -> i32
{
    if remote_network_id.is_null() {
        error!(LOG_LABEL, "remote_network_id is null");
        return -1;
    }
    let remote_network_id: String = match CStr::from_ptr(remote_network_id).to_str() {
        Ok(id) => { id.to_string() }
        Err(_) => {
            error!(LOG_LABEL, "Invalid network id");
            return -1;
        }
    };
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.start_coordination(user_data, &remote_network_id,
        start_device_id).map_or_else(|err| { i32::from(err) }, |_| { 0 })
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_stop_coordination(user_data: i32, is_unchained: i32) -> i32
{
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.stop_coordination(user_data, is_unchained).map_or_else(|err| { i32::from(err) }, |_| { 0 })
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn fusion_get_coordination_state(user_data: i32, device_id: *const c_char) -> i32
{
    if device_id.is_null() {
        error!(LOG_LABEL, "device_id is null");
        return -1;
    }
    let device_id: String = match CStr::from_ptr(device_id).to_str() {
        Ok(id) => { id.to_string() }
        Err(_) => {
            error!(LOG_LABEL, "Invalid device id");
            return -1;
        }
    };
    let fw = FusionFrameworks::get_instance().unwrap();
    fw.get_coordination_state(user_data, &device_id).map_or_else(|err| { i32::from(err) }, |_| { 0 })
}
