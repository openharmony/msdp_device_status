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

//! Delegator of IPC service.
//!
//! This is part of implementation of IPC service as required by IPC framework.
//! As IPC request will first be delegated to this [`delegator`]. [`delegator`]
//! will check user token, and then redirect request to business module.

#![allow(unused_variables)]

use std::cell::Cell;
use std::sync::Mutex;
use std::ffi::{ c_char, CString };
use hilog_rust::{ debug, info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Deserialize, InterfaceToken, IRemoteBroker, IRemoteStub };
use fusion_data_rust::{ Intention, IPlugin, CallingContext };
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use fusion_ipc_service_rust::{ IDeviceStatus, FusionIpcStub };
use fusion_plugin_manager_rust::PluginManager;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionIpcDelegator"
};

pub struct FusionIpcDelegator {
    plugin_mgr: Mutex<Cell<PluginManager>>,
}

impl FusionIpcDelegator {
    pub fn new() -> Self {
        Self {
            plugin_mgr: Mutex::new(Cell::new(PluginManager::default()))
        }
    }

    fn check_interface_token(&self, data: &BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::check_interface_token");
        match InterfaceToken::deserialize(data) {
            Ok(token) => {
                debug!(LOG_LABEL, "check interface token");
                if token.get_token() != FusionIpcStub::get_descriptor() {
                    error!(LOG_LABEL, "Unexpected token");
                    Err(FusionErrorCode::Fail)
                } else {
                    Ok(())
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "Deserialization of interface token failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn load_plugin(&self, intention: Intention) -> FusionResult<Box<dyn IPlugin>> {
        call_debug_enter!("FusionIpcDelegator::load_plugin");
        match self.plugin_mgr.lock() {
            Ok(mut guard) => {
                let plugin = guard.get_mut().load_plugin(intention);
                match plugin {
                    Some(plugin) => {
                        debug!(LOG_LABEL, "Plugin loaded");
                        Ok(plugin)
                    }
                    None => {
                        error!(LOG_LABEL, "Failed to load intention module");
                        Err(FusionErrorCode::Fail)
                    }
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "Error locking");
                Err(FusionErrorCode::Fail)
            }
        }
    }
}

impl IDeviceStatus for FusionIpcDelegator {
    fn enable(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::enable");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.enable()");
        plugin.enable(&context, data, reply)
    }

    fn disable(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::disable");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.disable()");
        plugin.disable(&context, data, reply)
    }

    fn start(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::start");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.start()");
        plugin.start(&context, data, reply)
    }

    fn stop(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::stop");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.stop()");
        plugin.stop(&context, data, reply)
    }

    fn add_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::add_watch");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.add_watch()");
        plugin.add_watch(&context, id, data, reply)
    }

    fn remove_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::remove_watch");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.remove_watch()");
        plugin.remove_watch(&context, id, data, reply)
    }

    fn set_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::set_param");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.set_param()");
        plugin.set_param(&context, id, data, reply)
    }

    fn get_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::get_param");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.get_param()");
        plugin.get_param(&context, id, data, reply)
    }

    fn control(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<()> {
        call_debug_enter!("FusionIpcDelegator::control");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "Call plugin.control()");
        plugin.control(&context, id, data, reply)
    }
}

impl IRemoteBroker for FusionIpcDelegator {}
