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

//! IPC service definition.

#![allow(unused_variables)]

mod identity;

use std::ffi::{ c_char, CString };

use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{
    BorrowedMsgParcel, IRemoteBroker, IRemoteObj, MsgParcel, IMsgParcel,
    RemoteObj, RemoteStub, define_remote_object
};

use fusion_data_rust::Intention;
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };

use crate::identity::{ CommonAction, compose_param_id, split_action, split_intention, split_param };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionIpcService"
};

/// SA ID for "ohos.msdp.Idevicestatus"
pub const MSDP_DEVICESTATUS_SERVICE_ID: i32 = 2902;

/// Abstration of services.
///
/// By design, for ease of extention, all service implementations are required to
/// map its functions to this collection of interface, with services identified
/// by Intentions.
pub trait IDeviceStatus: IRemoteBroker {
    /// Enable the service identified by [`intention`].
    fn enable(&self, intention: Intention, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Disable the service identified by [`intention`].
    fn disable(&self, intention: Intention, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Start the service identified by [`intention`].
    fn start(&self, intention: Intention, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Stop the service identified by [`intention`].
    fn stop(&self, intention: Intention, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Add a watch of state of service, with the service identified by [`intention`],
    /// the state to watch identified by [`id`], parameters packed in [`data`] parcel.
    fn add_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Remove a watch of state of service.
    fn remove_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Set a parameter of service, with the service identified by [`intention`],
    /// the parameter identified by [`id`], and values packed in [`data`] parcel.
    fn set_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Get a parameter of service, with the service identified by [`intention`],
    /// the parameter identified by [`id`].
    fn get_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
    /// Interact with service identified by [`intention`] for general purpose. This interface
    /// supplements functions of previous intefaces. Functionalities of this interface is
    /// service spicific.
    fn control(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>;
}

fn on_remote_request(stub: &dyn IDeviceStatus, code: u32, data: &BorrowedMsgParcel<'_>,
                     reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
    call_debug_enter!("FusionIpcService::on_remote_request");
    let intention = split_intention(code)?;
    let id = split_param(code);

    match split_action(code)? {
        CommonAction::Enable => {
            info!(LOG_LABEL, "Call stub.enable()");
            stub.enable(intention, data, reply)
        }
        CommonAction::Disable => {
            info!(LOG_LABEL, "Call stub.disable()");
            stub.disable(intention, data, reply)
        }
        CommonAction::Start => {
            info!(LOG_LABEL, "Call stub.start()");
            stub.start(intention, data, reply)
        }
        CommonAction::Stop => {
            info!(LOG_LABEL, "Call stub.stop()");
            stub.stop(intention, data, reply)
        }
        CommonAction::AddWatch => {
            info!(LOG_LABEL, "Call stub.add_watch()");
            stub.add_watch(intention, id, data, reply)
        }
        CommonAction::RemoveWatch => {
            info!(LOG_LABEL, "Call stub.remove_watch()");
            stub.remove_watch(intention, id, data, reply)
        }
        CommonAction::SetParam => {
            info!(LOG_LABEL, "Call stub.set_param()");
            stub.set_param(intention, id, data, reply)
        }
        CommonAction::GetParam => {
            info!(LOG_LABEL, "Call stub.get_param()");
            stub.get_param(intention, id, data, reply)
        }
        CommonAction::Control => {
            info!(LOG_LABEL, "Call stub.control()");
            stub.control(intention, id, data, reply)
        }
    }
}

define_remote_object!(
    IDeviceStatus["ohos.msdp.Idevicestatus"] {
        stub: FusionIpcStub(on_remote_request),
        proxy: FusionIpcProxy,
    }
);

impl FusionIpcProxy {
    fn transfer_data(&self, src: &dyn IMsgParcel, target: &mut dyn IMsgParcel) -> FusionResult<()>
    {
        call_debug_enter!("FusionIpcProxy::transfer_data");
        let data_size = src.get_data_size();
        match src.read_buffer(data_size) {
            Ok(data_vec) => {
                if target.write_buffer(data_vec.as_slice()) {
                    Ok(())
                } else {
                    error!(LOG_LABEL, "write_buffer() failed");
                    Err(FusionErrorCode::Fail)
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "read_buffer() failed");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn send_request(&self, action: CommonAction, intention: Intention, id: u32,
        data: &BorrowedMsgParcel<'_>, reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()>
    {
        call_debug_enter!("FusionIpcProxy::send_request");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                self.transfer_data(data, &mut data_parcel)?;
                let code = compose_param_id(action, intention, id);
                let rep = {
                    match self.remote.send_request(code, &data_parcel, false) {
                        Ok(tr) => {
                            tr
                        }
                        Err(_) => {
                            error!(LOG_LABEL, "Failed to send request");
                            return Err(FusionErrorCode::Fail);
                        }
                    }
                };

                self.transfer_data(&rep, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not deref data");
                Err(FusionErrorCode::Fail)
            }
        }
    }
}

impl IDeviceStatus for FusionIpcProxy {
    fn enable(&self, intention: Intention, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::enable");
        self.send_request(CommonAction::Enable, intention, 0u32, data, reply)
    }

    fn disable(&self, intention: Intention, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::disable");
        self.send_request(CommonAction::Disable, intention, 0u32, data, reply)
    }

    fn start(&self, intention: Intention, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::start");
        self.send_request(CommonAction::Start, intention, 0u32, data, reply)
    }

    fn stop(&self, intention: Intention, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::stop");
        self.send_request(CommonAction::Stop, intention, 0u32, data, reply)
    }

    fn add_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::add_watch");
        self.send_request(CommonAction::AddWatch, intention, id, data, reply)
    }

    fn remove_watch(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::remove_watch");
        self.send_request(CommonAction::RemoveWatch, intention, id, data, reply)
    }

    fn set_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::set_param");
        self.send_request(CommonAction::SetParam, intention, id, data, reply)
    }

    fn get_param(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::get_param");
        self.send_request(CommonAction::GetParam, intention, id, data, reply)
    }

    fn control(&self, intention: Intention, id: u32, data: &BorrowedMsgParcel<'_>,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcProxy::control");
        self.send_request(CommonAction::Control, intention, id, data, reply)
    }
}
