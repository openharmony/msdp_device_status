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
pub const RET_ERROR: i32 = -1;
pub const C_CHAR_SIZE: usize = std::mem::size_of::<c_char>();

/// NodeBasicInfo is a structure that represents basic information about a node.
#[repr(C)]
pub struct NodeBasicInfo {
    /// The network ID of the device.
    pub network_id: [i8; NETWORK_ID_BUF_LEN],
    /// The device name of the device.
    pub device_name: [i8; DEVICE_NAME_BUF_LEN],
    /// The device type ID of the device.
    pub device_type_id: u16,
}

/// SessionAttribute is a structure that represents session attributes.
#[repr(C)]
pub struct SessionAttribute {
    /// The data type of the session attribute.
    pub data_type: i32,
    /// The number of link types in the session attribute.
    pub link_type_num: i32,
    /// The array of link types in the session attribute.
    pub link_type: [i32; LINK_TYPE_MAX],
    /// The stream attribute of the session.
    pub stream_attr: i32,
    /// The pointer to the fast transmission data.
    pub fast_trans_data: *const u8,
    /// The size of the fast transmission data.
    pub fast_trans_data_size: u16,
}

/// StreamData is a structure that represents stream data.
#[repr(C)]
pub struct StreamData {
    /// The content of the buffer.
    pub buf_data: c_char,
    /// The length of the buffer.
    pub buf_len: i32,
}

/// StreamFrameInfo is a structure that contains information about a stream frame.
#[repr(C)]
pub struct StreamFrameInfo {
    /// The type of the frame.
    pub frame_type: i32,
    /// The time stamp of the frame.
    pub time_stamp: i64,
    /// The sequence number of the frame.
    pub seq_num: i32,
    /// The sub-sequence number of the frame.
    pub seq_sub_num: i32,
    /// The level number of the frame.
    pub level_num: i32,
    /// The bit map of the frame.
    pub bit_map: i32,
    /// The number of TV (Television) in the frame.
    pub tv_count: i32,
    /// The list of TV (Television) in the frame.
    pub tv_list: i32,
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

/// ISessionListener is a structure that defines the callbacks for session events.
#[repr(C)]
pub struct ISessionListener {
    /// The callback function is used to do something after listening that a session has been opened.
    pub on_session_opened: OnSessionOpened,
    /// The callback function is used to do something after listening that a session has been closed.
    pub on_session_closed: OnSessionClosed,
    /// The callback function is used to do something when listening to receive some byte messages.
    pub on_bytes_received: OnBytesReceived,
    /// The callback function is used to do something when listening to receive some string messages.
    pub on_message_received: OnMessageReceived,
    /// The callback function is used to do something when listening to receive some stream messages.
    pub on_stream_received: OnstreamReceived,
}

/// Provide for C lib to call
extern "C" fn on_session_opened_c(_session_id: i32, _result: i32) -> i32 {
    0
}

/// Provide for C lib to call
extern "C" fn on_session_closed_c(_session_id: i32) {
}

/// Provide for C lib to call
extern "C" fn on_bytes_received_c(_session_id: i32, _data: *const c_void, _data_len: u32) {
}

/// Provide for C lib to call
extern "C" fn on_message_received_c(_session_id: i32, _byte_data: *const c_void, _data_len: u32) {
}

/// Provide for C lib to call
extern "C" fn on_stream_received_c(_session_id: i32, _byte_data: *const StreamData,
    _ext_data: *const StreamData, _param_data: *const StreamFrameInfo) {
}

impl Default for ISessionListener {
    fn default() -> Self {
        ISessionListener {
            on_session_opened: on_session_opened_c,
            on_session_closed: on_session_closed_c,
            on_bytes_received: on_bytes_received_c,
            on_message_received: on_message_received_c,
            on_stream_received: on_stream_received_c,
        }
    }
}

// These C interfaces are defined in lib: dsoftbus:softbus_client
extern "C" {
    /// Creates a session server.
    ///
    /// # Arguments
    ///
    /// * `pkg_name` - The package name of the server.
    /// * `session_name` - The name of the session.
    /// * `session_listener` - A pointer to the session listener.
    ///
    /// # Returns
    ///
    /// The session ID if successful, otherwise an error code.
    pub fn CreateSessionServer(pkg_name: *const c_char, session_name: *const c_char,
        session_listener: *const ISessionListener) -> i32;

    /// Removes a session server.
    ///
    /// # Arguments
    ///
    /// * `pkg_name` - The package name of the server.
    /// * `session_name` - The name of the session.
    ///
    /// # Returns
    ///
    /// An error code indicating the result of the operation.
    pub fn RemoveSessionServer(pkg_name: *const c_char, session_name: *const c_char) -> i32;

    /// Closes a session.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session to be closed.
    pub fn CloseSession(session_id: i32);

    /// Opens a session.
    ///
    /// # Arguments
    ///
    /// * `my_session_name` - The name of the local session.
    /// * `peer_session_name` - The name of the remote session.
    /// * `peer_device_id` - The ID of the remote device.
    /// * `groupId` - The group ID of the session.
    /// * `attr` - A pointer to the session attribute.
    ///
    /// # Returns
    ///
    /// The session ID if successful, otherwise an error code.
    pub fn OpenSession(my_session_name: *const c_char, peer_session_name: *const c_char, peer_device_id: *const c_char,
        groupId: *const c_char, attr: *const SessionAttribute) -> i32;

    /// Gets the ID of the remote device for a given session.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session.
    /// * `peer_dev_id` - A mutable buffer to store the remote device ID.
    /// * `len` - The length of the buffer.
    ///
    /// # Returns
    ///
    /// An error code indicating the result of the operation.
    pub fn GetPeerDeviceId(session_id: i32, peer_dev_id: *mut c_char, len: u32) -> i32;

    /// Gets the session side (server or client) for a given session.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session.
    ///
    /// # Returns
    ///
    /// The session side if successful, otherwise an error code.
    pub fn GetSessionSide(session_id: i32) -> i32;

    /// Gets the local node's device information.
    ///
    /// # Arguments
    ///
    /// * `pkg_name` - The package name of the local node.
    /// * `info` - A mutable pointer to the node's basic information structure.
    ///
    /// # Returns
    ///
    /// An error code indicating the result of the operation.
    pub fn GetLocalNodeDeviceInfo(pkg_name: *const c_char, info: *mut NodeBasicInfo) -> i32;

    /// Sends bytes over a session.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session.
    /// * `data` - A pointer to the data to be sent.
    /// * `len` - The length of the data.
    ///
    /// # Returns
    ///
    /// An error code indicating the result of the operation.
    pub fn SendBytes(session_id: i32, data: *const c_void, len: u32) -> i32;
}