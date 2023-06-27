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

use std::rc::Rc;
use std::sync::Once;
use std::os::fd::AsRawFd;
use std::ffi::{ c_char, CString };
use hilog_rust::{ error, info, hilog, HiLogLabel, LogType };
use ipc_rust::FileDesc;
use fusion_utils_rust::call_debug_enter;
use fusion_ipc_client_rust::FusionIpcClient;
use fusion_data_rust::{ AllocSocketPairParam, CDragData, DragData, FusionResult };
use fusion_basic_client_rust::FusionBasicClient;
use fusion_drag_client_rust::DragClient;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionFrameworks"
};

static mut FRAMEWORKS: Option<FusionFrameworks> = None;
static INIT: Once = Once::new();

fn init_global_values() {
    unsafe {
        INIT.call_once(|| {
            FRAMEWORKS = Some(FusionFrameworks {
                ipc_client: None,
                basic: FusionBasicClient::default(),
                drag: DragClient::default()
            });
        });
    }
}

/// struct FusionFrameworks
#[derive(Default)]
pub struct FusionFrameworks {
    ipc_client: Option<Rc<FusionIpcClient>>,
    basic: FusionBasicClient,
    drag: DragClient,
}

impl<'a> FusionFrameworks {
    /// TODO: add documentation.
    pub fn get_instance() -> Option<&'static FusionFrameworks> {
        init_global_values();
        unsafe {
            FRAMEWORKS.as_ref()
        }
    }

    /// TODO: add documentation.
    pub fn get_mut_instance() -> Option<&'static mut FusionFrameworks> {
        init_global_values();
        unsafe {
            FRAMEWORKS.as_mut()
        }
    }

    /// TODO: add documentation.
    pub fn set_ipc_connect(&'a mut self) {
        if self.ipc_client.is_some() {
            return;
        }
        info!(LOG_LABEL, "trying to connect server");
        match FusionIpcClient::connect() {
            Ok(client) => {
                info!(LOG_LABEL, "Connect to server successfully");
                self.ipc_client = Some(Rc::new(client));
            }
            Err(_) => {
                error!(LOG_LABEL, "Can not connect to server.");
            }
        }
    }

    /// TODO: add documentation.
    pub fn alloc_socket_pair(&self, param: &AllocSocketPairParam) -> FusionResult<(FileDesc, i32)> {
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                info!(LOG_LABEL, "in FusionFrameworks::start_drag(): call basic.start_drag()");
                self.basic.alloc_socket_pair(param, ipc_client_ref.clone())
            }
            None => {
                Err(-1)
            }
        }
    }

    /// TODO: add documentation.
    pub fn start_drag(&self, drag_data: &DragData) -> FusionResult<i32> {
        match self.ipc_client.as_ref() {
            Some(ipc_client_ref) => {
                info!(LOG_LABEL, "in FusionFrameworks::start_drag(): call drag.start_drag()");
                self.drag.start_drag(drag_data, ipc_client_ref.clone())
            }
            None => {
                Err(-1)
            }
        }
    }
}

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
                error!(LOG_LABEL, "in fusion_start_drag(): can not dereference mutable FusionFrameworks instance");
                return -1;
            }
        }

        let drag_data = DragData::from_c(c_drag_data_ref);
        match FusionFrameworks::get_instance() {
            Some(fw) => {
                info!(LOG_LABEL, "in fusion_start_drag(): call start_drag()");
                match fw.start_drag(&drag_data) {
                    Ok(_) => {
                        0
                    }
                    Err(err) => {
                        error!(LOG_LABEL, "in fusion_start_drag(): error happened when starting drag");
                        err
                    }
                }
            }
            None => {
                error!(LOG_LABEL, "in fusion_start_drag(): can not dereference FusionFrameworks instance");
                -1
            }
        }
    })
}
