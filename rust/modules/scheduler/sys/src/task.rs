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

//! Implementation of task handle.

use fusion_utils_rust::{ FusionErrorCode, FusionResult };
use ylong_runtime::task::JoinHandle;

/// Representation of task handle.
pub struct TaskHandle<R> {
    join_handle: Option<JoinHandle<R>>,
}

impl<R> From<JoinHandle<R>> for TaskHandle<R> {
    fn from(value: JoinHandle<R>) -> Self
    {
        Self {
            join_handle: Some(value),
        }
    }
}

impl<R> TaskHandle<R> {
    /// Cancel this task.
    pub fn cancel(&mut self)
    {
        if let Some(join_handle) = self.join_handle.take() {
            join_handle.cancel();
        }
    }

    /// Get result of this task.
    pub fn result(&mut self) -> FusionResult<R>
    {
        if let Some(join_handle) = self.join_handle.take() {
            if let Ok(ret) = ylong_runtime::block_on(join_handle) {
                Ok(ret)
            } else {
                Err(FusionErrorCode::Fail)
            }
        } else {
            Err(FusionErrorCode::Fail)
        }
    }
}
