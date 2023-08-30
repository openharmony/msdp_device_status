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

//! rust input binding sys

#![allow(dead_code)]
#![allow(missing_docs)]

use std::ffi::{ c_void, c_char };

///for dsoftbus
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
pub const NETWORK_ID_BUF_LEN: i32 = 65;
pub const DEVICE_NAME_BUF_LEN: i32 = 128;
pub const RET_OK: i32 = 0;

/// struct for dsoftbus
#[repr(C)]
pub struct NodeBasicInfo {
    pub network_id: [i8; NETWORK_ID_BUF_LEN as usize],
    pub device_name: [i8; DEVICE_NAME_BUF_LEN as usize],
    pub device_type_id: u16,
}

/// struct for dsoftbus
#[repr(C)]
pub struct SessionAttribute {
    pub data_type: i32,
    pub link_type_num: i32,
    pub link_type: [i32; LINK_TYPE_MAX],
    pub stream_attr: i32,
    pub fast_trans_data: *const u8,
    pub fast_trans_data_size: u16,
}

/// struct for dsoftbus
#[repr(C)]
pub struct StreamData {
    pub buf_data: c_char,
    pub buf_len: i32,
}

/// struct for dsoftbus
#[repr(C)]
pub struct DataPacket {
    pub message_id: MessageId,
    pub buf_len: u32,
    pub data: Vec<*const c_char>,
}

/// enum for adapter and MessageId
#[repr(C)]
#[derive(Eq, Hash, PartialEq)]
#[derive(Copy, Clone)]
pub enum MessageId {
    MinId,
    DraggingData,
    StopdragData,
    IsPullUp,
    MaxId,
}

/// struct for dsoftbus
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

/// struct for dsoftbus
#[repr(C)]
#[derive(Default)]
pub struct ISessionListener {
    pub on_session_opened: Option<OnSessionOpened>,
    pub on_session_closed: Option<OnSessionClosed>,
    pub on_bytes_received: Option<OnBytesReceived>,
    pub on_message_received: Option<OnMessageReceived>,
    pub on_stream_received: Option<OnstreamReceived>,
}

/// callback type OnSessionOpened
pub type OnSessionOpened = extern "C" fn (session_id: i32, resultValue: i32) -> i32;
/// callback type OnSessionClosed
pub type OnSessionClosed = extern "C" fn (session_id: i32);
/// callback type OnBytesReceived
pub type OnBytesReceived = extern "C" fn (session_id: i32, byteData: *const c_void, data_len: u32);
/// callback type OnMessageReceived
pub type OnMessageReceived = extern "C" fn (session_id: i32, byteData: *const c_void, data_len: u32);
/// callback type OnstreamReceived
pub type OnstreamReceived = extern "C" fn (session_id: i32, byteData: *const StreamData,
    extData: *const StreamData, paramData: *const StreamFrameInfo);
pub type OnHandleRecvData = extern "C" fn (session_id: i32, message: *const c_char);

// C interface for main
extern "C" {
    /// interface of GetAccessToken
    pub fn GetAccessToken();
}

// C interface for dsoftbus,function definition in lib: dsoftbus:softbus_client
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
    /// interface of CReset
    pub fn CReset(network_id: *const c_char);
    /// interface of CSaveHandleCb
    pub fn CSaveHandleCb(call_back: Option<OnHandleRecvData>);
    /// interface of CGetHandleCb
    pub fn CGetHandleCb() -> Option<OnHandleRecvData>;
}
