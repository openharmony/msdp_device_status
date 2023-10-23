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

//! Implementation of client side of IPC.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, CString };

use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use ipc_rust::{
    BorrowedMsgParcel, FromRemoteObj, InterfaceToken, MsgParcel,
    RemoteObjRef, Serialize, get_service,
};

use fusion_data_rust::Intention;
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use fusion_ipc_service_rust::{ IDeviceStatus, FusionIpcProxy, MSDP_DEVICESTATUS_SERVICE_ID };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionIpcClient"
};

/// Representation of client side of IPC.
pub struct FusionIpcClient(RemoteObjRef<dyn IDeviceStatus>);

impl FusionIpcClient {
    /// Connect device status service.
    pub fn connect() -> FusionResult<Self> {
        call_debug_enter!("FusionIpcClient::connect");
        match get_service(MSDP_DEVICESTATUS_SERVICE_ID) {
            Ok(obj) => {
                match <dyn IDeviceStatus as FromRemoteObj>::try_from(obj) {
                    Ok(obj_ref) => {
                        info!(LOG_LABEL, "Connect to service successfully");
                        Ok(FusionIpcClient(obj_ref))
                    }
                    Err(err) => {
                        error!(LOG_LABEL, "Can not dereference remote object");
                        Err(FusionErrorCode::Fail)
                    }
                }
            }
            Err(err) => {
                error!(LOG_LABEL, "Can not connect to service");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    fn add_interface_token(&self, data_parcel: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::add_interface_token");
        let token = InterfaceToken::new(FusionIpcProxy::get_descriptor());
        match token.serialize(data_parcel) {
            Ok(_) => {
                Ok(())
            }
            Err(_) => {
                error!(LOG_LABEL, "Failed to serialize interface token");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to enable the service identified by [`intention`].
    pub fn enable(&self, intention: Intention, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::enable");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.enable()");
                self.0.enable(intention, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to disable the service identified by [`intention`].
    pub fn disable(&self, intention: Intention, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::disable");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.disable()");
                self.0.disable(intention, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to start the service identified by [`intention`].
    pub fn start(&self, intention: Intention, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::start");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.start()");
                self.0.start(intention, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to stop the service identified by [`intention`].
    pub fn stop(&self, intention: Intention, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::stop");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.stop()");
                self.0.stop(intention, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to add a watch of state of service, with the service identified by
    /// [`intention`], the state to watch identified by [`id`], parameters packed in
    /// [`data`] parcel.
    pub fn add_watch(&self, intention: Intention, id: u32, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::add_watch");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.add_watch()");
                self.0.add_watch(intention, id, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to remove a watch of state of service.
    pub fn remove_watch(&self, intention: Intention, id: u32, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::remove_watch");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.remove_watch()");
                self.0.remove_watch(intention, id, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to set a parameter of service, with the service identified by
    /// [`intention`], the parameter identified by [`id`], and values packed in
    /// [`data`] parcel.
    pub fn set_param(&self, intention: Intention, id: u32, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::set_param");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.set_param()");
                self.0.set_param(intention, id, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to get a parameter of service, with the service identified by
    /// [`intention`], the parameter identified by [`id`].
    pub fn get_param(&self, intention: Intention, id: u32, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::get_param");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.get_param()");
                self.0.get_param(intention, id, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Request to interact with service identified by [`intention`] for general purpose.
    /// This interface supplements functions of previous intefaces. Functionalities of
    /// this interface is service spicific.
    pub fn control(&self, intention: Intention, id: u32, data: &dyn Serialize,
        reply: &mut BorrowedMsgParcel<'_>) -> FusionResult<()> {
        call_debug_enter!("FusionIpcClient::control");
        match MsgParcel::new() {
            Some(mut data_parcel) => {
                let mut borrowed_data_parcel = data_parcel.borrowed();
                info!(LOG_LABEL, "Serialize interface token");
                self.add_interface_token(&mut borrowed_data_parcel)?;

                if data.serialize(&mut borrowed_data_parcel).is_err() {
                    return Err(FusionErrorCode::Fail);
                }
                info!(LOG_LABEL, "Call proxy.control()");
                self.0.control(intention, id, &borrowed_data_parcel, reply)
            }
            None => {
                error!(LOG_LABEL, "Can not instantiate MsgParcel");
                Err(FusionErrorCode::Fail)
            }
        }
    }
}
