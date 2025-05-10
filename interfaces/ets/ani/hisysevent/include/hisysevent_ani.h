/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef HITRACE_METER_ANI_H
#define HITRACE_METER_ANI_H

#include <ani.h>
#include <unordered_map>

#include "hisysevent.h"

namespace OHOS {
namespace HiviewDFX {
using Param = std::variant<bool, double, int64_t, uint64_t, std::string,
    std::vector<bool>, std::vector<double>, std::vector<int64_t>, std::vector<uint64_t>,
    std::vector<std::string>>;

using HiSysEventInfo = struct HiSysEventInfo {
    std::string domain;
    std::string name;
    HiSysEvent::EventType eventType;
    std::unordered_map<std::string, Param> params;
};
using JsCallerInfo = std::pair<std::string, int64_t>;

struct ParamArray {
    std::vector<bool> boolArray;
    std::vector<double> numberArray;
    std::vector<std::string> stringArray;
    std::vector<int64_t> bigintArray;
};

enum ParamType { BL = 0, DOU, I64, U64, STR, BOOL_ARR, DOUBLE_ARR, I64_ARR, U64_ARR, STR_ARR };

enum AniArgsType {
    ANI_UNKNOWN = 0,
    ANI_BOOLEAN = 1,
    ANI_NUMBER = 2,
    ANI_STRING = 3,
    ANI_BIGINT = 4,
    ANI_UNDEFINED = 5,
};

class HiSysEventAni {
public:
    static ani_object WriteSync(ani_env *env, ani_object info);
    static void AddWatcher(ani_env *env, ani_object watcher);
    static void RemoveWatcher(ani_env *env, ani_object watcher);
    static void Query(ani_env *env, ani_object queryArg, ani_array rules, ani_object querier);
    static ani_double ExportSysEvents(ani_env *env, ani_object queryArg, ani_array rules);
    static ani_double Subscribe(ani_env *env, ani_array rules);
    static void Unsubscribe(ani_env *env);
    
private:
    static int32_t WriteInner(ani_env *env, const HiSysEventInfo &eventInfo);
    static void AppendParams(HiSysEvent::EventBase &eventBase,
                             const std::unordered_map<std::string, Param> &params)
    {
        const std::vector<std::function<void(const std::string&, const Param&)>> allParamsHandlers = {
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<BL>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<DOU>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<I64>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<U64>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<STR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<BOOL_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<DOUBLE_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<I64_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<U64_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<STR_ARR>(param));
            },
        };
        for (auto iter = params.cbegin(); iter != params.cend(); ++iter) {
            Param param = iter->second;
            size_t paramIndex = static_cast<size_t>(param.index());
            if (paramIndex >= allParamsHandlers.size()) {
                continue;
            }
            auto paramHandler = allParamsHandlers.at(paramIndex);
            if (paramHandler != nullptr) {
                paramHandler(iter->first, param);
            }
        }
    }
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HITRACE_METER_ANI_H
