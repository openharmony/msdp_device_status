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

//! Fusion IPC delegator

#![allow(unused_variables)]

use std::cell::Cell;
use std::sync::Mutex;
use std::ffi::{ c_char, CString };
use crate::hilog_rust::{ debug, info, error, hilog, HiLogLabel, LogType };
use crate::ipc_rust::{ BorrowedMsgParcel, Deserialize, InterfaceToken, IRemoteBroker, IRemoteStub };
use crate::fusion_data_rust::{ Intention, IPlugin, CallingContext, FusionResult };
use crate::fusion_utils_rust::call_debug_enter;
use crate::fusion_ipc_service_rust::{ IDeviceStatus, FusionIpcStub };
use crate::fusion_plugin_manager_rust::PluginManager;

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

    fn check_interface_token(&self, data: &BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::check_interface_token");
        match InterfaceToken::deserialize(data) {
            Ok(token) => {
                debug!(LOG_LABEL, "check interface token");
                if token.get_token() != FusionIpcStub::get_descriptor() {
                    error!(LOG_LABEL, "unexpected token");
                    Err(-1)
                } else {
                    Ok(0)
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "Deserialization of interface token fail");
                Err(-1)
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
                        error!(LOG_LABEL, "Fail to load intention module");
                        Err(-1)
                    }
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "error locking");
                Err(-1)
            }
        }
    }
}

impl IDeviceStatus for FusionIpcDelegator {
    fn enable(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::enable");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.enable()");
        plugin.enable(&context, data, reply)
    }

    fn disable(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::disable");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.disable()");
        plugin.disable(&context, data, reply)
    }

    fn start(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::start");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.start()");
        plugin.start(&context, data, reply)
    }

    fn stop(&self, intention: Intention, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::stop");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.stop()");
        plugin.stop(&context, data, reply)
    }

    fn add_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::add_watch");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.add_watch()");
        plugin.add_watch(&context, id, data, reply)
    }

    fn remove_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::remove_watch");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.remove_watch()");
        plugin.remove_watch(&context, id, data, reply)
    }

    fn set_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::set_param");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.set_param()");
        plugin.set_param(&context, id, data, reply)
    }

    fn get_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::get_param");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.get_param()");
        plugin.get_param(&context, id, data, reply)
    }

    fn control(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel,
        reply: &mut BorrowedMsgParcel) -> FusionResult<i32> {
        call_debug_enter!("FusionIpcDelegator::control");
        self.check_interface_token(data)?;

        let plugin = self.load_plugin(intention)?;
        let context = CallingContext::current();
        info!(LOG_LABEL, "call plugin.control()");
        plugin.control(&context, id, data, reply)
    }
}

impl IRemoteBroker for FusionIpcDelegator {}
