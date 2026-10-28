# 查询与监听模型知识

本文记录事件查询、监听、订阅、导出与记录解析的架构。写入主链路见 `event-write-pipeline.md`，IPC IDL 与代理见下文"IPC 适配"。

## 两层 API

查询/监听对外暴露 C++ 与 C 两套 API，底层共用同一代理：

| 层 | 头文件 | 关键接口 |
| --- | --- | --- |
| C++ | `hisysevent_manager.h` | `HiSysEventManager::AddListener`/`RemoveListener`/`Query` |
| C++ 回调 | `hisysevent_listener.h`、`hisysevent_query_callback.h` | `HiSysEventListener::OnEvent`/`OnServiceDied`、`HiSysEventQueryCallback` |
| C | `hisysevent_manager_c.h` | `OH_HiSysEvent_Query`、`OH_HiSysEvent_Add_Watcher`、`OH_HiSysEvent_Remove_Watcher`、`OH_HiSysEvent_GetParam*` |

`HiSysEventManager`（`hisysevent_manager.cpp`）用 `listenerToBaseMap_` 把 `HiSysEventListener` 包装成 `HiSysEventBaseListener`，再委托 `HiSysEventBaseManager` → `HiSysEventDelegate`。Query 每次新建 `HiSysEventBaseQueryCallback` 包装用户回调。不要在 manager 层直接拼装 IPC 调用。

## 规则与记录

- **规则**：`hisysevent_rules.h`（`ListenerRule`、`QueryRule`）、`rule_type.h`（`RuleType::WHOLE_WORD/PREFIX/REGULAR`）。C 侧为 `HiSysEventWatchRule`、`HiSysEventQueryRule`（含 `MAX_NUMBER_OF_EVENT_LIST=10`、`MAX_NUMBER_OF_WATCH_EVENT_LIST=20`）。
- **记录**：`HiSysEventRecord`（`hisysevent_record.h`）从 hiview 返回的 JSON 解析，提供 `GetDomain`/`GetEventName`/`GetEventType`/`GetTime`/`GetPid` 等及 `GetParamValue`（按类型重载）。C 侧 `HiSysEventRecordC`（`hisysevent_record_c.h`）与 `OH_HiSysEvent_GetParam*` 取值。`hisysevent_record_convertor.cpp` 负责两者转换与内存释放（`InitRecord`/`DeleteRecord`/`DeleteRecords`）。

C 公共结构体布局（`HiSysEventQueryArg`、`HiSysEventQueryRule`、`HiSysEventWatchRule`、`HiSysEventWatcher`、`HiSysEventRecordC`）的字段顺序与定长数组大小不可变。

## IPC 适配

跨进程查询/监听通过 `adapter/native/idl/`，不在 manager 层直接实现：

| 组件 | 文件 | 职责 |
| --- | --- | --- |
| IDL 定义 | `ISysEventService.idl`、`ISysEventCallback.idl` | `AddListener`/`RemoveListener`/`Query`/`AddSubscriber`/`RemoveSubscriber`/`Export`；回调 `Handle` |
| Delegate | `hisysevent_delegate.cpp` | 规则转换（`ConvertListenerRule`/`ConvertQueryRule`）、获取服务（`GetSysEventService` 经 samgr）、目录与权限设置（`CreateHiviewDir`/`SetDirPermission`） |
| Proxy | `hisysevent_listener_proxy`、`hisysevent_query_proxy` | IPC 调用封装 |
| Stub | `query_sys_event_callback_stub` | 查询回调服务端桩 |
| 数据结构 | `query_argument.h`、`sys_event_rule.h`、`sys_event_query_rule.h`、`parcelable_vector_rw.h` | Parcel 序列化（`sequenceable`），布局不可变 |
| 共享内存 | `ash_mem_utils.h/cpp` | 导出（`Export`）大数据经 Ashmem 传递，`STORAGE_SERVICE_ENABLE` 时涉及 `storage_service` |

生成的 proxy/stub（`sys_event_service_proxy.cpp`、`sys_event_callback_stub.cpp`）由 `idl_gen_interface("sys_event_interface")` 生成在 `${target_gen_dir}`，不要手改。

## 修改前检查

- 接口是 C++ 还是 C？两者规则结构体不同，改动需同步。
- 是否触达 IPC？若是，确认 IDL 签名、Parcel 顺序、`sequenceable` 布局不变。
- 是否涉及导出（`Export`/`Subscribe`）？若是，确认 `storage_service` 与 Ashmem 路径。
- 记录解析改动是否影响 `GetParamValue` 的类型匹配与返回码（`VALUE_PARSED_SUCCEED`/`ERR_KEY_NOT_EXIST`/`ERR_TYPE_NOT_MATCH`）？

## 测试指引

- C++/C 管理 API：`hisysevent_manager_c_test.cpp`、`hisysevent_adapter_native_test.cpp`、`hisysevent_wrote_result_check_test.cpp`。
- IPC 变更：补充板侧证据（板侧发起查询/订阅，用 `hisysevent` 工具触发事件并观察回调），见 `board-verification.md`。
- fuzz：`fuzztest/common/hisyseventmanager_fuzzer`。
