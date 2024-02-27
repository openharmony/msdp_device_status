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

//! rust device profile binding sys

#![allow(dead_code)]

use std::ffi::{ c_char, c_int };

/// Constant for device profile
pub const RET_OK: i32 = 0;
pub const EMPTY_LENGTH: usize = 0;

/// C representation of [`ServiceCharacteristicProfile`].
#[repr(C)]
pub struct CServiceCharacteristicProfile {
    // The ID of the service.
    pub service_id: *const c_char,
    // The type of the service.
    pub service_type: *const c_char,
    // The json description of the service.
    pub service_profile_json: *const c_char,
    // The characteristic profile of the service.
    pub characteristic_profile_json: *const c_char,
}

/// Type definition for the clone function of the `CIProfileEvents` struct.
pub type CIProfileEventsClone = extern "C" fn (profile_events: *mut CIProfileEvents) -> *mut CIProfileEvents;
/// Type definition for the destruct function of the `CIProfileEvents` struct.
pub type CIProfileEventsDestruct = extern "C" fn (profile_events: *mut CIProfileEvents);

/// Represents a CIProfileEvents object that contains profile events information.
#[repr(C)]
pub struct CIProfileEvents {
    /// An optional clone function pointer for creating a clone of the `CIProfileEvents` object.
    pub clone: Option<CIProfileEventsClone>,
    /// An optional destruct function pointer for cleaning up the resources associated with the `CIProfileEvents` object
    pub destruct: Option<CIProfileEventsDestruct>,
    /// A mutable pointer to an array of unsigned 32-bit integers representing the profile events.
    pub profile_events: *mut u32,
    /// The number of profile events in the `profile_events` array.
    pub num_of_profile_events: usize,
}

/// C representation of [`SubscribeInfo`].
#[repr(C)]
pub struct CSubscribeInfo {
    /// The ID of the profile event to which the subscription applies.
    pub profile_event: u32,
    /// An optional pointer to extra information about the subscription.
    pub extra_info: *const c_char,
}

/// Type definition for the clone function of the `CISubscribeInfos` struct.
pub type CISubscribeInfosClone = extern "C" fn (profile_events: *mut CISubscribeInfos) -> *mut CISubscribeInfos;
/// Type definition for the destruct function of the `CISubscribeInfos` struct.
pub type CISubscribeInfosDestruct = extern "C" fn (profile_events: *mut CISubscribeInfos);

/// Struct representing subscription information.
#[repr(C)]
pub struct CISubscribeInfos {
    /// An optional clone function for creating a clone of the `CISubscribeInfos` object.
    pub clone: Option<CISubscribeInfosClone>,
    /// An optional destruct function for cleaning up the `CISubscribeInfos` object.
    pub destruct: Option<CISubscribeInfosDestruct>,
    /// A pointer to an array of `CSubscribeInfo` objects containing subscription details.
    pub subscribe_infos: *const CSubscribeInfo,
    /// The number of `CSubscribeInfo` objects in the `subscribe_infos` array.
    pub num_of_subscribe_infos: usize,
}

/// Enum representing the type of change that occurred in a profile entry.
#[repr(i32)]
#[derive(Copy, Clone)]
pub enum CProfileChangeType {
    /// Represents an unknown or uninitialized change type.
    Unknown,
    /// Represents an inserted profile entry.
    Inserted,
    /// Represents an updated profile entry.
    Updated,
    /// Represents a deleted profile entry.
    Deleted,
}

/// Struct representing a profile entry.
#[repr(C)]
pub struct CProfileEntry {
    /// A pointer to the key of the profile entry.
    pub key: *const c_char,
    /// A pointer to the value of the profile entry.
    pub value: *const c_char,
    /// The type of change that occurred for this profile entry.
    pub change_type: CProfileChangeType,
    /// A pointer to the next profile entry in the linked list.
    pub next: *const CProfileEntry,
}

/// C representation of [`ProfileChangeNotification`].
#[repr(C)]
pub struct CProfileChangeNotification {
    /// A pointer to an array of profile entries.
    pub profile_entries: *const CProfileEntry,
    /// The number of profile entries in the array.
    pub n_profile_entries: usize,
    /// A pointer to the device ID associated with the profile change.
    pub device_id: *const c_char,
    /// An integer indicating whether the profile change occurred locally.
    pub local_flag: c_int,
}

/// Type definition for the clone function of a CIProfileEventCb.
pub type CIProfileEventCbClone = extern "C" fn (profile_events: *mut CIProfileEventCb) -> *mut CIProfileEventCb;
/// Type definition for the destruct function of a CIProfileEventCb.
pub type CIProfileEventCbDestruct = extern "C" fn (profile_events: *mut CIProfileEventCb);
/// Type definition for the callback function when the profile changes.
pub type OnProfileChanged = extern "C" fn (cb: *mut CIProfileEventCb, notification: *const CProfileChangeNotification);

/// Struct representing a callback for profile events.
#[repr(C)]
pub struct CIProfileEventCb {
    /// The clone function for the callback.
    pub clone: Option<CIProfileEventCbClone>,
    /// The destruct function for the callback.
    pub destruct: Option<CIProfileEventCbDestruct>,
    /// The callback function to be executed when the profile changes.
    pub on_profile_changed: Option<OnProfileChanged>,
}

/// C representation of [`SyncMode`].
#[repr(i32)]
#[derive(Copy, Clone)]
pub enum CSyncMode {
    /// The pull synchronization mode, where the local device pulls data from the remote device.
    Pull,
    /// The push synchronization mode, where the local device pushes data to the remote device.
    Push,
    /// The push-pull synchronization mode, where the local and remote devices can push and pull data 
    /// in either direction.
    PushPull,
}

/// C representation of [`SyncOptions`].
#[repr(C)]
pub struct CSyncOptions {
    /// The synchronization mode.
    pub sync_mode: CSyncMode,
    /// Pointer to an array of device IDs.
    pub device_ids: *const *const c_char,
    /// The number of device IDs in the array.
    pub n_device_ids: usize,
}

// These C interfaces are defined in lib: device_profile:fusion_device_profile_binding.
extern "C" {
    /// Puts a device profile into the system.
    pub fn PutDeviceProfile(profile: *const CServiceCharacteristicProfile) -> i32;
    /// Gets a device profile from the system.
    pub fn GetDeviceProfile(ud_id: *const c_char,
                        service_id: *const c_char,
                        profile: *mut CServiceCharacteristicProfile) -> i32;
    /// Subscribes to profile events for the specified subscribe information.
    pub fn SubscribeProfileEvents(subscribe_infos: *const CISubscribeInfos,
                                  event_cb: *mut CIProfileEventCb,
                                  failed_events: *mut *mut CIProfileEvents) -> i32;
    /// Unsubscribes from profile events for the specified profile events and callback function.
    pub fn UnsubscribeProfileEvents(profile_events: *const CIProfileEvents,
                                    event_cb: *mut CIProfileEventCb,
                                    failed_events: *mut *mut CIProfileEvents) -> i32;
}

type CIStringVectorClone = extern "C" fn (*mut CIStringVector) -> *mut CIStringVector;
type CIStringVectorDestruct = extern "C" fn (*mut CIStringVector);
type CIStringVectorAt = extern "C" fn (*mut CIStringVector, usize) -> *const c_char;
type CIStringVectorSize = extern "C" fn (*mut CIStringVector) -> usize;

/// Struct representing a CIStringVector.
#[repr(C)]
pub struct CIStringVector {
    /// An optional clone function for cloning the CIStringVector.
    pub clone: Option<CIStringVectorClone>,
    /// An optional destruct function for cleaning up the CIStringVector.
    pub destruct: Option<CIStringVectorDestruct>,
    /// An optional function for accessing an element at a given index.
    pub get: Option<CIStringVectorAt>,
    /// An optional function for getting the size of the CIStringVector.
    pub get_size: Option<CIStringVectorSize>,
}

type CICrossStateListenerClone = extern "C" fn (listener: *mut CICrossStateListener) -> *mut CICrossStateListener;
type CICrossStateListenerDestruct = extern "C" fn (listener: *mut CICrossStateListener);
type OnCrossStateUpdate = extern "C" fn (listener: *mut CICrossStateListener, device_id: *const c_char, state: u32);

/// A struct representing a cross-state listener.
#[repr(C)]
pub struct CICrossStateListener {
    /// An optional function pointer for cloning the cross-state listener.
    pub clone: Option<CICrossStateListenerClone>,
    /// An optional function pointer for destructing or cleaning up the cross-state listener.
    pub destruct: Option<CICrossStateListenerDestruct>,
    /// An optional callback function for handling cross-state update events.
    pub on_update: Option<OnCrossStateUpdate>,
}

// These C interfaces are defined in lib: device_profile:fusion_device_profile_binding.
extern "C" {
    /// Updates the state of the cross switch for all registered devices.
    ///
    /// # Arguments
    ///
    /// * `state` - An unsigned integer value that represents the cross switch state.
    ///
    /// # Returns
    ///
    /// Returns an `i32` value representing the result of the update operation.
    pub fn UpdateCrossSwitchState(state: u32) -> i32;
    /// Synchronizes the state of the cross switch with multiple devices.
    ///
    /// # Arguments
    ///
    /// * `state` - The state to synchronize the cross switch to.
    /// * `device_ids` - A mutable pointer that contains the device IDs to synchronize.
    ///
    /// # Returns
    ///
    /// Returns an `i32` value representing the result of the synchronization operation.
    pub fn SyncCrossSwitchState(state: u32, device_ids: *mut CIStringVector) -> i32;
    /// Retrieves the state of the cross switch for a specific device.
    ///
    /// # Arguments
    ///
    /// * `device_id` - The ID of the device to retrieve the cross switch state for.
    ///
    /// # Returns
    ///
    /// Returns an `i32` value representing the state of the cross switch for the specified device.
    pub fn GetCrossSwitchState(device_id: *const c_char) -> i32;
    /// Registers a cross state listener for a specific device.
    ///
    /// # Arguments
    ///
    /// * `device_id` - The ID of the device to register the listener for.
    /// * `listener` - A mutable pointer to the cross state listener to register.
    ///
    /// # Returns
    ///
    /// Returns an `i32` value representing the result of the registration operation.
    pub fn RegisterCrossStateListener(device_id: *const c_char, listener: *mut CICrossStateListener) -> i32;
    /// Unregisters a cross state listener for a specific device.
    ///
    /// # Arguments
    ///
    /// * `device_id` - The ID of the device to unregister the listener from.
    ///
    /// # Returns
    ///
    /// Returns an `i32` value representing the result of the unregistration operation.
    pub fn UnregisterCrossStateListener(device_id: *const c_char) -> i32;
}

type CIStringClone = extern "C" fn (*mut CIString) -> *mut CIString;
type CIStringDestruct = extern "C" fn (*mut CIString);
type CIStringGetData = extern "C" fn (*mut CIString) -> *const c_char;

// Struct representing a CIString.
#[repr(C)]
pub struct CIString {
    /// Optional function pointer for cloning the string.
    pub clone: Option<CIStringClone>,
    /// Optional function pointer for destructing the string.
    pub destruct: Option<CIStringDestruct>,
    /// Optional function pointer for retrieving the string data.
    pub data: Option<CIStringGetData>,
}

// These C interfaces are defined in lib: device_profile:fusion_device_profile_binding.
extern "C" {
    /// Retrieves an access token from a C++ API.
    pub fn GetAccessToken();
    /// Get a mutable pointer to CIString.
    ///
    /// # Returns
    ///
    /// A mutable pointer to CIString. Please note that the pointer may be null if there is an error or no local network
    /// ID is available.The caller should handle null pointer values appropriately.
    pub fn GetLocalNetworkId() -> *mut CIString;
}
