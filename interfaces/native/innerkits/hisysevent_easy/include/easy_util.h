/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef HISYSEVENT_INTERFACES_NATIVE_INNERKITS_EASY_UTIL_H
#define HISYSEVENT_INTERFACES_NATIVE_INNERKITS_EASY_UTIL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize memory with designated length
 *
 * @param data      pointer to the memory which need to be initilized
 * @param dataLen   byte count of memory which need to be initilized
 */
int MemoryInit(uint8_t* data, const size_t dataLen);

/**
 * @brief Copy memory with designated length
 *
 * @param dest  pointer to the memory to which to copy
 * @param src   pointer to the memory from which to copy
 * @param len   byte count of memory to copy
 */
int MemoryCpy(uint8_t* dest, uint8_t* src, const size_t len);

#ifdef __cplusplus
}
#endif
#endif // HISYSEVENT_INTERFACES_NATIVE_INNERKITS_EASY_UTIL_H