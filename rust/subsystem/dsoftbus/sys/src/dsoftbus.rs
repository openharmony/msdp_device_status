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

//! rust dsoftbus sys.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{c_void, c_char, CString, CStr};
use std::sync::{Once, Mutex, Arc, Condvar};
use std::collections::{HashMap, HashSet};
use std::time::Duration;
use std::hash::{Hash, Hasher};
use std::vec::Vec;
use std::ptr;
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode};
use hilog_rust::{ error, info, hilog, HiLogLabel, LogType };
use crate::binding::{
    INTERCEPT_STRING_LENGTH,
    DINPUT_LINK_TYPE_MAX,
    DEVICE_ID_SIZE_MAX,
    SESSION_SIDE_SERVER,
    LINK_TYPE_WIFI_WLAN_5G,
    LINK_TYPE_WIFI_WLAN_2G,
    LINK_TYPE_WIFI_P2P,
    LINK_TYPE_BR,
    TYPE_BYTES,
    NETWORK_ID_BUF_LEN,
    DEVICE_NAME_BUF_LEN,
    RET_OK,
    RET_ERROR,
    C_CHAR_SIZE,
    ISessionListener,
    StreamData,
    StreamFrameInfo,
    SessionAttribute,
    NodeBasicInfo,
    GetPeerDeviceId,
    GetSessionSide,
    OpenSession,
    CloseSession,
    CreateSessionServer,
    RemoveSessionServer,
    GetLocalNodeDeviceInfo,
    SendBytes,
};

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DSoftbus"
};

/// Provide for C lib to call
extern "C" fn on_session_opened(session_id: i32, result: i32) -> i32 {
    if let Some(dsoftbus) = DSoftbus::get_instance() {
        if let Err(err) = dsoftbus.on_session_opened(session_id, result) {
            RET_ERROR
        } else {
            RET_OK
        }
    } else {
        error!(LOG_LABEL, "DSoftbus is none");
        FusionErrorCode::Fail.into()
    }
}

/// Provide for C lib to call
extern "C" fn on_session_closed(session_id: i32) {
    if let Some(dsoftbus) = DSoftbus::get_instance() {
        dsoftbus.on_session_closed(session_id);
        } else {
        error!(LOG_LABEL, "DSoftbus is none");
    }
}

/// Provide for C lib to call
extern "C" fn on_bytes_received(session_id: i32, data: *const c_void, data_len: u32) {
    if let Some(dsoftbus) = DSoftbus::get_instance() {
        dsoftbus.on_bytes_received(session_id, data, data_len);
    } else {
        error!(LOG_LABEL, "DSoftbus is none");
    }
}

/// Provide for C lib to call
extern "C" fn on_message_received(session_id: i32, byte_data: *const c_void, data_len: u32) {
}

/// Provide for C lib to call
extern "C" fn on_stream_received(session_id: i32, byte_data: *const StreamData,
    ext_data: *const StreamData, param_data: *const StreamFrameInfo) {
}

/// Callback trait used for handling events in the DSoftbus instance.
pub trait IDSoftbufCallback {
    /// Handles the event when a session is closed.
    fn on_session_closed(&self, device_id: &str);

    /// Handles some things when messages are received from a session.
    fn on_handle_msg(&self, session_id: i32, data: &str);
}

impl Hash for Box<dyn IDSoftbufCallback> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        let raw_ptr = self.as_ref() as *const dyn IDSoftbufCallback;
        raw_ptr.hash(state);
    }
}

impl PartialEq for Box<dyn IDSoftbufCallback> {
    fn eq(&self, other: &Self) -> bool {
        let self_data_ptr = self.as_ref() as *const dyn IDSoftbufCallback as *const ();
        let other_data_ptr = other.as_ref() as *const dyn IDSoftbufCallback as *const ();
        ptr::eq(self_data_ptr, other_data_ptr)
    }
}

impl Eq for Box<dyn IDSoftbufCallback> {}

/// Inner is a struct that represents inner data.
#[derive(Default)]
struct Inner {
    /// The session ID.
    session_id: i32,
    /// The session listener.
    session_listener: ISessionListener,
    /// The local session name.
    local_session_name: String,
    /// The session device map.
    session_dev_map: HashMap<String, i32>,
    /// The channel status map.
    channel_status_map: HashMap<String, bool>,
    /// The operation mutex.
    operation_mutex: Mutex<HashMap<String, i32>>,
    /// The wait condition.
    wait_cond: Arc<(Mutex<bool>, Condvar)>,
    /// The set of callback functions.
    callback: HashSet<Box<dyn IDSoftbufCallback>>,
}

impl Inner {
    /// Initializes the `DSoftbus` instance.
    ///
    /// # Returns
    ///
    /// A `FusionResult` which can either be `Ok(())` if the operation was successful or an error wrapped in a
    /// `FusionErrorCode`.
    ///
    /// # Example
    ///
    /// ```
    /// let mut softbus = DSoftbus::default();
    /// softbus.init();
    /// ```
    fn init(&mut self) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::init");
        let session_name = self.get_session_name()?;

        if self.local_session_name.eq(&session_name) {
            info!(LOG_LABEL, "Session server has already created");
            return Ok(());
        }

        let fi_pkg_name: String = String::from("ohos.msdp.fusioninteraction");
        if !self.local_session_name.is_empty() {
            error!(LOG_LABEL, "Remove last sesison server, sessionName:{}", @public(self.local_session_name));
            // SAFETY: no `None` here, cause `fi_pkg_name` and `local_session_name` is valid.
            let ret: i32 = unsafe { RemoveSessionServer(fi_pkg_name.as_ptr(), self.local_session_name.as_ptr()) };
            if ret != RET_OK {
                error!(LOG_LABEL, "Remove session server failed, error code:{}", @public(ret));
            }
        }

        self.local_session_name = session_name;
        let session_listener = self.create_session_listener(
            Some(on_session_opened),
            Some(on_session_closed),
            Some(on_bytes_received),
            Some(on_message_received),
            Some(on_stream_received),
        ).map_err (|_| {
            error!(LOG_LABEL, "Create session_listener failed");
            FusionErrorCode::Fail
        })?;

        // SAFETY: no `None` here, cause `fi_pkg_name`、`local_session_name` and `session_listener` is valid.
        let ret: i32 = unsafe { CreateSessionServer(fi_pkg_name.as_ptr(), self.local_session_name.as_ptr(),
            &self.session_listener) };
        if ret != RET_OK {
            error!(LOG_LABEL, "Create session server failed, error code:{}", @public(ret));
            return Err(FusionErrorCode::Fail);
        }
        Ok(())
    }

    /// Create a session listener object with the given callback functions.
    ///
    /// # Arguments
    ///
    /// * `on_session_opened` - Callback function for session opened event.
    /// * `on_session_closed` - Callback function for session closed event.
    /// * `on_bytes_received` - Callback function for bytes received event.
    /// * `on_message_received` - Callback function for message received event.
    /// * `on_stream_received` - Callback function for stream received event.
    ///
    /// # Returns
    ///
    /// Returns a `Result` containing the created `ISessionListener` object if successful,
    /// or an error code if any of the callback functions are NULL.
    fn create_session_listener(&mut self,
        on_session_opened_ptr: Option<extern "C" fn(i32, i32) -> i32>,
        on_session_closed_ptr: Option<extern "C" fn(i32)>,
        on_bytes_received_ptr: Option<extern "C" fn(i32, *const c_void, u32)>,
        on_message_received_ptr: Option<extern "C" fn(i32, *const c_void, u32)>,
        on_stream_received_ptr: Option<extern "C" fn(i32, *const StreamData, *const StreamData, *const StreamFrameInfo)>
    ) -> FusionResult<ISessionListener> {
        let session_listener = ISessionListener {
            on_session_opened: on_session_opened_ptr.ok_or(FusionErrorCode::Fail)?,
            on_session_closed: on_session_closed_ptr.ok_or(FusionErrorCode::Fail)?,
            on_bytes_received: on_bytes_received_ptr.ok_or(FusionErrorCode::Fail)?,
            on_message_received: on_message_received_ptr.ok_or(FusionErrorCode::Fail)?,
            on_stream_received: on_stream_received_ptr.ok_or(FusionErrorCode::Fail)?,
        };

        Ok(session_listener)
    }

    /// This function is used to generate a session name by concatenating a fixed prefix `ohos.msdp.device_status` and
    /// local network ID. The function returns the session name as a `String`.
    ///
    /// # Arguments
    ///
    /// * `&mut self` - A mutable reference to the current instance of the struct that holds this function.
    ///
    /// # Returns
    ///
    /// * `FusionResult<String>` - The generated session name, wrapped in a `FusionResult` indicating whether the
    /// operation was successful or not.
    ///
    /// # Errors
    ///
    /// This function could fail if there is no local network ID available, in which case a `FusionErrorCode::Fail`
    /// error code is returned.
    ///
    fn get_session_name(&mut self) -> FusionResult<String> {
        let session_name_head = String::from("ohos.msdp.device_status");

        let local_network_id = self.local_network_id().map_err(|_| {
            error!(LOG_LABEL, "Local_network_id is empty");
            FusionErrorCode::Fail
        })?;
        println!("local_network_id= {}", local_network_id);
        if local_network_id.len() >= INTERCEPT_STRING_LENGTH {
            let local_network_id_slice = local_network_id[0..INTERCEPT_STRING_LENGTH].to_string();
            Ok(session_name_head + &local_network_id_slice)
        } else {
            error!(LOG_LABEL, "Length of local_network_id less than 20");
            Err(FusionErrorCode::Fail)
        }
    }

    /// Retrieves the local network ID.
    ///
    /// # Returns
    ///
    /// A `FusionResult` which can either be `Ok(String)` containing the local network ID if the operation was
    /// successful or an error wrapped in a `FusionErrorCode`.
    ///
    /// # Example
    ///
    /// ```
    /// let mut softbus = DSoftbus::default();
    /// let local_network_id = softbus.local_network_id();
    /// ```
    fn local_network_id(&mut self) -> FusionResult<String> {
        call_debug_enter!("DSoftbus::local_network_id");
        let mut local_node = NodeBasicInfo {
            network_id: [0; NETWORK_ID_BUF_LEN],
            device_name: [0; DEVICE_NAME_BUF_LEN],
            device_type_id: 0,
        };

        let fi_pkg_name: String = String::from("ohos.msdp.fusioninteraction");
        // SAFETY: no `None` here, cause `fi_pkg_name`、`local_node` is valid.
        let ret: i32 = unsafe { GetLocalNodeDeviceInfo(fi_pkg_name.as_ptr() as *const c_char,
            &mut local_node as *mut NodeBasicInfo) };
        if ret != RET_OK {
            error!(LOG_LABEL, "GetLocalNodeDeviceInfo result:{}", @public(ret));
            return Err(FusionErrorCode::Fail);
        }

        // let network_id_ptr: *const c_char = local_node.network_id.as_ptr() as *const c_char;
        // // SAFETY: no `None` here, cause `network_id_ptr` is valid.
        // let network_id_str = unsafe {CStr::from_ptr(network_id_ptr)};
        // let network_id_slice: &str = network_id_str.to_str().unwrap();
        // let network_id = network_id_slice.to_owned();
        // if network_id.is_empty() {
        //     error!(LOG_LABEL, "Local network id is empty");
        //     return Err(FusionErrorCode::Fail.into());
        // }
        let network_id = self.convert_i8_array_to_string(&local_node.network_id);
        Ok(network_id)
    }

    /// Converts an array of signed 8-bit integers to a String.
    ///
    /// # Arguments
    ///
    /// * `arr` - The input array containing i8 values.
    ///
    /// # Returns
    ///
    /// A new String containing the UTF-8 encoded characters from the input array.
    ///
    /// # Examples
    ///
    /// ```
    /// let arr: [i8; 5] = [72, 101, 108, 108, 111];
    /// let result = convert_i8_array_to_string(&arr);
    /// assert_eq!(result, "Hello");
    /// ```
    fn convert_i8_array_to_string(&self, arr: &[i8]) -> String {
        let u8_arr: Vec<u8> = arr.iter().map(|&x| x as u8).collect();
        String::from_utf8_lossy(&u8_arr).to_string()
    }

    /// Releases all resources associated with the `DSoftbus` instance.
    ///
    /// # Example
    ///
    /// ```
    /// let mut softbus = DSoftbus::default();
    /// softbus.release();
    /// ```
    fn release(&mut self) {
        call_debug_enter!("DSoftbus::release");
        for (_key, value) in self.session_dev_map.iter() {
            // SAFETY: no `None` here, if `session_dev_map` is empty, the function will not be called.
            unsafe { CloseSession(*value) };
        }
        let fi_pkg_name: String = String::from("ohos.msdp.fusioninteraction");
        // SAFETY: no `None` here, cause `fi_pkg_name` and `local_session_name` is valid.
        unsafe { RemoveSessionServer(fi_pkg_name.as_ptr(), self.local_session_name.as_ptr()) };

        self.session_dev_map.clear();
        self.channel_status_map.clear();
    }

    /// Generates a peer session name based on the provided remote network ID.
    /// The generated session name is a concatenation of a fixed session name and the sliced remote network ID.
    /// If the length of the remote network ID is less than INTERCEPT_STRING_LENGTH, an error is returned.
    ///
    /// # Arguments
    ///
    /// * `remote_network_id` - A reference to a String representing the remote network ID.
    ///
    /// # Returns
    ///
    /// - `Ok(peer_session_name)` if the peer session name is successfully generated.
    /// - `Err(FusionErrorCode::Fail)` if the length of the remote network ID is less than INTERCEPT_STRING_LENGTH.
    fn get_peer_session_name(&mut self, remote_network_id: &String) -> FusionResult<String>{
        if remote_network_id.len() >= INTERCEPT_STRING_LENGTH {
            let session_name = String::from("ohos.msdp.device_status");
            let remote_network_id_slice = remote_network_id[0..INTERCEPT_STRING_LENGTH].to_string();
            Ok(session_name + &remote_network_id_slice)
        } else {
            error!(LOG_LABEL, "Length of remote_network_id less than 20");
            Err(FusionErrorCode::Fail)
        }
    }
    /// Opens the input softbus connection for the specified remote network ID.
    ///
    /// # Arguments
    ///
    /// * `remote_network_id` - A `String` representing the ID of the remote network.
    ///
    /// # Returns
    ///
    /// A `FusionResult` which can either be `Ok(())` if the operation was successful or an error wrapped in a
    /// `FusionErrorCode`.
    ///
    /// # Example
    ///
    /// ```
    /// let mut softbus = DSoftbus::default();
    /// softbus.open_input_softbus(&"network_id".to_string());
    /// ```
    fn open_input_softbus(&mut self, remote_network_id: &String) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::open_input_softbus");
        if self.check_device_session_state(remote_network_id) {
            error!(LOG_LABEL, "Softbus session has already opened");
            return Ok(());
        }

        self.init()?;
     
        let data: u8 = 0;
        let session_attr = SessionAttribute {
            data_type: TYPE_BYTES,
            link_type_num: DINPUT_LINK_TYPE_MAX,
            link_type: [LINK_TYPE_WIFI_WLAN_2G, LINK_TYPE_WIFI_WLAN_5G, LINK_TYPE_WIFI_P2P,
                        LINK_TYPE_BR, 0, 0, 0, 0, 0],
            stream_attr: 0,
            fast_trans_data:data as *const u8,
            fast_trans_data_size: 0,
        };

        let peer_session_name = self.get_peer_session_name(remote_network_id)?;
        let group_id = String::from("fi_softbus_group_id");
        // SAFETY: no `None` here, `local_session_name` is initialized in `init()`, `peer_session_name`,
        // `remote_network_id`, `group_id` and `session_attr` is valid.
        let session_id = unsafe { OpenSession(self.local_session_name.as_ptr() as *const c_char,
            peer_session_name.as_ptr() as *const c_char, remote_network_id.as_ptr() as *const c_char,
            group_id.as_ptr() as *const c_char, &session_attr as *const SessionAttribute) };
        if session_id < 0 {
            error!(LOG_LABEL, "OpenSession failed, session_id:{}", @public(session_id));
            return Err(FusionErrorCode::Fail);
        }

        self.wait_session_opend(remote_network_id, session_id)
    }

    /// Closes the input softbus connection for the specified remote network ID.
    ///
    /// # Arguments
    ///
    /// * `remote_network_id` - A `String` representing the ID of the remote network.
    ///
    /// # Example
    ///
    /// ```
    /// let mut softbus = DSoftbus::default();
    /// softbus.close_input_softbus(&"network_id".to_string());
    /// ```
    fn close_input_softbus(&mut self, remote_network_id: &String) {
        call_debug_enter!("DSoftbus::close_input_softbus");
        if let Some(session_id) = self.session_dev_map.get(remote_network_id) {
            // SAFETY: no `None` here, `session_id` is valid.
            unsafe { CloseSession(*session_id) };
        } else {
            error!(LOG_LABEL, "SessionDevIdMap not found");
            return;
        }
        self.session_dev_map.remove(remote_network_id);
        self.channel_status_map.remove(remote_network_id);
        self.session_id = -1;
    }

    /// Waits for a session to be opened with the given remote network ID and session ID.
    ///
    /// # Arguments
    ///
    /// * `remote_network_id` - A `String` representing the ID of the remote network.
    /// * `session_id` - An integer representing the ID of the session.
    ///
    /// # Returns
    ///
    /// A `FusionResult` which can either be `Ok(())` if the operation was successful or an error wrapped in a
    /// `FusionErrorCode`.
    ///
    /// # Example
    ///
    /// ```
    /// let mut softbus = DSoftbus::default();
    /// softbus.wait_session_opend(&"network_id".to_string(), 1);
    /// ```
    fn wait_session_opend(&mut self, remote_network_id: &String, session_id_: i32) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::wait_session_opend");
        self.session_dev_map.insert(remote_network_id.to_string(), session_id_);
        self.wait_cond = Arc::new((Mutex::new(false), Condvar::new()));
        let pair = Arc::clone(&self.wait_cond);
        let (lock, cvar) = &*pair;
        let result = (cvar.wait_timeout(self.operation_mutex.lock().unwrap(), Duration::from_secs(5))).unwrap();

        let get_result = self.channel_status_map.get(remote_network_id);
        if get_result.is_some() && !get_result.copied().unwrap() {
            error!(LOG_LABEL, "OpenSession timeout");
            return Err(FusionErrorCode::Fail);
        }
        self.channel_status_map.insert(remote_network_id.to_string(), false);
        Ok(())
    }

    /// Handles the event when a session is opened.
    ///
    /// # Arguments
    ///
    /// * `session_id` - An integer representing the ID of the session.
    /// * `result` - An integer indicating the result of the session opening.
    ///
    /// # Returns
    ///
    /// A `FusionResult` which can either be `Ok(())` if the operation was successful or an error wrapped in a
    /// `FusionErrorCode`.
    ///
    /// # Example
    ///
    /// ```
    /// let mut softbus = DSoftbus::default();
    /// let session_id: i32 = 1;
    /// softbus.on_session_opened(1, RET_OK);
    /// ```
    fn on_session_opened(&mut self, session_id: i32, result: i32) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::on_session_opened");
        self.session_id = session_id;
        let mut peer_dev_id: Vec<c_char> = Vec::with_capacity(65);
        peer_dev_id.extend(vec![0; 65]);
        let peer_dev_id_ptr = peer_dev_id.as_mut_ptr() as *mut c_char;
        let len: u32 = (C_CHAR_SIZE * DEVICE_ID_SIZE_MAX) as u32;

        // SAFETY: Assumes valid input arguments and that `peer_dev_id_ptr` points to a memory block of at least `len`
        // bytes. Caller must ensure these conditions for correct behavior and to prevent memory issues or security
        // vulnerabilities.
        let get_peer_device_id_result: i32 = unsafe { GetPeerDeviceId(session_id, peer_dev_id_ptr, len) };
        
        let peer_dev_id_str = unsafe { CStr::from_ptr(peer_dev_id_ptr) };
        let peer_dev_id: String = peer_dev_id_str.to_string_lossy().into_owned();

        if result != RET_OK {
            let device_id = self.find_device(session_id).map_err(|_|{
                error!(LOG_LABEL, "find_device error");
                FusionErrorCode::Fail
            })?;

            if let Some(value) = self.session_dev_map.get(&device_id) {
                self.session_dev_map.remove(&device_id);
            }

            if get_peer_device_id_result == RET_OK {
                self.channel_status_map.insert(peer_dev_id, true);
            }
            self.wait_cond.1.notify_all();
            return Ok(());
        }
        // SAFETY: This function does not care about the boundary value of the input.
        let session_side: i32 = unsafe { GetSessionSide(session_id) };
        if session_side == SESSION_SIDE_SERVER {
            if get_peer_device_id_result == RET_OK {
                self.session_dev_map.insert(peer_dev_id, session_id);
            }
        }
        else if get_peer_device_id_result == RET_OK {
            self.channel_status_map.insert(peer_dev_id, true);
            self.wait_cond.1.notify_all();
        }
        Ok(())
    }

    /// Finds the device ID associated with a session ID.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session.
    ///
    /// # Returns
    ///
    /// The device ID associated with the session ID, wrapped in a `Result`. If the device ID is found, it is returned
    /// as an `Ok` variant. If the device ID is not found, an error is returned as an `Err` variant.
    ///
    /// # Example
    ///
    /// ```rust
    /// let dsoftbus = DSoftbus::default();
    /// let session_id = 123;
    /// match dsoftbus.find_device(session_id) {
    ///     Ok(device_id) => {
    ///         println!("Device ID: {}", device_id);
    ///     },
    ///     Err(err) => {
    ///         println!("Error finding device: {:?}", err);
    ///     }
    /// }
    /// ```
    fn find_device(&self, session_id: i32) -> FusionResult<String> {
        call_debug_enter!("DSoftbus::find_device");
        for (key, value) in self.session_dev_map.iter() {
            if *value == session_id {
                return Ok(key.to_string());
            }
        }
        error!(LOG_LABEL, "find_device error");
        Err(FusionErrorCode::Fail)
    }

    /// Handles the event when a session is closed.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session that was closed.
    ///
    /// # Example
    ///
    /// ```rust
    /// let mut dsoftbus = DSoftbus::default();
    /// let session_id = 123;
    /// dsoftbus.on_session_closed(session_id);
    /// ```
    fn on_session_closed(&mut self, session_id: i32) {
        call_debug_enter!("DSoftbus::on_session_closed");
        let device_id = match self.find_device(session_id) {
            Ok(device_id) => device_id,
            Err(err) => {
                error!(LOG_LABEL, "find_device error");
                return;
            }
        };
        if let Some(value) = self.session_dev_map.get(&device_id) {
            self.session_dev_map.remove(&device_id);
        }
        // SAFETY: This function does not care about the boundary value of the input.
        if unsafe { GetSessionSide(session_id) } != RET_OK {
            self.channel_status_map.remove(&device_id);
        }

        for callback in &self.callback {
            callback.on_session_closed(&device_id);
        }

        self.session_id = -1;
    }

    /// Handles received bytes when bytes are received from a session.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session.
    /// * `data` - A pointer to the received data.
    /// * `data_len` - The length of the received data in bytes.
    ///
    /// # Safety
    ///
    /// This function is marked as unsafe because it accesses and interprets raw pointers and assumes the validity of
    /// the `data` parameter. The caller needs to ensure that `session_id` is a valid session ID, and `data` is a valid
    /// pointer to the received data.
    ///
    /// # Example
    ///
    /// ```rust
    /// let dsoftbus = DSoftbus::default();
    /// let session_id = 123;
    /// let data: *const c_void = ...; // Initialize the data pointer
    /// let data_len = 10;
    /// dsoftbus.on_bytes_received(session_id, data, data_len);
    /// ```
    fn on_bytes_received(&self, session_id: i32, data: *const c_void, data_len: u32) {
        call_debug_enter!("DSoftbus::on_bytes_received");
        if session_id < 0 || data.is_null() || data_len == 0 {
           error!(LOG_LABEL, "Param check failed");
        }

        // SAFETY: no `None` here, cause `network_id_ptr` is valid.
        let data_str = unsafe {CStr::from_ptr(data as *const c_char)};
        let data_slice: &str = data_str.to_str().unwrap();
        let data: String = data_slice.to_owned();

        for callback in &self.callback {
            callback.on_handle_msg(session_id, &data);
        }
    }

    /// Checks the session state of a remote device.
    ///
    /// # Arguments
    ///
    /// * `remote_network_id` - A reference to a String representing the network ID of the remote device.
    ///
    /// # Returns
    ///
    /// Returns `true` if the session state of the remote device exists in the session device map, or `false` otherwise.
    ///
    /// # Example
    ///
    /// ```rust
    /// let dsoftbus = DSoftbus::default();
    /// let remote_network_id = "target_network".to_string();
    /// let session_state = dsoftbus.check_device_session_state(&remote_network_id);
    /// if session_state {
    ///     println!("The session state of the remote device exists.");
    /// } else {
    ///     println!("The session state of the remote device does not exist.");
    /// }
    /// ```
    fn check_device_session_state(&self, remote_network_id: &String) -> bool {
        call_debug_enter!("DSoftbus::check_device_session_state");
        if let Some(value) = self.session_dev_map.get(remote_network_id) {
            true
        } else {
            error!(LOG_LABEL, "Check session state error");
            false
        }
    }

    /// Sends a message to a specified device.
    ///
    /// # Arguments
    ///
    /// * `device_id` - A reference to a String representing the ID of the target device.
    /// * `data` - A pointer to the data to be sent.
    /// * `data_len` - The length of the data in bytes.
    ///
    /// # Returns
    ///
    /// Returns `Ok(())` if the message was sent successfully, or an error of type `FusionResult<()>` if sending the
    /// message failed.
    ///
    /// # Example
    ///
    /// ```rust
    /// let dsoftbus = DSoftbus::default();
    /// let device_id = "target_device".to_string();
    /// let data: *const c_void = ...; // Initialize the data pointer
    /// let data_len = 10;
    /// dsoftbus.send_msg(&device_id, data, data_len).unwrap();
    /// ```
    fn send_msg(&self, device_id: &String, data: *const c_void, data_len: u32) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::send_msg");
        if let Some(session_id) = self.session_dev_map.get(device_id) {
            // SAFETY: no `None` here, `session_id`, `data` and `data_len` is valid.
            let result: i32 = unsafe {SendBytes(*session_id, data, data_len) };
            if result != RET_OK {
                error!(LOG_LABEL, "Send bytes failed, result:{}", @public(result));
                return Err(FusionErrorCode::Fail);
            }
            Ok(())
        } else {
            error!(LOG_LABEL, "Check session state error");
            Err(FusionErrorCode::Fail)
        }
    }

    /// Returns a clone of the session device map.
    ///
    /// The session device map is a HashMap that maps session IDs to device IDs. Each session ID is represented as a
    /// String, and each device ID is represented as an i32.
    ///
    /// # Returns
    ///
    /// Returns a clone of the session device map.
    ///
    /// # Example
    ///
    /// ```rust
    /// let dsoftbus = DSoftbus::default();
    /// let session_dev_map = dsoftbus.get_session_dev_map();
    /// // Use the session_dev_map here...
    /// ```
    fn get_session_dev_map(&self) -> HashMap<String, i32> {
        self.session_dev_map.clone()
    }

    /// Registers a callback to receive SoftBus events.
    ///
    /// # Arguments
    ///
    /// * `callback` - A Boxed trait object implementing the IDSoftbufCallback trait. This callback will be called when
    /// SoftBus events occur.
    ///
    /// # Example
    ///
    /// ```rust
    /// let mut dsoftbus = DSoftbus::default();
    /// let callback = Box::new(MyCallback {});
    /// dsoftbus.register_callback(callback);
    /// ```
    fn register_callback(&mut self, callback: Box<dyn IDSoftbufCallback>) {
        call_debug_enter!("DeviceProfileAdapter::register_callback");
        self.callback.insert(callback);
    }
}

/// DSoftbus is a struct that represents the DSoftbus object.
#[derive(Default)]
pub struct DSoftbus {
    /// The implementation of DSoftbus.
    dsoftbus_impl: Mutex<Inner>,
}

impl DSoftbus {
    /// Returns a reference to the SoftBus singleton instance.
    ///
    /// # Returns
    ///
    /// Returns an `Option<&'static Self>` containing a reference to the SoftBus singleton instance, or `None` if the
    /// singleton has not yet been initialized.
    ///
    /// # Safety
    ///
    /// This function is marked as unsafe because it modifies static variables. It is expected that the caller is aware
    /// of the risks and takes appropriate measures to ensure safety.
    ///
    /// # Example
    ///
    /// ```rust
    /// match DSoftbus::get_instance() {
    ///     Some(instance) => {
    ///         println!("SoftBus singleton instance found.");
    ///         // Use the instance here...
    ///     }
    ///     None => {
    ///         println!("SoftBus singleton instance not yet initialized.");
    ///     }
    /// }
    /// ```
    pub fn get_instance() -> Option<&'static Self> {
        static mut G_DSOFTBUS: Option<DSoftbus> = None;
        static INIT_ONCE: Once = Once::new();
        // SAFETY: no `None` here. just Modifying the Static Variables
        unsafe {
            INIT_ONCE.call_once(|| {
                G_DSOFTBUS = Some(DSoftbus::default());
            });
            G_DSOFTBUS.as_ref()
        }
    }

    /// Initializes the SoftBus instance.
    ///
    /// # Returns
    ///
    /// Returns `Result<(), FusionError>` indicating the success or failure of the initialization. An `Err` value is
    /// returned if there was a lock error during the execution.
    ///
    /// # Note
    ///
    /// This function initializes the SoftBus instance. It should be called before using any SoftBus functionality to
    /// ensure proper setup and initialization.
    ///
    /// # Example
    ///
    /// ```rust
    /// match my_instance.init() {
    ///     Ok(()) => {
    ///         println!("SoftBus initialized successfully");
    ///     }
    ///     Err(err) => {
    ///         eprintln!("Failed to initialize SoftBus: {:?}", err);
    ///     }
    /// }
    /// ```
    pub fn init(&self) -> FusionResult<()> {
        match self.dsoftbus_impl.lock() {
            Ok(mut guard) => {
                guard.init()
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Releases the resources held by the SoftBus instance.
    ///
    /// # Note
    ///
    /// This function releases the resources held by the SoftBus instance. It should be called when you no longer need
    /// to use the SoftBus functionality and want to free up any associated resources.
    ///
    /// # Example
    ///
    /// ```rust
    /// my_instance.release();
    /// ```
    pub fn release(&self) {
        match self.dsoftbus_impl.lock() {
            Ok(mut guard) => {
                guard.release();
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }

    /// Opens an input SoftBus connection for a specified remote network ID.
    ///
    /// # Arguments
    ///
    /// * `remote_network_id` - The remote network ID for which to open the input SoftBus connection.
    ///
    /// # Returns
    ///
    /// Returns `Result<(), FusionError>` indicating the success or failure of the operation. An `Err` value is
    /// returned if there was a lock error during the execution.
    ///
    /// # Note
    ///
    /// This function opens an input SoftBus connection for the specified remote network ID. It should be called when
    /// you want to establish communication with the remote network.
    ///
    /// # Example
    ///
    /// ```rust
    /// let remote_network_id = String::from("example_network_id");
    ///
    /// match my_instance.open_input_softbus(&remote_network_id) {
    ///     Ok(()) => {
    ///         println!("Input SoftBus connection opened successfully");
    ///     }
    ///     Err(err) => {
    ///         eprintln!("Failed to open input SoftBus connection: {:?}", err);
    ///     }
    /// }
    /// ```
    pub fn open_input_softbus(&self, remote_network_id: &String) -> FusionResult<()> {
        match self.dsoftbus_impl.lock() {
            Ok(mut guard) => {
                guard.open_input_softbus(remote_network_id)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Closes the input SoftBus connection for a specified remote network ID.
    ///
    /// # Arguments
    ///
    /// * `remote_network_id` - The remote network ID for which to close the input SoftBus connection.
    ///
    /// # Note
    ///
    /// This function closes the input SoftBus connection for the specified remote network ID. It should be called when
    /// you want to terminate the communication with the remote network.
    ///
    /// # Example
    ///
    /// ```rust
    /// let remote_network_id = String::from("example_network_id");
    /// my_instance.close_input_softbus(&remote_network_id);
    /// ```
    pub fn close_input_softbus(&self, remote_network_id: &String) {
        match self.dsoftbus_impl.lock() {
            Ok(mut guard) => {
                guard.close_input_softbus(remote_network_id);
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }

    /// Callback function triggered when a session is opened.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the opened session.
    /// * `result` - The result of the session opening operation.
    ///
    /// # Returns
    ///
    /// Returns `Result<(), FusionError>` indicating the success or failure of the operation. An `Err` value is
    /// returned if there was a lock error during the callback execution.
    ///
    /// # Note
    ///
    /// This function is called when a session is opened, allowing you to perform any necessary handling or operations
    /// related to the opened session. The `result` parameter provides the outcome of the session opening operation.
    ///
    /// # Example
    ///
    /// ```rust
    /// let session_id = 123;
    /// let result = 0; // Assume the session opening was successful
    ///
    /// match callback.on_session_opened(session_id, result) {
    ///     Ok(()) => {
    ///         println!("Session opened successfully");
    ///     }
    ///     Err(err) => {
    ///         eprintln!("Failed to open session: {:?}", err);
    ///     }
    /// }
    /// ```
    fn on_session_opened(&self, session_id: i32, result: i32) -> FusionResult<()> {
        match self.dsoftbus_impl.lock() {
            Ok(mut guard) => {
                guard.on_session_opened(session_id, result)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Callback function triggered when a session is closed.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the closed session.
    ///
    /// # Note
    ///
    /// This function is called when a session is closed, allowing you to perform any necessary cleanup or handling
    /// related to the closed session.
    /// 
    /// # Example
    ///
    /// ```rust
    /// let session_id = 123;
    ///
    /// callback.on_session_closed(session_id);
    /// ```
    fn on_session_closed(&self, session_id: i32) {
        match self.dsoftbus_impl.lock() {
            Ok(mut guard) => {
                guard.on_session_closed(session_id);
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }

    /// Callback function triggered when bytes are received.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session related to the received bytes.
    /// * `data` - The pointer to the received bytes data.
    /// * `data_len` - The length of the received bytes data in bytes.
    ///
    /// # Note
    ///
    /// This function is called when bytes are received, allowing you to handle and process the received data.
    /// Please note that the pointer `data` is a raw pointer that needs to be handled carefully to avoid memory safety
    /// issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    ///
    /// # Example
    ///
    /// ```rust
    /// let session_id = 123;
    /// let data_ptr: *const c_void = ...; // Obtain the actual pointer to the received bytes data
    /// let data_len = ...; // Obtain the length of the received bytes data
    ///
    /// callback.on_bytes_received(session_id, data_ptr, data_len);
    /// ```
    fn on_bytes_received(&self, session_id: i32, data: *const c_void, data_len: u32) {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.on_bytes_received(session_id, data, data_len);
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }

    /// Sends a message to the specified device.
    ///
    /// # Arguments
    ///
    /// * `device_id` - The ID of the target device to send the message to.
    /// * `data` - The pointer to the message data.
    /// * `data_len` - The length of the message data in bytes.
    ///
    /// # Returns
    ///
    /// Returns `Ok(())` if the message is successfully sent, otherwise returns an `Err` containing an error code.
    ///
    /// # Note
    ///
    /// This function sends the message to the specified device using the internal `dsoftbus_impl` instance.
    /// The pointer `data` is a raw pointer that needs to be handled carefully to avoid memory safety issues and
    /// undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    ///
    /// # Example
    ///
    /// ```rust
    /// let device_id = String::from("example_device");
    /// let data_ptr: *const c_void = ...; // Obtain the actual pointer to the message data
    /// let data_len = ...; // Obtain the length of the message data
    ///
    /// match my_instance.send_msg(&device_id, data_ptr, data_len) {
    ///     Ok(()) => println!("Message sent successfully"),
    ///     Err(err) => eprintln!("Failed to send message: {:?}", err),
    /// }
    /// ```
    pub fn send_msg(&self, device_id: &String, data: *const c_void,
        data_len: u32) -> FusionResult<()> {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.send_msg(device_id, data, data_len)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Callback function triggered when a message is received.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session related to the received message.
    /// * `data` - The pointer to the message data.
    /// * `data_len` - The length of the message data in bytes.
    ///
    /// # Note
    ///
    /// This function is called when a message is received, allowing you to handle and process the received message
    /// data.
    /// Please note that the pointer `data` is a raw pointer that needs to be handled carefully to avoid memory safety
    /// issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    ///
    /// # Example
    ///
    /// ```rust
    /// let session_id = 123;
    /// let data_ptr: *const c_void = ...; // Obtain the actual pointer to the message data
    /// let data_len = ...; // Obtain the length of the message data
    ///
    /// callback.on_message_received(session_id, data_ptr, data_len);
    /// ```
    fn on_message_received(&self, session_id: i32, data: *const c_void, data_len: u32) {
    }

    /// Callback function triggered when a stream is received.
    ///
    /// # Arguments
    ///
    /// * `session_id` - The ID of the session related to the received stream.
    /// * `data` - The pointer to the stream data.
    /// * `ext` - The pointer to the extended stream data.
    /// * `param` - The pointer to the stream frame information.
    ///
    /// # Note
    ///
    /// This function is called when a stream is received, allowing you to handle and process the received stream data.
    /// Please note that the pointers `data`, `ext`, and `param` are raw pointers that need to be handled carefully to
    /// avoid memory safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    ///
    /// # Example
    ///
    /// ```rust
    /// let session_id = 123;
    /// let data_ptr: *const StreamData = ...; // Obtain the actual pointer to the stream data
    /// let ext_ptr: *const StreamData = ...; // Obtain the actual pointer to the extended stream data
    /// let param_ptr: *const StreamFrameInfo = ...; // Obtain the actual pointer to the stream frame information
    ///
    /// callback.on_stream_received(session_id, data_ptr, ext_ptr, param_ptr);
    /// ```
    fn on_stream_received(&self, session_id: i32, data: *const StreamData,
        ext: *const StreamData, param: *const StreamFrameInfo) {
    }

    /// Get the session device mapping from the DSoftbus instance.
    ///
    /// # Returns
    /// Returns a `HashMap` that maps session IDs to device IDs.
    ///
    /// # Errors
    /// Returns an error of type `FusionError` if the lock cannot be acquired.
    ///
    /// # Example
    /// ```rust
    /// match my_instance.get_session_dev_map() {
    ///     Ok(map) => {
    ///         // Process the session device map
    ///         for (session_id, device_id) in map {
    ///             println!("Session ID: {}, Device ID: {}", session_id, device_id);
    ///         }
    ///     }
    ///     Err(err) => {
    ///         eprintln!("Error: {:?}", err);
    ///     }
    /// }
    /// ```
    pub fn get_session_dev_map(&self) -> FusionResult<HashMap<String, i32>> {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                Ok(guard.get_session_dev_map())
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail)
            }
        }
    }

    /// Register a callback function to the DSoftbus instance.
    /// 
    /// # Arguments
    /// - `callback`: A callback function that implements the `IDSoftbufCallback` trait.
    /// 
    /// # Returns
    /// This function does not return anything.
    ///
    /// # Example
    /// ```rust
    /// let callback = Box::new(MyCallback {});
    /// my_instance.register_callback(callback);
    /// ```
    pub fn register_callback(&self, callback: Box<dyn IDSoftbufCallback>) {
        match self.dsoftbus_impl.lock() {
            Ok(mut guard) => {
                guard.register_callback(callback);
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }
}