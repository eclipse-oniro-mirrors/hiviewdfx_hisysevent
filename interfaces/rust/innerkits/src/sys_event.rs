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

use std::ffi::{CString, c_char, c_int, c_uint, c_ulonglong};

/// Length limit for the name of a param.
pub(crate) const MAX_LENGTH_OF_PARAM_NAME: usize = 33;

/// This type represent to HiSysEventParamValue defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub union HiSysEventParamValue {
    /// Bool.
    pub b_: bool,

    /// Int8_t.
    pub i8_: i8,

    /// Uint8_t.
    pub u8_: u8,

    /// Int16_t.
    pub i16_: i16,

    /// Uint16_t.
    pub u16_: u16,

    /// Int32_t.
    pub i32_: i32,

    /// Uint32_t.
    pub u32_: u32,

    /// Int64_t.
    pub i64_: i64,

    /// Uint64_t.
    pub u64_: u64,

    /// Float.
    pub f32_: f32,

    /// Double.
    pub f64_: f64,

    /// String.
    pub char_ptr_: *const c_char,

    /// Array.
    pub void_ptr_: *const (),
}

/// This type represent to HiSysEventParamType defined in C.
#[derive(Copy, Clone)]
pub enum HiSysEventParamType {
    /// Invalid type.
    Invalid = 0,

    /// Bool type.
    Bool,

    /// Int8_t type.
    Int8,

    /// Uint8_t type.
    Uint8,

    /// Int16_t type.
    Int16,

    /// Uint16_t type.
    Uint16,

    /// Int32_t type.
    Int32,

    /// Uint32_t type.
    Uint32,

    /// Int64_t type.
    Int64,

    /// Uint64_t type.
    Uint64,

    /// Float type.
    Float,

    /// Double type.
    Double,

    /// String type.
    ParamTypeString,

    /// Bool array type.
    BoolArray,

    /// Int8_t array type.
    Int8Array,

    /// Uint8_t array type.
    Uint8Array,

    /// Int16_t array type.
    Int16Array,

    /// Uint16_t array type.
    Uint16Array,

    /// Int32_t array type.
    Int32Array,

    /// Unt32_t array type.
    Uint32Array,

    /// Int64_t array type.
    Int64Array,

    /// Uint16_t array type.
    Uint64Array,

    /// Float array type.
    FloatArray,

    /// Double array type.
    DoubleArray,

    /// String array type.
    ParamTypeStringArray,
}

/// Definition customized param.
pub struct HiSysEventParam<'a> {
    /// Param key
    pub param_name: &'a str,

    /// Param type
    pub param_type: HiSysEventParamType,

    /// Param value
    pub param_value: HiSysEventParamValue,

    /// Size of param value
    pub array_size: usize,
}

/// This type represent to HiSysEventParamWrapper defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventParamWrapper {
    /// Param name.
    pub param_name: [c_char; MAX_LENGTH_OF_PARAM_NAME],

    /// Param type.
    pub param_type: c_int,

    /// Param value.
    pub param_value: HiSysEventParamValue,

    /// Length of a param with array type.
    pub array_size: c_uint,
}

/// Translate HiSysEventParam which defined in rust into HiSysEventParamWrapper which defined in C.
pub(crate) fn convert_param_to_wrapper<const N: usize>(src: &[HiSysEventParam; N], dest: &mut [HiSysEventParamWrapper; N]) {
    for i in 0..N {
        crate::utils::trans_slice_to_array(src[i].param_name, &mut dest[i].param_name);
        dest[i].param_type = src[i].param_type as i32 as c_int;
        dest[i].param_value = src[i].param_value;
        dest[i].array_size = src[i].array_size as c_uint;
    }
}

/// Parse type and length of a variable.
#[allow(dead_code)]
pub fn parse_type_len<T>(_: T) -> (&'static str, usize) {
    let mut value_type = std::any::type_name::<T>();
    let mut bytes = value_type.as_bytes();
    if bytes[0] == b'&' {
        value_type = &value_type[1..];
		bytes = value_type.as_bytes();
    }
    if bytes[0] != b'[' {
        // not a array
        if bytes[0] == b'&' {
            // reference
            return (&value_type[1..], 0);
        } else {
            return (value_type, 0);
        }
    }
    // array
    let mut val_end: usize = 0;
    let mut len_start: usize = 0;
    for (i, &item) in bytes.iter().enumerate() {
        if item == b';' {
            val_end = i;
        }
        if item == b' ' {
            len_start = i + 1;
            break;
        }
    }
    let array_len = value_type[len_start..(bytes.len() - 1)].parse::<usize>().unwrap();
    if bytes[1] == b'&' {
        // reference
        (&value_type[2..val_end], array_len)
    } else {
        (&value_type[1..val_end], array_len)
    }
}

/// Build hisysevent param with string array type.
#[allow(dead_code)]
pub fn build_string_arrays<'a, const N: usize>(param_name: &'a str, param_value: [&'a str; N]) -> HiSysEventParam<'a> {
    let init_wrapper = CString::new("").unwrap();
    let mut dest = std::boxed::Box::<[*const c_char;N]>::new([
        init_wrapper.as_ptr() as *const c_char; N
    ]);
    for i in 0..N {
        let str_wrapper = CString::new(param_value[i]).unwrap();
        dest[i] = str_wrapper.into_raw() as *const c_char;
    }
    HiSysEventParam {
        param_name,
        param_type: HiSysEventParamType::ParamTypeStringArray,
        param_value: HiSysEventParamValue {
            void_ptr_: std::boxed::Box::<[*const c_char;N]>::into_raw(dest) as *const c_int as *const (),
        },
        array_size: N,
    }
}

/// Write system event.
pub fn write<const N: usize>(event_domain: &str, event_name: &str, event_type: c_int, event_params: &[HiSysEventParam; N]) -> i32 {
    let mut param_wrappers = [
        HiSysEventParamWrapper {
            param_name: [0; MAX_LENGTH_OF_PARAM_NAME],
            param_type: 0,
            param_value: HiSysEventParamValue {
                b_: false,
            },
            array_size: 0,
        }; N];
    convert_param_to_wrapper(event_params, &mut param_wrappers);
    unsafe {
        let func = CString::new(crate::function!()).unwrap();
        let domain = CString::new(event_domain).unwrap();
        let event_name = CString::new(event_name).unwrap();
        let params = &param_wrappers as *const [HiSysEventParamWrapper; N];
        HiSysEventWriteWrapper(
            func.as_ptr() as *const c_char,
            line!() as c_ulonglong,
            domain.as_ptr() as *const c_char,
            event_name.as_ptr() as *const c_char,
            event_type,
            params as *const HiSysEventParamWrapper,
            N as c_uint
        )
    }
}

extern "C" {
    /// ffi border function.
    pub(crate) fn HiSysEventWriteWrapper(func: *const c_char, line: c_ulonglong, domain: *const c_char,
        name: *const c_char, event_type: c_int, params: *const HiSysEventParamWrapper,
        size: c_uint) -> c_int;
}