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

#include "js_callback_manager.h"

#include "hilog/log.h"
#include "uv.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "JS_CALLBACK_MANAGER"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int CONTEXT_INDEX = 0;
constexpr int CALLBACK_FUNC_INDEX = 1;
constexpr int RELEASE_FUNC_INDEX = 2;
struct CallbackWorkData {
    std::shared_ptr<CallbackContext> context;
    CALLBACK_FUNC callback;
    RELEASE_FUNC release;
};
void DeleteWork(uv_work_t* work)
{
    if (work != nullptr) {
        delete reinterpret_cast<CallbackWorkData*>(work->data);
        work->data = nullptr;
        delete work;
    }
}

void OnWorkExecute(uv_work_t* work)
{
    HILOG_DEBUG(LOG_CORE, "enter uv work callback.");
}

void OnWorkDone(uv_work_t* work, int status)
{
    if (work == nullptr) {
        return;
    }
    auto* data = reinterpret_cast<CallbackWorkData*>(work->data);
    if (data == nullptr) {
        HILOG_ERROR(LOG_CORE, "uv work data is invalid.");
        DeleteWork(work);
        return;
    }
    auto context = data->context;
    if (context == nullptr || context->env == nullptr) {
        HILOG_ERROR(LOG_CORE, "napi environment is invalid.");
        DeleteWork(work);
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(context->env, &scope);
    if (scope == nullptr) {
        HILOG_ERROR(LOG_CORE, "napi scope is null.");
        DeleteWork(work);
        return;
    }
    if (data->callback != nullptr) {
        data->callback(context->env, context->ref, context->threadId);
    }
    napi_close_handle_scope(context->env, scope);
    if (data->release != nullptr) {
        data->release(context->threadId);
    }
    DeleteWork(work);
}

void RunCallback(std::shared_ptr<CallbackContext> context,
    std::tuple<std::shared_ptr<CallbackContext>, CALLBACK_FUNC, RELEASE_FUNC>& current)
{
    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(context->env, &loop);
    if (loop == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to get uv_loop.");
        return;
    }
    uv_work_t* work = new(std::nothrow) uv_work_t();
    if (work == nullptr) {
        HILOG_ERROR(LOG_CORE, "uv_work new failed, no memory left.");
        return;
    }
    auto* data = new(std::nothrow) CallbackWorkData{
        context,
        std::get<CALLBACK_FUNC_INDEX>(current),
        std::get<RELEASE_FUNC_INDEX>(current)
    };
    if (data == nullptr) {
        HILOG_ERROR(LOG_CORE, "callback work data new failed, no memory left.");
        DeleteWork(work);
        return;
    }
    work->data = reinterpret_cast<void*>(data);
    uv_queue_work_with_qos(loop, work, OnWorkExecute, OnWorkDone, uv_qos_default);
}
}

void JsCallbackManager::Add(std::shared_ptr<CallbackContext> context, CALLBACK_FUNC callback,
    RELEASE_FUNC release)
{
    {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (isReleased.load(std::memory_order_acquire)) {
            return;
        }
        jsCallbacks.emplace(std::make_tuple(context, callback, [this, release] (pid_t threadId) {
            if (release == nullptr) {
                this->ImmediateRun(true);
            } else {
                // Destructor of JsCallbackManager will be called in release callback,
                // so no need to call next callback in queue.
                release(threadId);
            }
        }));
        if (inCalling.load(std::memory_order_acquire)) {
            return;
        }
        inCalling = true;
    }
    ImmediateRun();
}

void JsCallbackManager::Release()
{
    std::lock_guard<std::mutex> lock(managerMutex);
    isReleased = true;
    Clear(jsCallbacks);
    inCalling = false;
}

void JsCallbackManager::ImmediateRun(bool needPop)
{
    inCalling = true;
    std::tuple<std::shared_ptr<CallbackContext>, CALLBACK_FUNC, RELEASE_FUNC> current;
    std::shared_ptr<CallbackContext> context;
    {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (isReleased.load(std::memory_order_acquire)) {
            inCalling = false;
            return;
        }
        if (needPop && !jsCallbacks.empty()) {
            jsCallbacks.pop();
        }
        if (jsCallbacks.empty()) {
            inCalling = false;
            return;
        }
        current = jsCallbacks.front();
        context = std::get<CONTEXT_INDEX>(current);
        if (context == nullptr) {
            inCalling = false;
            return;
        }
    }
    RunCallback(context, current);
}

void JsCallbackManager::Clear(TaskQueue& tasks)
{
    TaskQueue empty;
    swap(empty, tasks);
}
} // HiviewDFX
} // OHOS
