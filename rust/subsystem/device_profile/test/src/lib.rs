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

mod binding;

use std::ffi::{ c_char, CString };
use std::sync::{ Arc, Condvar, Mutex };
use std::time::Duration;
use hilog_rust::{ debug, hilog, HiLogLabel, LogType };
use fusion_device_profile_rust::DeviceProfileAdapter;
use fusion_utils_rust::call_debug_enter;
use crate::binding::{ GetAccessToken, get_local_network_id };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DeviceProfileTest",
};

#[derive(Default)]
struct Waiter {
    data: Mutex<i32>,
    var: Condvar,
}

#[test]
fn test_get_cross_switch_state()
{
    call_debug_enter!("test_get_cross_switch_state");
    unsafe { GetAccessToken(); };
    let dpa = DeviceProfileAdapter::default();
    let device_id = get_local_network_id().unwrap();
    debug!(LOG_LABEL, "local network id: {}", @public(device_id));
    assert_eq!(dpa.get_cross_switch_state(&device_id), Ok(0));
}

#[test] 
fn test_update_cross_switch_state()
{
    call_debug_enter!("test_update_cross_switch_state");
    unsafe { GetAccessToken(); };
    let dpa = DeviceProfileAdapter::default();
    let device_id = get_local_network_id().unwrap();
    debug!(LOG_LABEL, "local network id: {}", @public(device_id));
    assert_eq!(dpa.update_cross_switch_state(true), Ok(0));
}

#[test]
fn test_register_cross_state_listener()
{
    call_debug_enter!("test_register_cross_state_listener");
    unsafe { GetAccessToken(); };
    let dpa = DeviceProfileAdapter::default();
    let waiter = Arc::new(Waiter::default());
    let waiter_cloned = waiter.clone();

    let device_id = get_local_network_id().unwrap();
    debug!(LOG_LABEL, "local network id: {}", @public(device_id));
    let ret = dpa.register_cross_state_listener(&device_id,
        move |_device_id_a, _state| {
            debug!(LOG_LABEL, "cross state update");
            waiter.var.notify_one();
        }
    );
    assert_eq!(ret, Ok(0));

    let mut guard = waiter_cloned.data.lock().unwrap();
    let result = waiter_cloned.var.wait_timeout(guard, Duration::from_secs(30)).unwrap();
    guard = result.0;
    if *guard == 1 {
        debug!(LOG_LABEL, "wait timeout");
    }
}

#[test]
fn test_sync_cross_switch_state()
{
    call_debug_enter!("test_sync_cross_switch_state");
    unsafe { GetAccessToken(); };
    let dpa = DeviceProfileAdapter::default();
    let mut network_id_b = String::new();
    std::io::stdin().read_line(&mut network_id_b).expect("failed to read line.");
    println!("{}", network_id_b);
    debug!(LOG_LABEL, "local network id: {}", @public(network_id_b));
    let vec = vec![network_id_b];
    assert_eq!(dpa.sync_cross_switch_state(true, &vec), Ok(0));
}

#[test]
fn test_unregister_cross_state_listener()
{
    call_debug_enter!("test_unregister_cross_state_listener");
    unsafe { GetAccessToken(); };
    let dpa = DeviceProfileAdapter::default();
    let device_id = get_local_network_id().unwrap();
    debug!(LOG_LABEL, "local network id: {}", @public(device_id));
    assert_eq!(dpa.unregister_cross_state_listener(&device_id), Ok(0));
}

#[test]
fn test_scene()
{
    call_debug_enter!("test_scene"); 
    unsafe { GetAccessToken(); };
    let dpa = DeviceProfileAdapter::default();
    let device_id = get_local_network_id().unwrap();
    debug!(LOG_LABEL, "local network id: {}", @public(device_id));

    let ret = dpa.register_cross_state_listener(&device_id,
        move |_device_id_a, _state| {
            debug!(LOG_LABEL, "cross state update");
        }
    );
    assert_eq!(ret, Ok(0));

    assert_eq!(dpa.unregister_cross_state_listener(&device_id), Ok(0));
}
