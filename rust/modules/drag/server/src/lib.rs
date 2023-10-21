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

use std::ffi::{ c_char, CString };
use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Serialize, Deserialize };
use fusion_data_rust::{ IPlugin, CallingContext, DragData };
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use fusion_plugin_manager_rust::export_plugin;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionDragServer"
};

/// Implementation of drag service.
///
/// # Functions provided by drag service:
///
///     * Start drag and put service in DRAG mode.
///     * Stop drag and reset mode of service.
///     * Add listener for drag events.
///     * Remove listener of drag events.
///     * Set visibility of drag window.
///     * Update shadow.
///
/// For integration with `intention framework`, we have to map functions
/// mention above to [`IPlugin`] as:
///
///     * `IPlugin::start` to start drag and put service in DRAG mode.
///     * `IPlugin::stop` to start drag and reset mode of service.
///     * `IPlugin::add_watch` to add listener for drag events.
///     * `IPlugin::remove_watch` to remove listener of drag events.
///     * `IPlugin::set_param` to set visibility of drag window, using
///       [`DRAG_WINDOW_VISIBILITY`] as parameter ID.
///     * `IPlugin::set_param` to update shadow, using [`UPDATE_SHADOW`]
///       as parameter ID.
///
/// Default action for unmapped interfaces of [`IPlugin`] is simply to
/// fail and return error.
///
#[derive(Default)]
pub struct FusionDragServer(i32);

impl IPlugin for FusionDragServer {
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::enable");
        Ok(())
    }

    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::disable");
        Ok(())
    }

    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::start");
        match DragData::deserialize(data) {
            Ok(drag_data) => {
                info!(LOG_LABEL, "In FusionDragServer::start(): call start_drag()");
                info!(LOG_LABEL, "{}", drag_data);
                match 0u32.serialize(reply) {
                    Ok(_) => {
                        Ok(())
                    }
                    Err(_) => {
                        error!(LOG_LABEL, "Failed to serialize reply");
                        Err(FusionErrorCode::Fail)
                    }
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "In FusionDragServer::start(): DragData::deserialize() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::stop");
        Ok(())
    }

    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::add_watch");
        Ok(())
    }

    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::remove_watch");
        Ok(())
    }

    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::set_param");
        Ok(())
    }

    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::get_param");
        Ok(())
    }

    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionDragServer::control");
        Ok(())
    }
}

export_plugin!(FusionDragServer);
