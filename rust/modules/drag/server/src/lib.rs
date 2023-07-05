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

//! Drag Server implementation.

#![allow(dead_code)]
#![allow(unused_variables)]
// FIXME: need abi_stable crate or thin_trait_object crate
#![allow(improper_ctypes_definitions)]

extern crate fusion_data_rust;
extern crate fusion_utils_rust;
extern crate fusion_plugin_manager_rust;
extern crate hilog_rust;
extern crate ipc_rust;

use std::ffi::{ c_char, CString };
use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Serialize, Deserialize };
use fusion_data_rust::{ IPlugin, CallingContext, DragData, FusionResult };
use fusion_utils_rust::{ call_debug_enter };
use fusion_plugin_manager_rust::{ export_plugin };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionDragServer"
};

/// struct FusionDragServer
#[derive(Default)]
pub struct FusionDragServer {
    dummy: i32
}

// impl FusionDragServer {
//     /// TODO: add documentation.
//     pub fn new(ops: &FusionDragOperations) -> Self {
//         Self(ops.clone())
//     }
// }

impl IPlugin for FusionDragServer {
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::enable");
        Ok(0)
    }

    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::disable");
        Ok(0)
    }

    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::start");
        match DragData::deserialize(data) {
            Ok(drag_data) => {
                // let ret = unsafe {
                //     self.0.on_start_drag.map_or(-1, |call_start_drag| {
                //         info!(LOG_LABEL, "call on_start_drag()");
                //         call_start_drag(&dragData)
                //     })
                // };
                // if ret == 0 {
                //     Ok(0)
                // } else {
                //     error!(LOG_LABEL, "in FusionDragServer::start(): call_start_drag() failed");
                //     Err(-1)
                // }
                info!(LOG_LABEL, "in FusionDragServer::start(): call start_drag()");
                info!(LOG_LABEL, "{}", drag_data);
                match 0u32.serialize(reply) {
                    Ok(_) => {
                        Ok(0)
                    }
                    Err(_) => {
                        error!(LOG_LABEL, "Failed to serialize reply");
                        Err(-1)
                    }
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "in FusionDragServer::start(): DragData::deserialize() failed");
                Err(-1)
            }
        }
    }

    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::stop");
        Ok(0)
    }

    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::add_watch");
        Ok(0)
    }

    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::remove_watch");
        Ok(0)
    }

    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::set_param");
        Ok(0)
    }

    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::get_param");
        Ok(0)
    }

    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionDragServer::control");
        Ok(0)
    }
}

fn start(context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
    info!(LOG_LABEL, "in start()");
    Ok(0)
}

// #[no_mangle]
// pub unsafe extern "C" fn _create_plugin1() -> *mut IPluginInterface {
//     info!(LOG_LABEL, "in FusionDragServer::_create_plugin1(): enter");
//     let boxed = Box::new(IPluginInterface {
//         on_start_drag: start
//     });
//     let ptr = Box::into_raw(boxed);
//     info!(LOG_LABEL, "in FusionDragServer::export_plugin(): Box::into return");
//     ptr
// }

export_plugin!(FusionDragServer);
