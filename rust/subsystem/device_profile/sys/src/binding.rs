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

use std::ffi::{ c_char, c_int };

#[repr(C)]
pub struct CServiceCharacteristicProfile {
    pub service_id: *const c_char,
    pub service_type: *const c_char,
    pub state: bool,
    pub characteristic_profile_json: *const c_char,
}

pub type CIProfileEventsClone = extern "C" fn (profile_events: *mut CIProfileEvents) -> *mut CIProfileEvents;
pub type CIProfileEventsDestruct = extern "C" fn (profile_events: *mut CIProfileEvents);

#[repr(C)]
pub struct CIProfileEvents {
    pub clone: Option<CIProfileEventsClone>,
    pub destruct: Option<CIProfileEventsDestruct>,
    pub profile_events: *mut u32,
    pub num_of_profile_events: usize,
}

#[repr(C)]
pub struct CSubscribeInfo {
    pub profile_event: u32,
    pub extra_info: *const c_char,
}

pub type CISubscribeInfosClone = extern "C" fn (profile_events: *mut CISubscribeInfos) -> *mut CISubscribeInfos;
pub type CISubscribeInfosDestruct = extern "C" fn (profile_events: *mut CISubscribeInfos);

#[repr(C)]
pub struct CISubscribeInfos {
    pub clone: Option<CISubscribeInfosClone>,
    pub destruct: Option<CISubscribeInfosDestruct>,
    pub subscribe_infos: *const CSubscribeInfo,
    pub num_of_subscribe_infos: usize,
}

#[repr(i32)]
#[derive(Copy, Clone)]
pub enum CProfileChangeType {
    Unknown,
    Inserted,
    Updated,
    Deleted,
}

#[repr(C)]
pub struct CProfileEntry {
    pub key: *const c_char,
    pub value: *const c_char,
    pub change_type: CProfileChangeType,
    pub next: *const CProfileEntry,
}

#[repr(C)]
pub struct CProfileChangeNotification {
    pub profile_entries: *const CProfileEntry,
    pub n_profile_entries: usize,
    pub device_id: *const c_char,
    pub is_local: c_int,
}

pub type CIProfileEventCbClone = extern "C" fn (profile_events: *mut CIProfileEventCb) -> *mut CIProfileEventCb;
pub type CIProfileEventCbDestruct = extern "C" fn (profile_events: *mut CIProfileEventCb);
pub type OnSyncCompleted = extern "C" fn (cb: *mut CIProfileEventCb, device_id: *const c_char, sync_result: c_int);
pub type OnProfileChanged = extern "C" fn (cb: *mut CIProfileEventCb, notification: *const CProfileChangeNotification);

#[repr(C)]
pub struct CIProfileEventCb {
    pub clone: Option<CIProfileEventCbClone>,
    pub destruct: Option<CIProfileEventCbDestruct>,
    pub on_sync_completed: Option<OnSyncCompleted>,
    pub on_profile_changed: Option<OnProfileChanged>,
}

#[repr(i32)]
#[derive(Copy, Clone)]
pub enum CSyncMode {
    Pull,
    Push,
    PushPull,
}

#[repr(C)]
pub struct CSyncOptions {
    pub sync_mode: CSyncMode,
    pub device_ids: *const *const c_char,
    pub n_device_ids: usize,
}

extern "C" {
    pub fn PutDeviceProfile(profile: *const CServiceCharacteristicProfile) -> i32;
    pub fn GetDeviceProfile(ud_id: *const c_char,
                        service_id: *const c_char,
                        profile: *mut CServiceCharacteristicProfile) -> i32;
    pub fn SubscribeProfileEvents(subscribe_infos: *const CISubscribeInfos,
                                  event_cb: *mut CIProfileEventCb,
                                  failed_events: *mut *mut CIProfileEvents) -> i32;
    pub fn UnsubscribeProfileEvents(profile_events: *const CIProfileEvents,
                                    event_cb: *mut CIProfileEventCb,
                                    failed_events: *mut *mut CIProfileEvents) -> i32;
    pub fn SyncDeviceProfile(sync_options: *const CSyncOptions, sync_cb: *const CIProfileEventCb) -> i32;
}

type CIStringVectorClone = extern "C" fn (*mut CIStringVector) -> *mut CIStringVector;
type CIStringVectorDestruct = extern "C" fn (*mut CIStringVector);
type CIStringVectorAt = extern "C" fn (*mut CIStringVector, usize) -> *const c_char;
type CIStringVectorSize = extern "C" fn (*mut CIStringVector) -> usize;

#[repr(C)]
pub struct CIStringVector {
    pub clone: Option<CIStringVectorClone>,
    pub destruct: Option<CIStringVectorDestruct>,
    pub at: Option<CIStringVectorAt>,
    pub size: Option<CIStringVectorSize>,
}

type CICrossStateListenerClone = extern "C" fn (listener: *mut CICrossStateListener) -> *mut CICrossStateListener;
type CICrossStateListenerDestruct = extern "C" fn (listener: *mut CICrossStateListener);
type OnCrossStateUpdate = extern "C" fn (listener: *mut CICrossStateListener, device_id: *const c_char, state: i32);

#[repr(C)]
pub struct CICrossStateListener {
    pub clone: Option<CICrossStateListenerClone>,
    pub destruct: Option<CICrossStateListenerDestruct>,
    pub on_update: Option<OnCrossStateUpdate>,
}

extern "C" {
    pub fn UpdateCrossSwitchState(state: i32) -> i32;
    pub fn SyncCrossSwitchState(state: i32, device_ids: *mut CIStringVector) -> i32;
    pub fn GetCrossSwitchState(device_id: *const c_char) -> i32;
    pub fn RegisterCrossStateListener(device_id: *const c_char, listener: *mut CICrossStateListener) -> i32;
    pub fn UnregisterCrossStateListener(device_id: *const c_char) -> i32;
}
