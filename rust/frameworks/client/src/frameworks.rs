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

//! Implementation of FusionFrameworks.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::sync::{ Mutex, Once };
use std::ffi::{ c_char, CString };

use hilog_rust::{ error, info, hilog, HiLogLabel, LogType };
use ipc_rust::FileDesc;
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use fusion_ipc_client_rust::FusionIpcClient;
use fusion_data_rust::{ AllocSocketPairParam, DragData };
use fusion_basic_client_rust::FusionBasicClient;
use fusion_drag_client_rust::DragClient;
use fusion_coordination_client_rust::FusionCoordinationClient;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionFrameworks"
};

#[derive(Default)]
struct Client {
    ipc_client: Option<FusionIpcClient>,
    basic: FusionBasicClient,
    drag: DragClient,
    coordination: FusionCoordinationClient,
}

impl Client {
    /// Connect service.
    fn connect(&mut self)
    {
        if self.ipc_client.is_some() {
            return;
        }
        info!(LOG_LABEL, "Trying to connect server");
        match FusionIpcClient::connect() {
            Ok(client) => {
                info!(LOG_LABEL, "Connect to server successfully");
                self.ipc_client = Some(client);
            }
            Err(_) => {
                error!(LOG_LABEL, "Can not connect to server");
            }
        }
    }

    fn alloc_socket_pair(&self, param: &AllocSocketPairParam) -> FusionResult<(FileDesc, i32)>
    {
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                info!(LOG_LABEL, "Call basic.start_drag()");
                self.basic.alloc_socket_pair(param, ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn start_drag(&self, drag_data: &DragData) -> FusionResult<i32>
    {
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                info!(LOG_LABEL, "Call drag.start_drag()");
                self.drag.start_drag(drag_data, ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn register_coordination_listener(&self) -> FusionResult<()>
    {
        call_debug_enter!("FusionFrameworks::register_coordination_listener");
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                self.coordination.register_coordination_listener(ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn unregister_coordination_listener(&self) -> FusionResult<()>
    {
        call_debug_enter!("FusionFrameworks::unregister_coordination_listener");
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                self.coordination.unregister_coordination_listener(ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn enable_coordination(&self, user_data: i32) -> FusionResult<()>
    {
        call_debug_enter!("FusionFrameworks::enable_coordination");
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                self.coordination.enable_coordination(user_data, ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn disable_coordination(&self, user_data: i32) -> FusionResult<()>
    {
        call_debug_enter!("FusionFrameworks::disable_coordination");
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                self.coordination.disable_coordination(user_data, ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn start_coordination(&self, user_data: i32,
                          remote_network_id: &str,
                          start_device_id: i32) -> FusionResult<()>
    {
        call_debug_enter!("FusionFrameworks::start_coordination");
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                info!(LOG_LABEL, "Call coordination.start_coordination");
                self.coordination.start_coordination(user_data, remote_network_id,
                    start_device_id, ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn stop_coordination(&self, user_data: i32, is_unchained: i32) -> FusionResult<()>
    {
        call_debug_enter!("FusionFrameworks::stop_coordination");
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                self.coordination.stop_coordination(user_data, is_unchained, ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn get_coordination_state(&self, user_data: i32, device_id: &str) -> FusionResult<i32>
    {
        call_debug_enter!("FusionFrameworks::get_coordination_state");
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                self.coordination.get_coordination_state(user_data, device_id, ipc_client_ref)
            }
            None => {
                error!(LOG_LABEL, "ipc_client is none");
                Err(FusionErrorCode::Fail)
            }
        }
    }
}

/// struct FusionFrameworks
#[derive(Default)]
pub struct FusionFrameworks(Mutex<Client>);

impl FusionFrameworks {
    /// Get a reference to the single instance of `FusionFrameworks`.
    pub fn get_instance() -> Option<&'static FusionFrameworks>
    {
        static mut FRAMEWORKS: Option<FusionFrameworks> = None;
        static INIT_ONCE: Once = Once::new();
        unsafe {
            INIT_ONCE.call_once(|| {
                FRAMEWORKS = Some(FusionFrameworks::default());
            });
            FRAMEWORKS.as_ref()
        }
    }

    /// Request connection of service via socket.
    pub fn alloc_socket_pair(&self, param: &AllocSocketPairParam) -> FusionResult<(FileDesc, i32)>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.alloc_socket_pair(param)
    }

    /// Request service to change to [`DRAG`] mode.
    pub fn start_drag(&self, drag_data: &DragData) -> FusionResult<i32>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.start_drag(drag_data)
    }

    /// Request to listen for events of multi-device cooperation.
    pub fn register_coordination_listener(&self) -> FusionResult<()>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.register_coordination_listener()
    }

    /// Request to stop listening for events of multi-device cooperation.
    pub fn unregister_coordination_listener(&self) -> FusionResult<()>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.unregister_coordination_listener()
    }

    /// Request to enable multi-device cooperation.
    pub fn enable_coordination(&self, user_data: i32) -> FusionResult<()>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.enable_coordination(user_data)
    }

    /// Request to disable multi-device cooperation.
    pub fn disable_coordination(&self, user_data: i32) -> FusionResult<()>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.disable_coordination(user_data)
    }

    /// Request to start multi-device cooperation.
    pub fn start_coordination(&self, user_data: i32,
                              remote_network_id: &str,
                              start_device_id: i32) -> FusionResult<()>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.start_coordination(user_data, remote_network_id, start_device_id)
    }

    /// Request to stop multi-device cooperation.
    pub fn stop_coordination(&self, user_data: i32, is_unchained: i32) -> FusionResult<()>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.stop_coordination(user_data, is_unchained)
    }

    /// Request for current switch status of multi-device cooperation.
    pub fn get_coordination_state(&self, user_data: i32, device_id: &str) -> FusionResult<i32>
    {
        let mut guard = self.0.lock().unwrap();
        guard.connect();
        guard.get_coordination_state(user_data, device_id)
    }
}
