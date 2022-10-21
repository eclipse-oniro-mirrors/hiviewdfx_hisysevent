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

#ifndef ARG_RULES_H
#define ARG_RULES_H

#include <string>
#include <vector>

#include "rule_type.h"

namespace OHOS {
namespace HiviewDFX {
struct QueryArg {
    long long beginTime;
    long long endTime;
    int maxEvents;
    QueryArg(const long long begin, const long long end, const int max)
        : beginTime(begin), endTime(end), maxEvents(max) {}
    QueryArg() {}
};

class ListenerRule {
public:
    ListenerRule(const std::string& domain, const std::string& eventName,
        const std::string& tag, RuleType ruleType = RuleType::WHOLE_WORD) : domain(domain),
        eventName(eventName), tag(tag), ruleType(ruleType) {}
    ListenerRule(const std::string& domain, const std::string& eventName,
        RuleType ruleType = RuleType::WHOLE_WORD) : ListenerRule(domain, eventName, "", ruleType) {}
    ListenerRule(const std::string& tag, RuleType ruleType = RuleType::WHOLE_WORD)
        : ListenerRule("", "", tag, ruleType) {}

public:
    std::string GetDomain() const
    {
        return domain;
    }
    std::string GetEventName() const
    {
        return eventName;
    }
    std::string GetTag() const
    {
        return tag;
    }
    RuleType GetRuleType() const
    {
        return ruleType;
    }

private:
    std::string domain;
    std::string eventName;
    std::string tag;
    RuleType ruleType;
};

class QueryRule {
public:
    QueryRule(const std::string& domain, const std::vector<std::string>& eventList)
        : domain(domain), eventList(eventList), ruleType(RuleType::WHOLE_WORD) {}

public:
    std::string GetDomain() const
    {
        return domain;
    }
    std::vector<std::string> GetEventList() const
    {
        return eventList;
    }
    RuleType GetRuleType() const
    {
        return ruleType;
    }

private:
    std::string domain;
    std::vector<std::string> eventList;
    RuleType ruleType;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // ARG_RULES_H
