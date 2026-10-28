# 多语言绑定架构知识

本文记录 NAPI、ETS/ANI、Rust 三套语言绑定如何统一复用原生库。写入主链路见 `event-write-pipeline.md`，查询/监听见 `query-listener-model.md`。

## 统一后端原则

三套绑定都必须复用 `libhisysevent`（埋点）与 `libhisyseventmanager`（查询/监听），不得在各语言绑定中重新实现编码、传输或 IPC 逻辑。Rust 因 FFI 限制，经 `c_wrapper` 桥接后再调原生库。

```
NAPI (hisysevent_napi) ─────────────┐
ETS/ANI (hisysevent_ani) ───────────┼─► libhisysevent + libhisyseventmanager
Rust (hisysevent_rust) ──► c_wrapper ┘
```

## NAPI 绑定

- 目录：`interfaces/js/kits/napi/`，目标 `hisysevent_napi`（`ohos_shared_library`，`relative_install_dir = "module"`）。
- 源文件：`napi_hisysevent_js.cpp`（JS 接口）、`napi_hisysevent_init.cpp`（模块注册）、`napi_hisysevent_adapter.cpp`（适配原生库）、`napi_hisysevent_querier.cpp`/`napi_hisysevent_listener.cpp`（查询/监听回调）、`js_callback_manager.cpp`（JS 回调线程管理）、`napi_callback_context.cpp`、`napi_hisysevent_util.cpp`、`ret_def.h`。
- 依赖：`libhisysevent`、`libhisyseventmanager`；`external_deps` 含 `access_token:libaccesstoken_sdk`（权限校验）、`napi:ace_napi`、`node`、`libuv`。
- 受 `support_jsapi` 开关控制（`interfaces/js/kits/BUILD.gn`）。

## ETS/ANI 绑定

- 目录：`interfaces/ets/ani/hisysevent/`，目标 `hisysevent_ani`（+ `hisysevent_etc`），受 `support_jsapi` 控制。
- 源文件：`hisysevent_ani.cpp`（接口）、`ani_hisysevent_querier.cpp`/`ani_hisysevent_listener.cpp`（回调）、`ani_callback_manager.cpp`、`ani_callback_context.cpp`、`hisysevent_ani_util.cpp`、`ret_def.h`。
- 与 NAPI 结构对称，同样复用原生库。

## Rust 绑定

- 目录：`interfaces/rust/innerkits/`，crate `hisysevent`（`dylib`，`ohos_rust_shared_library`）。
- 源文件：`lib.rs`（`write`/`query`/`add_watcher`/`remove_watcher` 及 `EventType`/`RuleType`/`QueryArg`/`QueryRule`/`WatchRule`）、`sys_event.rs`（埋点参数构造）、`sys_event_manager.rs`（`HiSysEventRecord`/`Querier`/`Watcher`）、`macros.rs`、`utils.rs`。
- 依赖：`c_wrapper`（`hisysevent_c_wrapper`）、`libhisysevent`、`libhisyseventmanager`。Rust 侧通过 FFI 调 `c_wrapper` 的 C 接口，再由 C++ 实现调原生库。
- 格式化遵循 `rustfmt.toml`（edition 2021、`wrap_comments`、`imports_granularity = "Module"`、`group_imports = "StdExternalCrate"`）。
- Rust 测试受 `!use_clang_coverage` 控制（`test/unittest/rust`）。

## c_wrapper（Rust 的 C 桥接）

- 目录：`frameworks/native/c_wrapper/`，目标 `hisysevent_c_wrapper`（`ohos_shared_library`）。
- 源文件：`hisysevent_c_wrapper.cpp`、`hisysevent_rust_manager.cpp`、`hisysevent_rust_listener.cpp`、`hisysevent_rust_querier.cpp`。
- 暴露 C 接口供 Rust FFI 调用，内部调 `libhisysevent`/`libhisyseventmanager`。不要在 c_wrapper 中引入业务逻辑，只做类型转换与生命周期桥接。

## 命令行工具（非语言绑定，但共用后端）

- 目录：`frameworks/native/`，可执行程序 `hisysevent`（`hisyseventcat_package`）。
- `hisysevent_tool.cpp` 解析命令行（`ArgStuct`：domain/eventName/tag/beginTime/endTime/maxEvents/ruleType 等），`hisysevent_tool_listener.cpp`/`hisysevent_tool_query.cpp` 订阅/查询，`hisysevent_json_decorator.cpp`/`json_flatten_parser.cpp` 格式化输出。
- 依赖 `sys_event_impl_client`（IPC）、`libhisysevent`、`libhisyseventmanager`。
- `hisysevent_feature_support_usr_symlink` 控制是否建立 `/usr/bin/hisysevent` 软链。

## 修改前检查

- 绑定改动是否绕过原生库重新实现了编码/传输/IPC？必须复用后端。
- 新增语言接口是否与 C++/C 公共 API 语义一致（错误码、异步/同步行为）？
- 回调线程管理（`js_callback_manager`/`ani_callback_manager`）改动是否影响回调时序与生命周期？
- `support_jsapi` 关闭场景下（无 JS 运行时）是否仍能构建 native 库？

## 测试指引

- NAPI：`test/unittest/common/napi:HiSysEventJsTest`、`test/unittest/common/permission:HiSysEventPermissionJsTest`。
- Rust：`test/unittest/rust:rust_hisysevent_test`（`!use_clang_coverage`）。
- c_wrapper：`frameworks/native/test/unittest/common:hisysevent_c_wrapper_test`。
- 工具：`frameworks/native/test/unittest/common:hisysevent_tool_unit_test`。
