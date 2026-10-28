# HiSysEvent 系统事件埋点组件指引

## 项目定位

本仓库对应 OpenHarmony `base/hiviewdfx/hisysevent`，是 hiviewdfx 子系统下的系统事件埋点（埋点/查询/监听）组件。对外提供 C++、C、JS/NAPI、ETS/ANI、Rust 多语言埋点与查询接口，内部通过 Unix Domain Socket 将编码后的事件发送给 hiview 数据源服务，并通过 IPC（IDL）代理对接 hiview 的事件查询与监听能力。

- 组件名：`@ohos/hisysevent`（`bundle.json`），版本 3.1，SysCap：`SystemCapability.HiviewDFX.HiSysEvent`，适配 `standard` 系统。
- 依赖组件（`bundle.json` deps）：`access_token`、`bounds_checking_function`、`c_utils`、`hilog`、`hitrace`、`ipc`、`jsoncpp`、`libuv`、`napi`、`node`、`safwk`、`samgr`、`storage_service`、`ets_frontend`、`runtime_core`。

优先按这些目录定位问题：

- `interfaces/native/innerkits/hisysevent/`：核心 C++/C 埋点库 `libhisysevent`，事件编码、传输、限流、校验。
- `interfaces/native/innerkits/hisysevent_easy/`：轻量级纯 C 埋点库 `libhisysevent_easy`，无 C++ 运行时依赖。
- `interfaces/native/innerkits/hisysevent_manager/`：查询与监听管理库 `libhisyseventmanager`。
- `interfaces/js/kits/napi/`：JS/NAPI 绑定 `hisysevent_napi`。
- `interfaces/ets/ani/hisysevent/`：ETS/ANI 绑定 `hisysevent_ani`。
- `interfaces/rust/innerkits/`：Rust 绑定 `hisysevent_rust`。
- `frameworks/native/`：`hisysevent` 命令行工具、`c_wrapper`（Rust 的 C 桥接）、`util` 工具库。
- `adapter/native/idl/`：IPC IDL 定义与 proxy/stub 适配，对接 hiview 查询/监听服务。
- `test/`：模块测试、单元测试、fuzz 测试。

### 按任务类型定位代码

| 任务类型 | 首选目录 | 关键文件 |
| --- | --- | --- |
| 修改埋点写入（C++） | `interfaces/native/innerkits/hisysevent/` | `hisysevent.h`, `hisysevent.cpp`, `def.h` |
| 修改埋点写入（C） | `interfaces/native/innerkits/hisysevent/` | `hisysevent_c.h`, `hisysevent_c.cpp` |
| 修改轻量级埋点 | `interfaces/native/innerkits/hisysevent_easy/` | `hisysevent_easy.h`, `hisysevent_easy.c`, `easy_def.h`, `easy_event_builder.c` |
| 修改事件编码格式 | `interfaces/native/innerkits/hisysevent/include/` | `raw_data_base_def.h`, `raw_data.h/cpp`, `raw_data_encoder.cpp`, `encoded_param.h/cpp` |
| 修改传输/Socket 逻辑 | `interfaces/native/innerkits/hisysevent/` | `transport.h/cpp`, `event_socket_factory.h/cpp` |
| 修改写入限流/频率控制 | `interfaces/native/innerkits/hisysevent/include/` | `write_controller.h`, `write_controller.cpp` |
| 修改域名/事件名/参数名校验 | `interfaces/native/innerkits/hisysevent/include/` | `stringfilter.h`, `stringfilter.cpp` |
| 修改域名屏蔽（DOMAIN_MASKS） | `interfaces/native/innerkits/hisysevent/include/` | `hisysevent.h`（`isMasked` 模板与 `DOMAIN_MASKS` 宏） |
| 修改查询/监听管理（C++） | `interfaces/native/innerkits/hisysevent_manager/` | `hisysevent_manager.h/cpp`, `hisysevent_base_manager.cpp`, `hisysevent_record.cpp` |
| 修改查询/监听管理（C） | `interfaces/native/innerkits/hisysevent_manager/` | `hisysevent_manager_c.h/cpp`, `hisysevent_listener_c.cpp`, `hisysevent_query_callback_c.cpp`, `hisysevent_record_c.h/cpp` |
| 修改查询规则/监听规则 | `interfaces/native/innerkits/hisysevent_manager/include/` | `hisysevent_rules.h`, `rule_type.h`, `hisysevent_record_convertor.cpp` |
| 修改 IPC 接口定义 | `adapter/native/idl/` | `ISysEventService.idl`, `ISysEventCallback.idl` |
| 修改 IPC Proxy/Delegate | `adapter/native/idl/src/` | `hisysevent_delegate.cpp`, `hisysevent_query_proxy.cpp`, `hisysevent_listener_proxy.cpp`, `query_sys_event_callback_stub.cpp` |
| 修改 IPC 传递数据结构 | `adapter/native/idl/include/` | `query_argument.h`, `sys_event_rule.h`, `sys_event_query_rule.h`, `parcelable_vector_rw.h`, `ash_mem_utils.h` |
| 修改 NAPI 绑定 | `interfaces/js/kits/napi/src/` | `napi_hisysevent_js.cpp`, `napi_hisysevent_init.cpp`, `napi_hisysevent_adapter.cpp`, `napi_hisysevent_querier.cpp`, `napi_hisysevent_listener.cpp`, `js_callback_manager.cpp` |
| 修改 ETS/ANI 绑定 | `interfaces/ets/ani/hisysevent/src/` | `hisysevent_ani.cpp`, `ani_hisysevent_querier.cpp`, `ani_hisysevent_listener.cpp`, `ani_callback_manager.cpp` |
| 修改 Rust 绑定 | `interfaces/rust/innerkits/src/` | `lib.rs`, `sys_event.rs`, `sys_event_manager.rs`, `macros.rs`, `utils.rs` |
| 修改 Rust 的 C 桥接 | `frameworks/native/c_wrapper/` | `hisysevent_c_wrapper.cpp`, `hisysevent_rust_manager.cpp`, `hisysevent_rust_listener.cpp`, `hisysevent_rust_querier.cpp` |
| 修改命令行工具 | `frameworks/native/` | `hisysevent_tool.h/cpp`, `hisysevent_tool_listener.cpp`, `hisysevent_tool_query.cpp`, `hisysevent_json_decorator.cpp`, `json_flatten_parser.cpp`, `main.cpp` |
| 修改共享工具 | `frameworks/native/util/` | `string_util.h/cpp` |
| 修改构建配置/ABI | 各目标 `BUILD.gn` | `bundle.json`, `libhisysevent.map`, `libhisyseventmanager.map` |

### 嵌套指引

本仓库无目录级别的嵌套指引。所有任务级指导均通过 `docs/knowledge/` 中的场景文档提供。

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。`bundle.json` 中 `segment.destPath` 为 `base/hiviewdfx/hisysevent`。

```sh
# 全量构建本组件
./build.sh --product-name rk3568 --build-target hisysevent --ccache

# 单独构建各库目标（ninja）
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 libhisysevent
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 libhisyseventmanager
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 hisysevent_napi
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 hisysevent_rust
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 hisysevent_c_wrapper
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 hisysevent            # 命令行可执行程序

# 构建测试目标
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 moduletest
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 unittest
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 fuzztest
```

`bundle.json` 中 `build.sub_component` 与 `build.test` 定义了全部构建与测试入口；`build.inner_kits` 定义了对其它子系统暴露的头文件库。

### 完成标准

任务被认为完成，当且仅当：

1. **代码改动已提交** - 使用 `git commit -s`，多代理协作时添加 `Co-Authored-By: Agent`。
2. **本地构建通过** - 执行上述构建命令，相关 `libhisysevent`/`libhisyseventmanager`/`hisysevent_napi`/`hisysevent_rust`/`hisysevent` 等目标编译通过。
3. **相关测试通过** - 对应模块测试（`test/moduletest`）、单元测试（`test/unittest`）、fuzz 测试（`test/fuzztest`）通过。
4. **板侧验证（如适用）** - 涉及传输/IPC/查询/监听/命令行工具的改动需提供板侧验证证据（埋点写入后用 `hisysevent` 命令或 `hidumper` 验证、hdc 输出）。
5. **文档更新（如适用）** - 公共 API 修改需更新 `README.md`/`README_zh.md` 及头文件注释；ABI 变更需同步更新 `.map` 版本脚本。

### 如果无法运行验证

明确说明无法运行的原因，列出推荐的验证步骤供人工执行，标记需要人工验证的部分。涉及传输/查询的改动至少应说明如何在板侧触发一次埋点写入并通过 `hisysevent` 工具订阅/查询来观察结果。

### 完成报告格式

报告应包含：改动摘要（文件列表、改动点）、验证结果（构建/测试输出）、风险评估（API 兼容性、ABI 兼容性、数据格式兼容性、性能风险）、未完成事项。

## 知识索引

稳定背景知识放在 `docs/knowledge/`。改动前按场景读取对应文件：

### 场景与路径路由

| 场景 | 修改目录 | 先读文档 |
| --- | --- | --- |
| 事件写入、参数编码、限流、域名屏蔽、传输/Socket、高频写入路径、轻量级埋点 | `interfaces/native/innerkits/hisysevent/`、`interfaces/native/innerkits/hisysevent_easy/` | `docs/knowledge/event-write-pipeline.md` |
| 编码二进制格式、RawData/EncodedParam/RawDataEncoder、HiSysEventHeader/TraceInfo/ParamValueType 紧凑结构体、ValueType/EncodeType 枚举、名字校验与转义、ABI/.map | `interfaces/native/innerkits/hisysevent/include/` | `docs/knowledge/event-encoding-format.md` |
| 事件查询、监听、订阅、导出、HiSysEventRecord 解析、IPC IDL/Delegate/Proxy/Stub、Ashmem、Parcel 数据结构 | `interfaces/native/innerkits/hisysevent_manager/`、`adapter/native/idl/` | `docs/knowledge/query-listener-model.md` |
| NAPI、ETS/ANI、Rust 多语言绑定、c_wrapper 桥接、命令行工具 `hisysevent` | `interfaces/js/`、`interfaces/ets/`、`interfaces/rust/`、`frameworks/native/c_wrapper/`、`frameworks/native/` | `docs/knowledge/multi-language-bindings.md` |
| 构建、板侧测试、PR 证据、重构建共享库、公开 API 或传输/查询行为验证 | 任何构建/测试相关改动 | `docs/knowledge/board-verification.md` |

### 历史教训

| 编号 | 主题 | 文档 |
| --- | --- | --- |
| KP-001 | 导出符号变更必须同步版本脚本（`.map`） | `docs/knowledge/key_history_issue/导出符号变更必须同步版本脚本.md` |
| KP-002 | 紧凑结构体/C 结构体布局是跨进程二进制协议与 ABI 契约 | `docs/knowledge/key_history_issue/紧凑结构体布局是跨进程二进制协议.md` |

### 开始编辑前

在修改代码前，按以下顺序确认：

1. 确认任务类别，根据"按任务类型定位代码"表确定目标目录。
2. 根据"场景与路径路由"表确定需要阅读的文档。
3. 根据"项目约束"确认不违反任何约束。
4. 声明："我将修改 X，已阅读 Y 文档，遵循 Z 约束"。

## 项目约束

### 性能约束

- `HiSysEvent::Write` 是高频调用路径（系统关键进程埋点）。不要在每次写入中增加全量扫描、字符串格式化、同步 IPC 或高于 `LOG_CORE` 的日志。
- 传输层使用 `AF_UNIX` `SOCK_DGRAM` 非阻塞 Socket 直接送往 hiview 数据源，每次写入新建并关闭 fd（避免跨线程锁竞争），不要改为复用单 fd + 大锁阻塞路径。
- `WriteController` 已实现 LRU 缓存的频率限流（默认 `HISYSEVENT_DEFAULT_PERIOD=5`，`HISYSEVENT_DEFAULT_THRESHOLD=100`），不要在写入主路径重复实现限流或新增阻塞等待。
- `libhisysevent_easy` 使用固定 1120 字节缓冲（`EVENT_BUFF_LEN`），不要引入动态内存分配或扩大缓冲。
- 所有共享库以 `-Os` 编译（`cflags_cc = ["-Os"]`），改动应保持精简；不要引入大体积依赖。

### 架构约束

- **编码层与传输层分离**：事件编码（`RawData`/`EncodedParam`/`RawDataEncoder`）与传输（`Transport`）职责分离，不要把 Socket 逻辑混入编码器，也不要在传输层解析事件语义。
- **多语言绑定统一后端**：NAPI/ANI/Rust 绑定必须复用 `libhisysevent` 与 `libhisyseventmanager`，Rust 通过 `c_wrapper` 桥接。不要在各语言绑定中重新实现编码或传输逻辑。
- **easy 库独立**：`libhisysevent_easy` 是无 C++ 运行时依赖的纯 C 静态库（CFI 启用），不得依赖 `libhisysevent`、`hilog`、`ipc` 等；其编码与 Socket 写入自行实现。
- **IPC 适配隔离**：跨进程查询/监听通过 `adapter/native/idl` 的 proxy/delegate 走 `ISysEventService`，不要在 `hisysevent_manager` 中直接拼装 IPC 调用。
- **域名屏蔽为编译期机制**：`DOMAIN_MASKS` + `isMasked` 模板在编译期裁剪，不要改为运行期判断，以免影响高频路径性能与二进制兼容。
- **限流参数可定制**：`HISYSEVENT_PERIOD`/`HISYSEVENT_THRESHOLD` 可在编译期覆盖，默认值改动需评估全局影响。

### 编码约定

- 命名空间统一为 `OHOS::HiviewDFX`（编码相关为 `OHOS::HiviewDFX::Encoded`），新增代码沿用。
- 日志使用 `HILOG_*`（`LOG_CORE` 域），并定义 `LOG_DOMAIN`（统一 `0xD002D08`）与 `LOG_TAG`（如 `HISYSEVENT`/`HISYSEVENT_TRANSPORT`）。埋点写入路径仅用 `HILOG_DEBUG`，避免影响性能。
- 返回值约定（`def.h`）：`SUCCESS=0` 为成功；负值为错误（`ERR_*`，事件被丢弃）；正值为告警（`ERR_KEY_NAME_INVALID` 等，部分数据被忽略但事件仍发送）。新增校验沿用此约定。
- 内存操作使用 `memcpy_s`/`securec`（`bounds_checking_function`），禁止裸 `memcpy`/`strcpy`。
- 文件描述符使用 `fdsan`（`fdsan_create_owner_tag`/`fdsan_close_with_tag`）保护，Socket 使用 `SOCK_NONBLOCK|SOCK_CLOEXEC`。
- C 接口头文件使用 `extern "C"` 守卫（`__cplusplus`），与 C++ 头文件隔离；公共 C 结构体使用 `typedef`。
- Rust 代码遵循 `rustfmt.toml`（edition 2021、`wrap_comments`、`imports_granularity = "Module"`、`group_imports = "StdExternalCrate"`）。

### 公共 API 约束

公共 API 头文件（`bundle.json` inner_kits）：`hisysevent.h`、`hisysevent_c.h`、`hisysevent_easy.h`、`hisysevent_manager.h`、`hisysevent_manager_c.h`、`hisysevent_record_c.h`，以及 Rust crate `hisysevent`、NAPI/ANI 接口。

**Do not（禁止）：**

- 修改已发布 C++/C API 的签名、参数类型、返回值类型（如 `HiSysEvent::Write`、`HiSysEventWrite` 宏、`OH_HiSysEvent_Write`、`HiSysEventEasyWrite`、`HiSysEventManager::AddListener/Query`、`OH_HiSysEvent_Query`/`OH_HiSysEvent_Add_Watcher`/`OH_HiSysEvent_Remove_Watcher`/`OH_HiSysEvent_GetParam*`）。
- 修改已有 API 的错误码语义（`def.h`/`easy_def.h` 中的 `ERR_*` 值与含义），除非明确标注废弃。
- 删除或重命名已有公共 API；修改导出符号（必须同步 `libhisysevent.map`/`libhisyseventmanager.map`）。
- 修改 `HiSysEvent::Domain` 中已有域名字符串值、`EventType` 枚举值（`FAULT=1/STATISTIC=2/SECURITY=3/BEHAVIOR=4`）。
- 修改已有校验上限常量（`MAX_DOMAIN_LENGTH=16`、`MAX_EVENT_NAME_LENGTH=32`、`MAX_PARAM_NAME_LENGTH=48`、`MAX_PARAM_NUMBER=128`、`MAX_ARRAY_SIZE=100`、`MAX_STRING_LENGTH`、`MAX_DATA_SIZE`）。
- 修改 NAPI/ANI/Rust 对应用暴露的接口签名与行为语义（如异步变同步、阻塞变非阻塞）。

**Ask before（修改前必须确认）：**

- 新增公共 API：确认是否需要 DFX 日志、hitrace 集成、权限检查；确认 `innerapi_tags`（`chipsetsdk_sp`/`platformsdk`/`sasdk`）归属与 `.map` 导出。
- 新增 `HiSysEvent::Domain` 域名常量：确认命名规范与上游对齐。
- 修改内部接口（如 `Transport`、`WriteController`、`HiSysEventRecord`）：评估是否影响跨模块与多语言绑定兼容性。
- 修改校验/限流逻辑：确认是否影响应用层错误码兼容性与埋点行为。

### 安全与权限边界

**Do not（禁止）：**

- 绕过 `access_token` 权限校验逻辑（NAPI/ANI 中调用 `libaccesstoken_sdk`）。
- 在未校验的情况下直接使用跨进程传递的文件描述符、共享内存（Ashmem，`ash_mem_utils.cpp`）。
- 将敏感事件内容（安全事件、含用户隐私的参数）写入非安全日志；写入路径仅允许 `HILOG_DEBUG`。
- 修改 `HiSysEvent::EventType::SECURITY` 类事件的处理逻辑，除非经过安全评审。

**Ask before（修改前必须确认）：**

- 涉及 `access_token` 相关代码的改动。
- 涉及 IPC 查询/监听权限控制（`ISysEventService` 服务端侧）的改动。
- 涉及 `storage_service`（`STORAGE_SERVICE_ENABLE`）导出/订阅逻辑的改动。
- 涉及隐私数据（用户标识、设备信息等）的事件参数处理。

### 协议与数据格式兼容性

**Do not（禁止）：**

- 修改 IDL 定义的 IPC 接口签名与 `[in]/[out]` 方向（`ISysEventService.idl`、`ISysEventCallback.idl`）。
- 修改 `HiSysEventHeader`、`TraceInfo`、`ParamValueType` 等 `#pragma pack(1)` 紧凑结构体的字段顺序、类型、位域布局（`raw_data_base_def.h`）——这是与 hiview 数据源的跨进程二进制协议。
- 修改 `ValueType`/`EncodeType` 枚举值（编码协议）。
- 修改 Parcel 序列化顺序与 `sequenceable` 数据结构布局（`SysEventRule`、`SysEventQueryRule`、`QueryArgument`）。
- 修改 C 公共结构体布局（`HiSysEventParam`、`HiSysEventQueryRule`、`HiSysEventWatchRule`、`HiSysEventWatcher`、`HiSysEventRecord`/`HiSysEventRecordC` 的字段顺序与定长数组大小）。
- 修改导出符号集而不更新 `libhisysevent.map`/`libhisyseventmanager.map`。

**Ask before（修改前必须确认）：**

- 新增 IPC 接口：确认是否需要跨版本兼容性处理。
- 新增事件参数类型：确认编码格式与 hiview 解码端兼容。
- 修改 Socket 传输地址/协议（`event_socket_factory`）：确认与 hiview 数据源 socket 路径一致。

### 生成代码边界

**Do not（禁止）：**

- 直接修改 IDL 编译器生成的 `sys_event_service_proxy.cpp`、`sys_event_callback_stub.cpp`（位于 `${target_gen_dir}`，由 `idl_gen_interface("sys_event_interface")` 生成）。
- 手动编辑生成的 IPC Proxy/Stub 代码。

**正确做法：**

- 修改 IDL 定义文件（`ISysEventService.idl`/`ISysEventCallback.idl`）。
- 重新运行 IDL 编译器生成代码（构建时自动）。
- 如生成代码不满足需求，调整 IDL 定义或使用回调机制（`IQuerySysEventCallback`/`ISysEventCallback`）。

### 设备操作约束

**涉及真实设备时的注意事项：**

- 传输层通过 Unix Domain Socket 向 hiview 数据源写事件，不要执行可能影响 hiview 服务正常运行的破坏性操作（如异常关闭数据源 socket）。
- 涉及传输/查询/监听/命令行工具的改动，必须在真实设备上验证：触发埋点写入后通过 `hisysevent` 命令行工具订阅或查询，提供板侧证据（日志、`hidumper`、`hdc` 输出）。
- 查询/导出涉及 `storage_service`（`STORAGE_SERVICE_ENABLE`）时，需验证导出文件路径与权限。
- Socket 操作需保持非阻塞与 `fdsan` 保护，避免 fd 泄漏。
