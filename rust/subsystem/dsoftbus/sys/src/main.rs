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

//! rust deviceProfile test main

#![allow(unused_imports)]
#![allow(dead_code)]
#![allow(unused_variables)]
#![allow(missing_docs)]

use std::sync::{Arc, Mutex};
use std::ffi::{CString, c_char, c_void, c_int, CStr};
use std::env;
use std::vec::Vec;
use std::io::Read;
use std::io::Stdin;

use hilog_rust::{error, hilog, debug, HiLogLabel, LogType};
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "main"
};

use fusion_dsoftbus_rust::{ dsoftbus::DSoftbus, binding::{GetAccessToken, MessageId} };

fn main() {
    debug!(LOG_LABEL, "Main:test");
    let mut buf = String::new();
    let std_in: Stdin = std::io::stdin();
    if std_in.read_line(&mut buf).is_ok() {
        let init = String::from("init\n");
        if buf.eq(&init) {
            println!("I am init");
            unsafe { GetAccessToken() };
            match DSoftbus::get_instance() {
                Some(dsoftbus) => {
                    match dsoftbus.init() {
                        Ok(ret) => {
                            println!("DSoftbus init success");
                        }
                        Err(err) => {
                            println!("DSoftbus init failed");
                        }
                    };
                    0
                }
                None => {
                    error!(LOG_LABEL, "DSoftbus init failed");
                    -1
                }
            };
            println!("init is sleeping");
            std::thread::sleep(std::time::Duration::from_millis(1000*60*10));
        } else {
            println!("I am open_input_softbus");
            let mut buf = String::new();
            let std_in: Stdin = std::io::stdin();
            if std_in.read_line(&mut buf).is_ok() {     
                println!("before remove, remote_network_id len= {}", buf.len());
                if buf.len() > 64 {
                    buf.remove(64);
                    println!("after remove, remote_network_id len= {}", buf.len());
                }
                        
                let remote_network_id = buf;
                println!("init::remote_network_id = {}", &remote_network_id);
                match DSoftbus::get_instance() {
                    Some(dsoftbus) => {
                        unsafe { GetAccessToken() };
                        match dsoftbus.open_input_softbus(&remote_network_id) {
                            Ok(ret) => {
                                println!("DSoftbus open_input_softbus success");
                            }
                            Err(err) => {
                                println!("DSoftbus open_input_softbus failed");
                            }
                        };
                        println!("start open_input_softbus sleep 10 sec");
                        std::thread::sleep(std::time::Duration::from_millis(1000*10));
                        println!("stop open_input_softbus sleep 10 sec");
                        let message = String::from("I am server!\n");
                        let len = message.len().try_into().unwrap();
                        match dsoftbus.send_msg(&remote_network_id, MessageId::MinId,
                            message.as_ptr() as *const c_char as *const c_void, len) {
                            Ok(ret) => {
                                println!("send success!!!");
                                std::thread::sleep(std::time::Duration::from_millis(1000*10));
                                dsoftbus.close_input_softbus(&remote_network_id);
                            }
                            Err(err) => {
                                println!("send failed!!!");
                            }
                        };        
                        println!("after send_data is sleeping");                  
                        std::thread::sleep(std::time::Duration::from_millis(1000*60*10));
                        0
                    }
                    None => {
                        error!(LOG_LABEL, "DSoftbus open_input_softbus failed");
                        -1
                    }
                };
            } else {
                println!("remote_network_id is empty!!!");
            }
        }
    }
}