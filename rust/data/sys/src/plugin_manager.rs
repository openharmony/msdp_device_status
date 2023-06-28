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
use ipc_rust::{ BorrowedMsgParcel };
use crate::{ CallingContext, FusionResult };
use crate::fusion_utils_rust::{ define_enum };

define_enum! {
    Intention {
        Basic,
        Drag,
        Coordination
    }
}

/// trait IPlugin
/// TOTO: add documentation.
pub trait IPlugin: Any + Send + Sync + std::panic::RefUnwindSafe {
    /// TODO: add documentation
    fn enable(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn disable(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn start(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn stop(&self, context: &CallingContext, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn add_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn remove_watch(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn set_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn get_param(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
    /// TODO: add documentation
    fn control(&self, context: &CallingContext, id: u32, data: &BorrowedMsgParcel, reply: &mut BorrowedMsgParcel) -> FusionResult<i32>;
}
