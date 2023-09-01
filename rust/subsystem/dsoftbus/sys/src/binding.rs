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

//! rust dsoftbus binding sys

#![allow(dead_code)]
#![allow(missing_docs)]

use std::ffi::{ c_void, c_char };

///Constant for dsoftbus
pub const INTERCEPT_STRING_LENGTH: usize = 20;
pub const DINPUT_LINK_TYPE_MAX: i32 = 4;
pub const DEVICE_ID_SIZE_MAX: usize = 65;
pub const SESSION_SIDE_SERVER: i32 = 0;
pub const LINK_TYPE_WIFI_WLAN_5G: i32 = 1;
pub const LINK_TYPE_WIFI_WLAN_2G: i32 = 2;
pub const LINK_TYPE_WIFI_P2P: i32 = 3;
pub const LINK_TYPE_BR: i32 = 4;
pub const LINK_TYPE_MAX: usize = 9;
pub const TYPE_MESSAGE: i32 = 1;
pub const TYPE_BYTES: i32 = 2;
pub const TYPE_FILE: i32 = 3;
pub const TYPE_STREAM: i32 = 4;
pub const TYPE_BUTT: i32 = 5;
pub const NETWORK_ID_BUF_LEN: usize = 65;
pub const DEVICE_NAME_BUF_LEN: usize = 128;
pub const RET_OK: i32 = 0;

/// struct NodeBasicInfo
#[repr(C)]
pub struct NodeBasicInfo {
    /// network id of the device 
    pub network_id: [i8; NETWORK_ID_BUF_LEN],
    /// device name of the device 
    pub device_name: [i8; DEVICE_NAME_BUF_LEN],
    /// device type id of the device 
    pub device_type_id: u16,
}

/// struct SessionAttribute
#[repr(C)]
pub struct SessionAttribute {
    pub data_type: i32,
    pub link_type_num: i32,
    pub link_type: [i32; LINK_TYPE_MAX],
    pub stream_attr: i32,
    pub fast_trans_data: *const u8,
    pub fast_trans_data_size: u16,
}

/// struct StreamData
#[repr(C)]
pub struct StreamData {
    /// content of the buf
    pub buf_data: c_char,
    /// length of the buf
    pub buf_len: i32,
}

/// struct DataPacket
#[repr(C)]
pub struct DataPacket {
    /// message id of the message
    pub message_id: MessageId,
    /// length of the message
    pub buf_len: u32,
    /// content of the message
    pub data: Vec<*const c_char>,
}

/// enum MessageId
#[repr(C)]
#[derive(Eq, Hash, PartialEq)]
#[derive(Copy, Clone)]
pub enum MessageId {
    /// min id
    MinId,
    /// dragging data
    DraggingData,
    /// stopdrag data
    StopdragData,
    /// is pull up
    IsPullUp,
    /// max id
    MaxId,
}

/// struct StreamFrameInfo
#[repr(C)]
pub struct StreamFrameInfo {
    pub frame_type: i32,
    pub time_stamp: i64,
    pub seq_num: i32,
    pub seq_sub_num: i32,
    pub level_num: i32,
    pub bit_map: i32,
    pub tv_count: i32,
    pub tv_list: i32,
}

/// struct ISessionListener
#[repr(C)]
#[derive(Default)]
pub struct ISessionListener {
    /// The callback function is used to do something after listening that a session has been opened.
    pub on_session_opened: Option<OnSessionOpened>,
    /// The callback function is used to do something after listening that a session has been closed.
    pub on_session_closed: Option<OnSessionClosed>,
    /// The callback function is used to do something when listening to receive some byte messages.
    pub on_bytes_received: Option<OnBytesReceived>,
    /// The callback function is used to do something when listening to receive some string messages.
    pub on_message_received: Option<OnMessageReceived>,
    /// The callback function is used to do something when listening to receive some stream messages.
    pub on_stream_received: Option<OnstreamReceived>,
}

// Callback function type for OnSessionOpened() from native, this callback will be called after listening
// that a session has been opened.
pub type OnSessionOpened = extern "C" fn (session_id: i32, resultValue: i32) -> i32;
// Callback function type for OnSessionClosed() from native, this callback will be called after listening
// that a session has been opened.
pub type OnSessionClosed = extern "C" fn (session_id: i32);
// Callback function type for OnBytesReceived() from native, this callback will be called after listening
// to receive some byte messages.
pub type OnBytesReceived = extern "C" fn (session_id: i32, byteData: *const c_void, data_len: u32);
// Callback function type for OnMessageReceived() from native, this callback will be called after listening
// to receive some string messages.
pub type OnMessageReceived = extern "C" fn (session_id: i32, byteData: *const c_void, data_len: u32);
// Callback function type for OnstreamReceived() from native, this callback will be called after listening
// to receive some stream messages.
pub type OnstreamReceived = extern "C" fn (session_id: i32, byteData: *const StreamData,
    extData: *const StreamData, paramData: *const StreamFrameInfo);
// Callback function type for OnHandleRecvData() from other, this callback will be called when callback
// functions that receive the message have been called.
pub type OnHandleRecvData = extern "C" fn (session_id: i32, message: *const c_char);

extern "C" {
    /// This function is used to get permission:DISTRIBUTED_DATASYNC, it will be called in main.rs.
    pub fn GetAccessToken();
}

// These C interfaces are defined in lib: dsoftbus:softbus_client
extern "C" {
    /// interface of CreateSessionServer
    pub fn CreateSessionServer(pkg_name: *const c_char, session_name: *const c_char, session_listener: *const ISessionListener) -> i32;
    /// interface of RemoveSessionServer
    pub fn RemoveSessionServer(pkg_name: *const c_char, session_name: *const c_char) -> i32;
    /// interface of CloseSession
    pub fn CloseSession(session_id: i32);
    /// interface of OpenSession
    pub fn OpenSession(my_session_name: *const c_char, peer_session_name: *const c_char, peer_device_id: *const c_char,
        groupId: *const c_char, attr: *const SessionAttribute) -> i32;
    /// interface of GetPeerDeviceId
    pub fn GetPeerDeviceId(session_id: i32, peer_dev_id: *mut c_char, len: u32) -> i32;
    /// interface of GetSessionSide
    pub fn GetSessionSide(session_id: i32) -> i32;
    /// interface of GetLocalNodeDeviceInfo
    pub fn GetLocalNodeDeviceInfo(pkg_name: *const c_char, info: *mut NodeBasicInfo) -> i32;
    /// interface of SendBytes
    pub fn SendBytes(session_id: i32, data: *const c_void, len: u32) -> i32;
}

extern "C" {
    /// The function is used to save callback function.
    pub fn CSaveHandleCb(call_back: Option<OnHandleRecvData>);
    /// The function is used to get callback function.
    pub fn CGetHandleCb() -> Option<OnHandleRecvData>;
}

// These C interfaces wrapper some c++ functions defined in 'coordination_sm_rust.cpp'.
extern "C" {
    /// interface of CReset
    pub fn CReset(network_id: *const c_char);
}

