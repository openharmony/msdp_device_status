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

//! IPC data definitions of DRAG module.

use std::ffi::{ c_char, CString };
use std::fmt::{ Display, Formatter, Error };

use hilog_rust::{ info, hilog, HiLogLabel, LogType };
use ipc_rust::{ BorrowedMsgParcel, Serialize, Deserialize, IpcResult };

use fusion_utils_rust::call_debug_enter;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionDragData"
};

/// C representation of [`ShadowInfo`].
#[repr(C)]
pub struct CShadowInfo {
    x: i32,
    y: i32,
}

/// C representation of [`DragData`].
#[repr(C)]
pub struct CDragData {
    shadow_info: CShadowInfo,
    buffer: *const u8,
    buffer_size: usize,
    source_type: i32,
    drag_num: i32,
    pointer_id: i32,
    display_x: i32,
    display_y: i32,
    display_id: i32,
    has_canceled_animation: bool,
}

/// Data of shadow.
pub struct ShadowInfo {
    x: i32,
    y: i32,
}

impl ShadowInfo {
    /// Converts `CShadowInfo` type to `ShadowInfo` type
    pub fn from_c(value: &mut CShadowInfo) -> Self
    {
        call_debug_enter!("ShadowInfo::from_c");
        Self {
            x: value.x,
            y: value.y,
        }
    }
}

impl Serialize for ShadowInfo {
    fn serialize(&self, parcel: &mut BorrowedMsgParcel<'_>) -> IpcResult<()>
    {
        call_debug_enter!("ShadowInfo::serialize");
        self.x.serialize(parcel)?;
        self.y.serialize(parcel)?;
        Ok(())
    }
}

impl Deserialize for ShadowInfo {
    fn deserialize(parcel: &BorrowedMsgParcel<'_>) -> IpcResult<Self>
    {
        call_debug_enter!("ShadowInfo::deserialize");
        let shadow_info = Self {
            x: i32::deserialize(parcel)?,
            y: i32::deserialize(parcel)?,
        };
        Ok(shadow_info)
    }
}

/// Bookkeeping of drag operation.
pub struct DragData {
    /// Meta data of shadow.
    pub shadow_info: ShadowInfo,
    /// Buffer for meta data of drag.
    pub buffer: Vec<u8>,
    /// The device type of input event that trigger this drag.
    pub source_type: i32,
    /// Represents `drag_num` information in the `DragData` struct
    pub drag_num: i32,
    /// Represents `pointer_id` information in the `DragData` struct
    pub pointer_id: i32,
    /// Represents `display_x` information in the `DragData` struct
    pub display_x: i32,
    /// Represents `display_y` information in the `DragData` struct
    pub display_y: i32,
    /// Represents `display_id` information in the `DragData` struct
    pub display_id: i32,
    /// Represents `has_canceled_animation` information in the `DragData` struct
    pub has_canceled_animation: bool,
}

impl DragData {
    /// Converts `CDragData` type to `DragData` type
    pub fn from_c(value: &mut CDragData) -> Self
    {
        call_debug_enter!("DragData::from_c");
        let mut buf: Vec<u8> = Vec::new();
        let ts = unsafe {
            std::slice::from_raw_parts(value.buffer, value.buffer_size)
        };
        info!(LOG_LABEL, "Fill buffer");
        for item in ts.iter() {
            buf.push(*item);
        }
        info!(LOG_LABEL, "new DragData instance");
        Self {
            shadow_info: ShadowInfo::from_c(&mut value.shadow_info),
            buffer: buf,
            source_type: value.source_type,
            drag_num: value.drag_num,
            pointer_id: value.pointer_id,
            display_x: value.display_x,
            display_y: value.display_y,
            display_id: value.display_id,
            has_canceled_animation: value.has_canceled_animation,
        }
    }
}

impl Serialize for DragData {
    fn serialize(&self, parcel: &mut BorrowedMsgParcel<'_>) -> IpcResult<()>
    {
        info!(LOG_LABEL, "In DragData::serialize() enter");
        self.shadow_info.serialize(parcel)?;
        self.buffer.serialize(parcel)?;
        self.source_type.serialize(parcel)?;
        self.drag_num.serialize(parcel)?;
        self.pointer_id.serialize(parcel)?;
        self.display_x.serialize(parcel)?;
        self.display_y.serialize(parcel)?;
        self.display_id.serialize(parcel)?;
        self.has_canceled_animation.serialize(parcel)?;
        Ok(())
    }
}

impl Deserialize for DragData {
    fn deserialize(parcel: &BorrowedMsgParcel<'_>) -> IpcResult<Self>
    {
        info!(LOG_LABEL, "In DragData::deserialize() enter");
        let drag_data = Self {
            shadow_info: ShadowInfo::deserialize(parcel)?,
            buffer: Vec::<u8>::deserialize(parcel)?,
            source_type: i32::deserialize(parcel)?,
            drag_num: i32::deserialize(parcel)?,
            pointer_id: i32::deserialize(parcel)?,
            display_x: i32::deserialize(parcel)?,
            display_y: i32::deserialize(parcel)?,
            display_id: i32::deserialize(parcel)?,
            has_canceled_animation: bool::deserialize(parcel)?,
        };
        Ok(drag_data)
    }
}

impl Display for DragData {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), Error>
    {
        writeln!(f, "\nDragData {{")?;
        writeln!(f, "  shadow_info: {{")?;
        writeln!(f, "    x: {},", self.shadow_info.x)?;
        writeln!(f, "    y: {},", self.shadow_info.y)?;
        writeln!(f, "  }},")?;
        writeln!(f, "  buffer: [*],")?;
        writeln!(f, "  source_type: {},", self.source_type)?;
        writeln!(f, "  drag_num: {},", self.drag_num)?;
        writeln!(f, "  pointer_id: {},", self.pointer_id)?;
        writeln!(f, "  display_x: {},", self.display_x)?;
        writeln!(f, "  display_y: {},", self.display_y)?;
        writeln!(f, "  display_id: {},", self.display_id)?;
        writeln!(f, "  has_canceled_animation: {},", self.has_canceled_animation)?;
        writeln!(f, "}}")?;
        Ok(())
    }
}
