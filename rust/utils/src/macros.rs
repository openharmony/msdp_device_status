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

//! Several macros

#![allow(dead_code)]

/// macro define_enum can be used to define enum type, which is associated with several helper functions.
#[macro_export]
macro_rules! define_enum {
    {
        $name:ident {
            $( $item:ident ), *
        }
    } => {
        /// enum $name
        #[allow(dead_code)]
        #[derive(Hash, PartialEq, Eq, Clone)]
        #[repr(u32)]
        pub enum $name {
            $(
                /// variant $item
                $item,
            )*
        }

        impl TryFrom<u32> for $name {
            type Error = FusionErrorCode;
            fn try_from(code: u32) -> std::result::Result<Self, Self::Error> {
                match code {
                    $(
                        _ if code == Self::$item as u32 => {
                            Ok(Self::$item)
                        }
                    )*
                    _ => {
                        Err(FusionErrorCode::Fail)
                    }
                }
            }
        }

        impl ipc_rust::Serialize for $name {
            fn serialize(&self, parcel: &mut ipc_rust::BorrowedMsgParcel<'_>) -> ipc_rust::IpcResult<()> {
                match self {
                    $(
                        Self::$item => {
                            (Self::$item as u32).serialize(parcel)
                        }
                    )*
                }
            }
        }

        impl ipc_rust::Deserialize for $name {
            fn deserialize(parcel: &ipc_rust::BorrowedMsgParcel<'_>) -> ipc_rust::IpcResult<Self> {
                match u32::deserialize(parcel) {
                    Ok(val) => {
                        match $name::try_from(val) {
                            Ok(e) => {
                                Ok(e)
                            }
                            Err(_) => {
                                Err(ipc_rust::IpcStatusCode::InvalidValue)
                            }
                        }
                    }
                    Err(err) => {
                        Err(err)
                    }
                }
            }
        }
    };
}

/// Helper to log the enter and leave of function calls.
pub struct InnerFunctionTracer<'a> {
    log: Box<dyn Fn(&str, &str)>,
    func_name: &'a str
}

impl<'a> InnerFunctionTracer<'a> {
    /// Construct a new instance of [`InnerFunctionTracer`].
    pub fn new(log: Box<dyn Fn(&str, &str)>, func_name: &'a str) -> Self {
        log(func_name, "enter");
        Self {
            log, func_name
        }
    }
}

impl<'a> Drop for InnerFunctionTracer<'a> {
    fn drop(&mut self) {
        (self.log)(self.func_name, "leave");
    }
}

/// Log in `DEBUG` level the enter and leave of function calls.
#[macro_export]
macro_rules! call_debug_enter {
    (
        $func_name: literal
    ) => {
        let __inner_function_tracer__ = $crate::InnerFunctionTracer::new(
            Box::new(|func_name: &str, action: &str| {
                hilog_rust::debug!(LOG_LABEL, "In {}: {}", @public(func_name), @public(action));
            }),
            $func_name
        );
    };
}

/// Log in `INFO` level the enter and leave of function calls.
#[macro_export]
macro_rules! call_info_trace {
    (
        $func_name: literal
    ) => {
        let __inner_function_tracer__ = $crate::InnerFunctionTracer::new(
            Box::new(|func_name: &str, action: &str| {
                hilog_rust::info!(LOG_LABEL, "In {}: {}", @public(func_name), @public(action));
            }),
            $func_name
        );
    };
}

/// When the expression is an `Err` variant, "log(C function name) failed" is recorded and return the error.
#[macro_export]
macro_rules! err_log {
    ($expr:expr, $log:expr) => {
        match $expr {
            Ok(val) => val,
            Err(err) => {
                error!(LOG_LABEL, "{} failed", $log);
                return Err(err);
            }
        }
    };
}