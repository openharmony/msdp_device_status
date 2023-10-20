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

//! Plugin manager related definitions.

use std::any::Any;

use ipc_rust::BorrowedMsgParcel;

use fusion_utils_rust::{ define_enum, FusionResult, FusionErrorCode };

use crate::CallingContext;

define_enum! {
    Intention {
        Basic,
        Drag,
        Coordination
    }
}

/// Interface between service modules and intention framework.
pub trait IPlugin: Any + Send + Sync + std::panic::RefUnwindSafe {
    /// Enable this service.
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Disable this service.
    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Start work of this service.
    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Stop work of this service.
    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Add a watch to a state of this service.
    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Remove a watch to a state of this service.
    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Set a paramenter of this service.
    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Get a parameter of this service.
    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
    /// Interact with this service. This interface supplements functions of previous intefaces.
    /// Functionalities of this interface is service spicific.
    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<()>;
}
