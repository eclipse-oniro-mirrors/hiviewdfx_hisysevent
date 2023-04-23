/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_RAW_DATA_H
#define INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_RAW_DATA_H

#include <cstdarg>
#include <cstddef>
#include <cstdint>

namespace OHOS {
namespace HiviewDFX {
namespace Encode {
class RawData {
public:
    RawData();
    RawData(const RawData& data);
    ~RawData();

public:
    RawData& operator=(const RawData& data);

public:
    void Reset();
    bool Append(uint8_t* data, size_t len);
    bool Update(uint8_t* data, size_t len, size_t pos);
    bool IsEmpty();
    uint8_t* GetData() const;
    size_t GetDataLength() const;

private:
    uint8_t* data_ = nullptr;
    size_t len_ = 0;
    size_t capacity_ = 0;
};
} // namespace Encode
} // namespace HiviewDFX
} // namespace OHOS

#endif // INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_RAW_DATA_H