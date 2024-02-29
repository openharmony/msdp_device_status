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

//! rust device profile sys

#![allow(dead_code)]

use std::{ ffi::{ c_char, c_int, CString }, sync::Arc };
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_utils_rust::{ call_debug_enter, FusionErrorCode, FusionResult, err_log };

use crate::binding::{
    CProfileChangeNotification,
    CIProfileEventCb,
    CIProfileEvents,
    CSubscribeInfo,
    CISubscribeInfos,
    SubscribeProfileEvents,
    UnsubscribeProfileEvents,
    RET_OK,
};

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "DeviceProfile",
};

/// Represents a service characteristic profile.
pub struct ServiceCharacteristicProfile;

/// Represents an event related to a profile.
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

/// Represents information for subscribing to profile events.
pub struct SubscribeInfo {
    profile_event: ProfileEvent,
    extra_info: String,
}

/// Represents options for synchronizing a profile.
pub struct SyncOptions;
/// Represents the result of a profile synchronization operation.
pub struct SyncResult;

/// Represents types of changes that can occur to a profile.
#[repr(i32)]
#[derive(Copy, Clone)]
pub enum ProfileChangeType {
    Unknown,
    Inserted,
    Updated,
    Deleted,
}

/// Represents an entry in a profile with key-value information and change type.
pub struct ProfileEntry {
    /// The key of the profile entry.
    pub key: String,
    /// The value of the profile entry.
    pub value: String,
    /// The change type of the profile entry.
    pub change_type: ProfileChangeType,
}

/// Represents a notification for profile changes, containing affected profile entries,
/// device ID, and whether the change is local.
pub struct ProfileChangeNotification {
    /// The profile entries affected by the change.
    pub profile_entries: Vec<ProfileEntry>,
    /// The ID of the device associated with the change.
    pub device_id: String,
    /// Indicates whether the change is local to the current device.
    pub is_local: bool,
}

/// Trait defining the callback methods for profile events.
pub trait IProfileEventCallback {
    /// This method is called when a profile change notification is received.
    fn on_profile_changed(&self, change_notification: &ProfileChangeNotification);
}

struct ProfileEventCallback {
    interface: CIProfileEventCb,
    instance: Arc<dyn IProfileEventCallback>,
}

impl ProfileEventCallback {
    /// Structures with C-compatible layouts that safely interconvert the first structure field and structure instance.
    /// Based on the C17 standard
    /// 
    /// # Note
    ///
    /// This function performs a conversion of the pointer type to `*mut Self` and returns it.
    /// Please note that the pointer `cb` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure that the returned pointer is not null before dereferencing it.
    fn from_interface(cb: *mut CIProfileEventCb) -> Option<&'static mut Self>
    {
        let cb_ptr = cb as *mut Self;
        // SAFETY: `cb_ptr` is valid, because `as_mut` has null pointer checking.
        unsafe {
            cb_ptr.as_mut()
        }
    }
    /// Clone a `CIProfileEventCb` instance.
    ///
    /// # Note
    ///
    /// Please note that the pointer `cb` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn clone(cb: *mut CIProfileEventCb) -> *mut CIProfileEventCb
    {
        if let Some(callback_mut) = ProfileEventCallback::from_interface(cb) {
            let callback_box = Box::new(Self {
                interface: CIProfileEventCb {
                    clone: Some(Self::clone),
                    destruct: Some(Self::destruct),
                    on_profile_changed: Some(Self::on_profile_changed),
                },
                instance: callback_mut.instance.clone(),
            });
            Box::into_raw(callback_box) as *mut CIProfileEventCb
        } else {
            error!(LOG_LABEL, "Failed to clone a CIProfileEventCb instance");
            std::ptr::null_mut()
        }
    }
    /// Destruct a `CIProfileEventCb` instance.
    /// 
    /// # Note
    ///
    /// Please note that the pointer `cb` is a raw pointer that needs to be handled carefully to avoid memory
    /// safety issues and undefined behavior.
    /// Make sure to properly dereference and manipulate the data using appropriate safe Rust code.
    extern "C" fn destruct(cb: *mut CIProfileEventCb)
    {
        if let Some(callback_mut) = ProfileEventCallback::from_interface(cb) {
            // SAFETY: `callback_mut` is valid, becauce has been matched to `Some`.
            unsafe { drop(Box::from_raw(callback_mut as *mut ProfileEventCallback)) };
        } else {
            error!(LOG_LABEL, "Failed to destruct a CIProfileEventCb instance");
        }
    }

    /// This callback function is invoked when a profile change notification occurs.
    extern "C" fn on_profile_changed(_cb: *mut CIProfileEventCb, _notification: *const CProfileChangeNotification)
    {
        todo!()
    }
}

impl From<Arc<dyn IProfileEventCallback>> for ProfileEventCallback {
    fn from(value: Arc<dyn IProfileEventCallback>) -> Self {
        Self {
            interface: CIProfileEventCb {
                clone: Some(Self::clone),
                destruct: None,
                on_profile_changed: Some(Self::on_profile_changed),
            },
            instance: value,
        }
    }
}

/// Represents a device profile.
pub struct DeviceProfile;

impl DeviceProfile {
    fn check_return_code(&self, ret: i32) -> FusionResult<()>
    {
        (ret == RET_OK).then_some(()).ok_or(FusionErrorCode::Fail)
    }

    /// Updates the device profile with the specified `ServiceCharacteristicProfile`.
    pub fn put_device_profile(_profile: &ServiceCharacteristicProfile) -> FusionResult<()>
    {
        todo!()
    }

    /// Retrieves the device profile for the specified UDID and service ID.
    pub fn get_device_profile(_udid: &str, _service_id: &str,
        _profile: &ServiceCharacteristicProfile) -> FusionResult<()>
    {
        todo!()
    }

    /// Subscribes to the specified profile events.
    /// 
    /// # Arguments
    ///
    /// * `subscribe_infos` - A slice of `SubscribeInfo` structures representing the events to subscribe to.
    /// * `event_callback` - A reference to the `IProfileEventCallback` trait object for handling profile events.
    /// * `failed_events` - A mutable vector to store any events that failed to subscribe.
    ///
    /// # Returns
    ///
    /// An empty `FusionResult` indicating success or an error if subscribing to events fails.
    pub fn subscribe_profile_events(&self, subscribe_infos: &[SubscribeInfo],
        event_callback: &Arc<dyn IProfileEventCallback>,
        failed_events: &mut Vec<ProfileEvent>) -> FusionResult<()>
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

        // SAFETY: no `None` here, cause `subscriptions_borrowed`, `event_cb_ptr` and `failed_ptr` are valid.
        let ret = unsafe {
            SubscribeProfileEvents(&subscriptions_borrowed, event_cb_ptr as *mut CIProfileEventCb, &mut failed_ptr)
        };
        if !failed_ptr.is_null() {
            // SAFETY: `failed_ptr` is valid, because null pointer check has been performed.
            let profile_events_slice = unsafe {
                std::slice::from_raw_parts((*failed_ptr).profile_events, (*failed_ptr).num_of_profile_events)
            };
            for profile_event in profile_events_slice.iter() {
                if let Ok(e) = ProfileEvent::try_from(*profile_event) {
                    failed_events.push(e);
                }
            }
            // SAFETY: `failed_ptr` is valid, because null pointer check has been performed.
            unsafe {
                if let Some(destruct) = (*failed_ptr).destruct {
                    destruct(failed_ptr);
                }
            }
        }
        Ok(err_log!(self.check_return_code(ret), "SubscribeProfileEvents"))
    }

    /// Unsubscribes from the specified profile events.
    /// 
    /// # Arguments
    ///
    /// * `profile_events` - A slice of `ProfileEvent` enumerations representing the events to unsubscribe from.
    /// * `event_callback` - A reference to the `IProfileEventCallback` trait object for handling profile events.
    /// * `failed_events` - A mutable vector to store any events that failed to unsubscribe.
    ///
    /// # Returns
    ///
    /// An empty `FusionResult` indicating success or an error if unsubscribing from events fails.
    pub fn unsubscribe_profile_events(&self, profile_events: &[ProfileEvent],
        event_callback: &Arc<dyn IProfileEventCallback>,
        failed_events: &mut Vec<ProfileEvent>) -> FusionResult<()>
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

        // SAFETY:  no `None` here, cause `profile_events_borrowed`, `event_cb_ptr` and `failed_ptr` are valid.
        let ret = unsafe {
            UnsubscribeProfileEvents(&profile_events_borrowed, event_cb_ptr as *mut CIProfileEventCb, &mut failed_ptr)
        };
        if !failed_ptr.is_null() {
            // SAFETY: `failed_ptr` is valid, because null pointer check has been performed.
            let profile_events_slice = unsafe {
                std::slice::from_raw_parts((*failed_ptr).profile_events, (*failed_ptr).num_of_profile_events)
            };
            for profile_event in profile_events_slice.iter() {
                if let Ok(e) = ProfileEvent::try_from(*profile_event) {
                    failed_events.push(e);
                }
            }
            // SAFETY: `failed_ptr`is valid, because null pointer check has been performed.
            unsafe {
                if let Some(destruct) = (*failed_ptr).destruct {
                    destruct(failed_ptr);
                }
            }
        }
        Ok(err_log!(self.check_return_code(ret), "UnsubscribeProfileEvents"))
    }

    /// Synchronizes the device profile with the specified options.
    pub fn sync_device_profile(_sync_options: &SyncOptions,
                               _sync_callback: &Arc<dyn IProfileEventCallback>) -> FusionResult<()>
    {
        todo!()
    }
}
