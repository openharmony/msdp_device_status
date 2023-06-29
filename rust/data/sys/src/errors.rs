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

//! Error codes definitions.

/// Error codes.
#[repr(i32)]
pub enum FusionErrorCode {
    /// Operation failed.
    Fail,
    /// Invalid input parameter(s).
    InvalidParam,
}

impl From<FusionErrorCode> for i32 {
    fn from(value: FusionErrorCode) -> Self
    {
        match value {
            FusionErrorCode::Fail => { -1i32 },
            FusionErrorCode::InvalidParam => { -2i32 },
        }
    }
}

impl TryFrom<i32> for FusionErrorCode {
    type Error = FusionErrorCode;

    fn try_from(value: i32) -> Result<Self, Self::Error> {
        match value {
            _ if i32::from(FusionErrorCode::Fail) == value => { Ok(FusionErrorCode::Fail) },
            _ if i32::from(FusionErrorCode::InvalidParam) == value => { Ok(FusionErrorCode::InvalidParam) },
            _ => { Err(FusionErrorCode::Fail) },
        }
    }
}

/// IPC specific Result, error is i32 type
pub type FusionResult<T> = std::result::Result<T, i32>;