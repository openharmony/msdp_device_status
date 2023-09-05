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

use std::cell::RefCell;
use std::ffi::{c_void, c_char, CString, CStr};
use std::sync::{Once, Mutex, Arc, Condvar};
use std::collections::HashMap;
use std::time::Duration;
use std::vec::Vec;
use fusion_data_rust::{FusionResult, FusionErrorCode};
use fusion_utils_rust::{ call_debug_enter };
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
    ISessionListener,
    StreamData,
    StreamFrameInfo,
    SessionAttribute,
    NodeBasicInfo,
    MessageId,
    GetPeerDeviceId,
    GetSessionSide,
    OpenSession,
    CloseSession,
    CreateSessionServer,
    RemoveSessionServer,
    GetLocalNodeDeviceInfo,
    SendBytes,
    CGetHandleCb,
    OnHandleRecvData,
    CReset,
};

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DSoftbus"
};

/// Provide for C lib to call
/// # Safety
extern "C" fn on_session_opened(session_id: i32, result: i32) -> i32 {
    match DSoftbus::get_instance() {
        Some(dsoftbus) => {
            match dsoftbus.on_session_opened(session_id, result) {
                Ok(_) => { RET_OK },
                Err(err) => { err },
            }
        }
        None => {
            error!(LOG_LABEL, "DSoftbus is none");
            FusionErrorCode::Fail.into()
        }
    }
}

/// Provide for C lib to call
/// # Safety
extern "C" fn on_session_closed(session_id: i32) {
    match DSoftbus::get_instance() {
        Some(dsoftbus) => {
            dsoftbus.on_session_closed(session_id);
        }
        None => {
            error!(LOG_LABEL, "DSoftbus is none");
        }
    }
}

/// Provide for C lib to call
/// # Safety
extern "C" fn on_bytes_received(session_id: i32, data: *const c_void, data_len: u32) {
    match DSoftbus::get_instance() {
        Some(dsoftbus) => {
            dsoftbus.on_bytes_received(session_id, data, data_len);
        }
        None => {
            error!(LOG_LABEL, "DSoftbus is none");
        }
    }
}
    
/// Provide for C lib to call
/// # Safety
extern "C" fn on_message_received(session_id: i32, data: *const c_void, data_len: u32) {
    let _id = session_id;
    let _dt = data;
    let _len = data_len;
}

/// Provide for C lib to call
/// # Safety
extern "C" fn on_stream_received(session_id: i32, data: *const StreamData, ext: *const StreamData,
    param: *const StreamFrameInfo) {
    let _id = session_id;
    let _dt = data;
    let _ext1 = ext;
    let _param1 = param;
}

impl From<MessageId> for i32 {
    fn from(value: MessageId) -> Self {
        match value {
            MessageId::MinId => 0,
            MessageId::DraggingData => 1,
            MessageId::StopdragData => 2,
            MessageId::IsPullUp => 3,
            MessageId::MaxId => 50,
        }
    }
}

/// TODO: add documentation.
#[derive(Default)]
struct Inner {
    session_id: i32,
    sess_listener: ISessionListener,
    local_session_name: String,
    session_dev_map: HashMap<String, i32>,
    channel_status_map: HashMap<String, bool>,
    operation_mutex: Mutex<HashMap<String, i32>>,
    wait_cond: Arc<(Mutex<bool>, Condvar)>,
    call_back_handle_msg: Option<OnHandleRecvData>,
}

impl Inner {
    /// TODO: add documentation.
    fn init(&mut self) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::init");

        // SAFETY: no `None` here
        match unsafe { CGetHandleCb() } {
            Some(callback) => {
                self.call_back_handle_msg = Some(callback);
            }
            None => {
                error!(LOG_LABEL, "call_back_handle_msg is null ");
                return Err(FusionErrorCode::Fail.into());
            }
        };

        let session_name_head = String::from("ohos.msdp.device_status");
        let local_network_id = match self.local_network_id() {
            Ok(local_network_id) => local_network_id,
            Err(err) => {
                error!(LOG_LABEL, "Local networkid is empty");
                return Err(FusionErrorCode::Fail.into());
            }
        };

        let mut local_network_id_slice = String::default();
        if local_network_id.len() > INTERCEPT_STRING_LENGTH {
            local_network_id_slice = local_network_id[0..INTERCEPT_STRING_LENGTH].to_string();
        }
        let session_name = session_name_head + &local_network_id_slice;
        if self.local_session_name.eq(&session_name) {
            info!(LOG_LABEL, "Session server has already created");
            return Ok(());
        }

        let fi_pkg_name: String = String::from("ohos.msdp.fusioninteraction");
        if !self.local_session_name.is_empty() {
            error!(LOG_LABEL, "Remove last sesison server, sessionName:{}", @public(self.local_session_name));
            // SAFETY: no `None` here, cause `fi_pkg_name` and `local_session_name` is valid.
            let ret = unsafe { RemoveSessionServer(fi_pkg_name.as_ptr(), self.local_session_name.as_ptr()) };
            if ret != RET_OK {
                error!(LOG_LABEL, "Remove session server failed, error code:{}", @public(ret));
            }
        } 

        self.local_session_name = session_name;  
        self.sess_listener = ISessionListener {
            on_session_opened: Some(on_session_opened),
            on_session_closed: Some(on_session_closed),
            on_bytes_received: Some(on_bytes_received),
            on_message_received: None,
            on_stream_received: None,
        };
        // SAFETY: no `None` here, cause `fi_pkg_name`、`local_session_name` and `sess_listener` is valid.
        let ret: i32 = unsafe { CreateSessionServer(fi_pkg_name.as_ptr(), self.local_session_name.as_ptr(),
            &self.sess_listener) };
        if ret != RET_OK {
            error!(LOG_LABEL, "Create session server failed, error code:{}", @public(ret));
            return Err(FusionErrorCode::Fail.into());
        }
        Ok(())
    }

    /// TODO: add documentation.
    fn local_network_id(&mut self) -> FusionResult<String> {
        call_debug_enter!("DSoftbus::local_network_id");
        let mut local_node = NodeBasicInfo {
            network_id: [0; NETWORK_ID_BUF_LEN],
            device_name: [0; DEVICE_NAME_BUF_LEN],
            device_type_id: 0,
        };

        let fi_pkg_name: String = String::from("ohos.msdp.fusioninteraction");
        // SAFETY: no `None` here, cause `fi_pkg_name`、`local_node` is valid.
        let ret = unsafe { GetLocalNodeDeviceInfo(fi_pkg_name.as_ptr(), &mut local_node as *mut NodeBasicInfo) };
        if ret != RET_OK {
            error!(LOG_LABEL, "GetLocalNodeDeviceInfo result:{}", @public(ret));
            return Err(FusionErrorCode::Fail.into());
        }

        let network_id_ptr: *const c_char =  local_node.network_id.as_ptr() as *const c_char;
        // SAFETY: no `None` here, cause `network_id_ptr` is valid.
        let network_id_str = unsafe {CStr::from_ptr(network_id_ptr)};
        let network_id_slice: &str = network_id_str.to_str().unwrap();
        let network_id = network_id_slice.to_owned();
        if network_id.is_empty() {
            error!(LOG_LABEL, "Local networkid is empty");
            return Err(FusionErrorCode::Fail.into());
        }
        Ok(network_id_slice.to_owned())
    }

    /// TODO: add documentation.
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

    /// TODO: add documentation. 
    fn open_input_softbus(&mut self, remote_network_id: &String) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::open_input_softbus");
        let session_name = String::from("ohos.msdp.device_status");
        let group_id = String::from("fi_softbus_group_id");

        if self.check_device_session_state(remote_network_id) {
            error!(LOG_LABEL, "Softbus session has already opened");
            return Ok(());
        }

        if self.init().is_err() {
            error!(LOG_LABEL, "init failed");
            return Err(FusionErrorCode::Fail.into());
        }
     
        let data: u8 = 0;
        let session_attr =  SessionAttribute {
            data_type: TYPE_BYTES,
            link_type_num: DINPUT_LINK_TYPE_MAX,
            link_type: [LINK_TYPE_WIFI_WLAN_2G, LINK_TYPE_WIFI_WLAN_5G, LINK_TYPE_WIFI_P2P,
            LINK_TYPE_BR, 0, 0, 0, 0, 0],
            stream_attr: 0,
            fast_trans_data:data as *const u8,
            fast_trans_data_size: 0,
        };  

        let mut remote_network_id_slice = String::from("");
        if remote_network_id.len() > INTERCEPT_STRING_LENGTH {
            remote_network_id_slice = remote_network_id[0..INTERCEPT_STRING_LENGTH].to_string();
        }
         let peer_session_name = session_name + &remote_network_id_slice;
        // SAFETY: no `None` here, `local_session_name` is initialized in `init()`, `peer_session_name`,
        // `remote_network_id`, `group_id` and `session_attr` is valid.
        let session_id = unsafe { OpenSession(self.local_session_name.as_ptr() as *const c_char,
            peer_session_name.as_ptr() as *const c_char, remote_network_id.as_ptr() as *const c_char,
            group_id.as_ptr() as *const c_char, &session_attr as *const SessionAttribute) };
        if session_id < 0 {
            error!(LOG_LABEL, "OpenSession failed, session_id:{}", @public(session_id));
            return Err(FusionErrorCode::Fail.into());
        }

        self.wait_session_opend(remote_network_id, session_id)
    }

    /// TODO: add documentation.
    fn close_input_softbus(&mut self, remote_network_id: &String) {
        call_debug_enter!("DSoftbus::close_input_softbus");
        let get_result: Option<&i32> = self.session_dev_map.get(remote_network_id);
        if get_result.is_none() {
            error!(LOG_LABEL, "SessionDevIdMap not found");
        }
        let session_id = get_result.copied().unwrap();
        // SAFETY: no `None` here, `session_id` is valid.
        unsafe { CloseSession(session_id) };
        self.session_dev_map.remove(remote_network_id);
        self.channel_status_map.remove(remote_network_id);
        self.session_id = -1;
    }

    /// TODO: add documentation. 
    fn wait_session_opend(&mut self, remote_network_id: &String, session_id: i32) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::wait_session_opend");
        self.session_dev_map.insert(remote_network_id.to_string(), session_id);
        self.wait_cond = Arc::new((Mutex::new(false), Condvar::new()));
        let pair = Arc::clone(&self.wait_cond);
        let (lock, cvar) = &*pair;
        let result = (cvar.wait_timeout(self.operation_mutex.lock().unwrap(), Duration::from_secs(5))).unwrap();

        let get_result = self.channel_status_map.get(remote_network_id);
        if get_result.is_some() && !get_result.copied().unwrap(){
            error!(LOG_LABEL, "OpenSession timeout");    
            return Err(FusionErrorCode::Fail.into());
        }
        self.channel_status_map.insert(remote_network_id.to_string(), false);
        Ok(())
    }

    /// TODO: add documentation.
    fn on_session_opened(&mut self, session_id: i32, result: i32) -> FusionResult<()> {
        call_debug_enter!("DSoftbus::on_session_opened");
        let mut peer_dev_id: Vec<c_char> = Vec::with_capacity(65);
        self.session_id = session_id;
        let peer_dev_id_ptr = peer_dev_id.as_mut_ptr() as *mut c_char;
        // SAFETY: no `None` here
        let get_peer_device_id_result: i32 = unsafe { GetPeerDeviceId(session_id, peer_dev_id_ptr,
            ((std::mem::size_of::<c_char>()) * DEVICE_ID_SIZE_MAX).try_into().unwrap()) };
        
        let peer_dev_id_str = unsafe {CStr::from_ptr(peer_dev_id_ptr)};
        let peer_dev_id_slice: &str = peer_dev_id_str.to_str().unwrap();
        let peer_dev_id = peer_dev_id_slice.to_owned();

        if result != RET_OK {
            let device_id = match self.find_device(session_id) {
                Ok(device_id) => device_id,
                Err(err) => {
                    error!(LOG_LABEL, "find_device error");
                    return Err(FusionErrorCode::Fail.into());
                }
            };

            let get_result: Option<&i32> = self.session_dev_map.get(&device_id);
            if get_result.is_some() {
                self.session_dev_map.remove(&device_id);
            }

            if get_peer_device_id_result == RET_OK {
                self.channel_status_map.insert(peer_dev_id, true);
            }
            self.wait_cond.1.notify_all();
            return Ok(());
        }
        // SAFETY: no `None` here
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

    /// TODO: add documentation.
    fn find_device(&self, session_id: i32) -> FusionResult<String> {
        call_debug_enter!("DSoftbus::find_device");
        for (key, value) in self.session_dev_map.iter() {
            if *value == session_id{
                return Ok(key.to_string());
            }
        }
        error!(LOG_LABEL, "find_device error");
        Err(FusionErrorCode::Fail.into())
    }

    /// TODO: add documentation.
    fn on_session_closed(&mut self, session_id: i32) {
        call_debug_enter!("DSoftbus::on_session_closed");
        let device_id = match self.find_device(session_id) {
            Ok(device_id) => device_id,
            Err(err) => {
                error!(LOG_LABEL, "find_device error");
                return;
            }
        };
        let get_result: Option<&i32> = self.session_dev_map.get(&device_id);
        if get_result.is_some() {
            self.session_dev_map.remove(&device_id);
        }
        // SAFETY: no `None` here, `session_id` is valid.
        if unsafe { GetSessionSide(session_id) } != RET_OK {
            self.channel_status_map.remove(&device_id);
        }
        // SAFETY: no `None` here, `device_id` is valid.
        unsafe { CReset(device_id.as_ptr() as *const c_char) };
        self.session_id = -1;
    }

    /// TODO: add documentation.
    fn on_bytes_received(&self, session_id: i32, data: *const c_void, data_len: u32) {
        call_debug_enter!("DSoftbus::on_bytes_received");
        if session_id < 0 || data.is_null() || data_len == 0 {
           error!(LOG_LABEL, "Param check failed");
           return;
        }

        if let Some(call_back) = self.call_back_handle_msg {
            call_back(session_id, data as *const c_char);
        }
    }

    /// TODO: add documentation.
    fn check_device_session_state(&self, remote_network_id: &String) -> bool {
        call_debug_enter!("DSoftbus::check_device_session_state");
        let get_result: Option<&i32> = self.session_dev_map.get(remote_network_id);
        if get_result.is_none() {
            error!(LOG_LABEL, "Check session state error");
            return false;
        }
        true
    }

    /// TODO: add documentation.
    fn send_msg(&self, device_id: &String, message_id: MessageId, data: *const c_void, data_len: u32) -> FusionResult<()>{
        call_debug_enter!("DSoftbus::send_msg");
        let session_id = self.session_dev_map.get(device_id).copied().unwrap_or(0);
        // SAFETY: no `None` here, `session_id`, `data` and `data_len` is valid.
        let result: i32 = unsafe {SendBytes(session_id, data, data_len) };
        if result != RET_OK {
            error!(LOG_LABEL, "Send bytes failed, result:{}", @public(result));
            return Err(FusionErrorCode::Fail.into());
        }
        Ok(())
    }

    /// TODO: add documentation.
    fn get_session_dev_map(&self) -> HashMap<String, i32>{
        self.session_dev_map.clone()
    }

    /// TODO: add documentation.
    fn get_wait_cond(&self) -> Arc<(Mutex<bool>, Condvar)>{
        self.wait_cond.clone()
    }

    /// TODO: add documentation.
    fn get_operation_mutex(&self) -> Mutex<HashMap<String, i32>> {
        self.operation_mutex.lock().unwrap().clone().into()
    }
}

/// TODO: add documentation.
#[derive(Default)]
pub struct DSoftbus {
    dsoftbus_impl: Mutex<RefCell<Inner>>,
}

impl DSoftbus {
    /// interface of get_instance
    pub fn get_instance() -> Option<&'static Self> {
        static mut DSOFTBUS: Option<DSoftbus> = None;
        static INIT_ONCE: Once = Once::new();
        // SAFETY: no `None` here. just Modifying the Static Variables
        unsafe {
            INIT_ONCE.call_once(|| {
                DSOFTBUS = Some(DSoftbus::default());
            });
            DSOFTBUS.as_ref()
        }
    }

    /// interface of init
    pub fn init(&self) -> FusionResult<()> {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().init()
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail.into())
            }
        }
    }
   
   /// interface of release
   pub fn release(&self){
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().release();
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }

    /// interface of open_input_softbus
    pub fn open_input_softbus(&self, remote_network_id: &String) -> FusionResult<()> {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().open_input_softbus(remote_network_id)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail.into())
            }
        }
    }

    /// interface of close_input_softbus
    pub fn close_input_softbus(&self, remote_network_id: &String) {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().close_input_softbus(remote_network_id);
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }   

    /// interface of on_session_opened
    pub fn on_session_opened(&self, session_id: i32, result: i32) -> FusionResult<()>{
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().on_session_opened(session_id, result)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail.into())
            }
        }
    }

    /// interface of on_session_closed
    pub fn on_session_closed(&self, session_id: i32) {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().on_session_closed(session_id);
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }

    /// interface of on_bytes_received
    pub fn on_bytes_received(&self, session_id: i32, data: *const c_void, data_len: u32) {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().on_bytes_received(session_id, data, data_len);
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
            }
        }
    }

    /// interface of send_msg
    pub fn send_msg(&self, device_id: &String, message_id: MessageId, data: *const c_void,
        data_len: u32) -> FusionResult<()>{
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                guard.borrow_mut().send_msg(device_id, message_id, data, data_len)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail.into())
            }
        }
    }

    /// interface of on_message_received
    pub fn on_message_received(&self, session_id: i32, data: *const c_void, data_len: u32) {

    }

    /// interface of on_stream_received
    pub fn on_stream_received(&self, session_id: i32, data: *const StreamData,
        ext: *const StreamData, param: *const StreamFrameInfo) {

    }

    /// interface of get_session_dev_map
    pub fn get_session_dev_map(&self) -> FusionResult<HashMap<String, i32>> {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                Ok(guard.borrow_mut().get_session_dev_map())
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail.into())
            }
        }
    }

    /// interface of get_wait_cond
    pub fn get_wait_cond(&self) -> FusionResult<Arc<(Mutex<bool>, Condvar)>> {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {
                Ok(guard.borrow_mut().get_wait_cond())
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail.into())
            }
        }
    }

    /// interface of get_operation_mutex
    pub fn get_operation_mutex(&self) -> FusionResult<Mutex<HashMap<String, i32>>> {
        match self.dsoftbus_impl.lock() {
            Ok(guard) => {               
                Ok(guard.borrow_mut().get_operation_mutex())
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {:?}", err);
                Err(FusionErrorCode::Fail.into())
            }
        }
    }
}