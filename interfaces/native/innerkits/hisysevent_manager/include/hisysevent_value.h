/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef HISYSEVENT_VALUE_H
#define HISYSEVENT_VALUE_H

#include <vector>

#include "cJSON.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventValue {
public:
    HiSysEventValue(std::string jsonStr)
    {
        ParseJsonStr(jsonStr);
    }
    HiSysEventValue(cJSON* jsonVal): jsonVal_(jsonVal), hasInitialized_(true), needDeleteJsonVal(false) {}
    ~HiSysEventValue();

public:
    bool HasInitialized() const;
    bool IsArray() const;
    bool IsMember(const std::string key) const;
    bool IsInt64() const;
    bool IsUInt64() const;
    bool IsDouble() const;
    bool IsString() const;
    bool IsBool() const;
    bool IsNull() const;
    bool IsNumeric() const;
    double AsDouble() const;
    int64_t AsInt64() const;
    uint64_t AsUInt64() const;
    int Size() const;
    std::string AsString() const;
    cJSON* Index(const int index) const;
    cJSON* GetParamValue(const std::string& key) const;
    void GetParamNames(std::vector<std::string>& params) const;

private:
    void ParseJsonStr(const std::string jsonStr);

private:
    cJSON* jsonVal_;
    bool hasInitialized_ = false;
    bool needDeleteJsonVal = false;
};
}
}

#endif // HISYSEVENT_VALUE_H