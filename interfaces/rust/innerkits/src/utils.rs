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

use std::ffi::{CString, c_char};

/// Translate a &str to &[c_char; N].
pub fn trans_slice_to_array<const N: usize>(src: &str, dest: &mut [c_char; N]) {
    let name = CString::new(src).unwrap();
    let name = name.to_bytes();
    let name_len = name.len();
    let end = if name_len <= N {
        name_len
    } else {
        N
    };
    dest[..end].copy_from_slice(&name[..end])
}

/// recycle CString as raw pointer.
///
/// # Safety
///
#[allow(dead_code)]
pub fn free_allocated_str(ptr: *mut c_char) -> bool {
    if ptr.is_null() {
        return false;
    }
    unsafe {
        let ret = CString::from_raw(ptr);
        ret.to_bytes().is_empty()
    }
}