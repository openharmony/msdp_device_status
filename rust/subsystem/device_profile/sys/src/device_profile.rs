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

//! TODO: add documentation.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, c_int, CString };
use std::rc::Rc;
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_utils_rust::{ call_debug_enter, FusionErrorCode, FusionResult };
use crate::binding::{
    CProfileChangeNotification,
    CIProfileEventCb,
    CIProfileEvents,
    CSubscribeInfo,
    CISubscribeInfos,
    SubscribeProfileEvents,
    UnsubscribeProfileEvents,
};

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DeviceProfile",
};

pub struct ServiceCharacteristicProfile {

}

#[repr(u32)]
#[derive(Copy, Clone)]
pub enum ProfileEvent {
    Unknown = 0,
    SyncCompleted = 1,
    ProfileChanged = 2,
}

impl From<ProfileEvent> for u32 {
    fn from(value: ProfileEvent) -> u32 {
        match value {
            ProfileEvent::Unknown => { 0 },
            ProfileEvent::SyncCompleted => { 1 },
            ProfileEvent::ProfileChanged => { 2 },
        }
    }
}

impl TryFrom<u32> for ProfileEvent {
    type Error = FusionErrorCode;

    fn try_from(value: u32) -> Result<Self, Self::Error> {
        match value {
            _ if u32::from(ProfileEvent::Unknown) == value => { Ok(ProfileEvent::Unknown) },
            _ if u32::from(ProfileEvent::SyncCompleted) == value => { Ok(ProfileEvent::SyncCompleted) },
            _ if u32::from(ProfileEvent::ProfileChanged) == value => { Ok(ProfileEvent::ProfileChanged) },
            _ => { Err(FusionErrorCode::Fail) },
        }
    }
}

pub struct SubscribeInfo {
    profile_event: ProfileEvent,
    extra_info: String,
}

pub struct SyncOptions {

}

pub struct SyncResult {

}

#[repr(i32)]
#[derive(Copy, Clone)]
pub enum ProfileChangeType {
    Unknown,
    Inserted,
    Updated,
    Deleted,
}

pub struct ProfileEntry {
    pub key: String,
    pub value: String,
    pub change_type: ProfileChangeType,
}

pub struct ProfileChangeNotification {
    pub profile_entries: Vec<ProfileEntry>,
    pub device_id: String,
    pub is_local: bool,
}

pub trait IProfileEventCallback {
    fn on_sync_completed(&self, sync_results: &SyncResult);
    fn on_profile_changed(&self, change_notification: &ProfileChangeNotification);
}

struct ProfileEventCallback {
    interface: CIProfileEventCb,
    instance: Rc<dyn IProfileEventCallback>,
}

impl ProfileEventCallback {
    extern "C" fn clone(cb: *mut CIProfileEventCb) -> *mut CIProfileEventCb
    {
        let callback_ptr = cb as *mut ProfileEventCallback;

        match unsafe { callback_ptr.as_ref() } {
            Some(callback_ref) => {
                let callback_box = Box::new(Self {
                    interface: CIProfileEventCb {
                        clone: Some(Self::clone),
                        destruct: Some(Self::destruct),
                        on_sync_completed: Some(Self::on_sync_completed),
                        on_profile_changed: Some(Self::on_profile_changed),
                    },
                    instance: callback_ref.instance.clone(),
                });
                Box::into_raw(callback_box) as *mut CIProfileEventCb
            },
            None => {
                std::ptr::null_mut()
            },
        }
    }

    extern "C" fn destruct(cb: *mut CIProfileEventCb)
    {
        if !cb.is_null() {
            unsafe {
                drop(Box::from_raw(cb as *mut Self));
            }
        }
    }

    extern "C" fn on_sync_completed(cb: *mut CIProfileEventCb, device_id: *const c_char, sync_result: c_int)
    {

    }

    extern "C" fn on_profile_changed(cb: *mut CIProfileEventCb, notification: *const CProfileChangeNotification)
    {

    }
}

impl From<Rc<dyn IProfileEventCallback>> for ProfileEventCallback {
    fn from(value: Rc<dyn IProfileEventCallback>) -> Self {
        Self {
            interface: CIProfileEventCb {
                clone: Some(Self::clone),
                destruct: None,
                on_sync_completed: Some(Self::on_sync_completed),
                on_profile_changed: Some(Self::on_profile_changed),
            },
            instance: value,
        }
    }
}

/// TODO: add documentation.
pub struct DeviceProfile {

}

impl DeviceProfile {
    /// TODO: add documentation.
    pub fn put_device_profile(profile: &ServiceCharacteristicProfile) -> FusionResult<i32>
    {
        Err(FusionErrorCode::Fail)
    }

    /// TODO: add documentation.
    pub fn get_device_profile(udid: &str, service_id: &str,
        profile: &ServiceCharacteristicProfile) -> FusionResult<i32>
    {
        Err(FusionErrorCode::Fail)
    }

    /// TODO: add documentation.
    pub fn subscribe_profile_events(subscribe_infos: &[SubscribeInfo],
        event_callback: &Rc<dyn IProfileEventCallback>,
        failed_events: &mut Vec<ProfileEvent>) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfile::subscribe_profile_events");
        let mut subscriptions = Vec::<CSubscribeInfo>::new();
        for info in subscribe_infos.iter() {
            subscriptions.push(CSubscribeInfo {
                profile_event: info.profile_event as u32,
                extra_info: info.extra_info.as_ptr() as *const c_char,
            });
        }
        let subscriptions_borrowed = CISubscribeInfos {
            clone: None,
            destruct: None,
            subscribe_infos: subscriptions.as_ptr(),
            num_of_subscribe_infos: subscriptions.len(),
        };

        let mut event_cb = ProfileEventCallback::from(event_callback.clone());
        let event_cb_ptr: *mut ProfileEventCallback = &mut event_cb;
        let mut failed_ptr: *mut CIProfileEvents = std::ptr::null_mut();

        let ret = unsafe {
            SubscribeProfileEvents(&subscriptions_borrowed, event_cb_ptr as *mut CIProfileEventCb, &mut failed_ptr)
        };
        if !failed_ptr.is_null() {
            let profile_events_slice = unsafe {
                std::slice::from_raw_parts((*failed_ptr).profile_events, (*failed_ptr).num_of_profile_events)
            };
            for profile_event in profile_events_slice.iter() {
                if let Ok(e) = ProfileEvent::try_from(*profile_event) {
                    failed_events.push(e);
                }
            }
            unsafe {
                if let Some(destruct) = (*failed_ptr).destruct {
                    destruct(failed_ptr);
                }
            }
        }
        if ret != 0 {
            error!(LOG_LABEL, "SubscribeProfileEvents fail");
            Err(FusionErrorCode::Fail)
        } else {
            Ok(0)
        }
    }

    /// TODO: add documentation.
    pub fn unsubscribe_profile_events(profile_events: &[ProfileEvent],
        event_callback: &Rc<dyn IProfileEventCallback>,
        failed_events: &mut Vec<ProfileEvent>) -> FusionResult<i32>
    {
        call_debug_enter!("DeviceProfile::unsubscribe_profile_events");
        let mut profileevents = Vec::<u32>::new();
        for info in profile_events.iter() {
            profileevents.push((*info).into());
        }
        let profile_events_borrowed = CIProfileEvents {
            clone: None,
            destruct: None,
            profile_events: profileevents.as_ptr() as *mut u32,
            num_of_profile_events: profileevents.len(),
        };

        let mut event_cb = ProfileEventCallback::from(event_callback.clone());
        let event_cb_ptr: *mut ProfileEventCallback = &mut event_cb;
        let mut failed_ptr: *mut CIProfileEvents = std::ptr::null_mut();

        let ret = unsafe {
            UnsubscribeProfileEvents(&profile_events_borrowed, event_cb_ptr as *mut CIProfileEventCb, &mut failed_ptr)
        };
        if !failed_ptr.is_null() {
            let profile_events_slice = unsafe {
                std::slice::from_raw_parts((*failed_ptr).profile_events, (*failed_ptr).num_of_profile_events)
            };
            for profile_event in profile_events_slice.iter() {
                if let Ok(e) = ProfileEvent::try_from(*profile_event) {
                    failed_events.push(e);
                }
            }
            unsafe {
                if let Some(destruct) = (*failed_ptr).destruct {
                    destruct(failed_ptr);
                }
            }
        }

        if ret != 0 {
            error!(LOG_LABEL, "UnsubscribeProfileEvents fail");
            Err(FusionErrorCode::Fail)
        } else {
            Ok(0)
        }
    }

    /// TODO: add documentation.
    pub fn sync_device_profile(sync_options: &SyncOptions,
                               sync_callback: &Rc<dyn IProfileEventCallback>) -> FusionResult<i32>
    {
        Err(FusionErrorCode::Fail)
    }
}
