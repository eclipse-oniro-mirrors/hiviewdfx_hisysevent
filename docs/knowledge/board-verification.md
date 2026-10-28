# 板侧验证知识

本文记录 HiSysEvent 变更需要板侧证据时的稳定做法。

## 构建位置

OpenHarmony 构建命令从源码根目录执行，本仓库 `segment.destPath` 为 `base/hiviewdfx/hisysevent`：

```sh
cd <openharmony-source-root>
# 全量构建组件
./build.sh --product-name rk3568 --build-target hisysevent --ccache
# 单独构建库/工具/测试（ninja）
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 libhisysevent
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 libhisyseventmanager
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 hisysevent_napi
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 hisysevent_rust
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 hisysevent        # 命令行可执行程序
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 moduletest
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 unittest
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 fuzztest
```

涉及传输、IPC、查询/监听、命令行工具或公开行为时，构建相关目标并视情况在板侧运行。

## 何时需要板侧证据

| 变更类型 | 最低证据 |
| --- | --- |
| 传输/Socket（`transport.cpp`、`event_socket_factory`） | 构建并通过板侧写入事件，用 `hisysevent` 工具订阅确认到达。 |
| IPC 查询/监听/订阅/导出（`adapter/native/idl`、`hisysevent_manager`） | 构建并在板侧发起查询/订阅，触发事件并观察回调。 |
| 编码格式（`raw_data_*`、`encoded_param`） | 构建并运行编码测试；涉及与 hiview 解码兼容时增加板侧写入验证。 |
| 命令行工具 `hisysevent` | 构建并在板侧运行订阅/查询命令。 |
| 轻量级埋点 `libhisysevent_easy` | 构建并在板侧写入，用工具确认。 |
| 多语言绑定（NAPI/ANI/Rust） | 构建对应目标并运行对应测试；触达权限/IPC 时增加板侧证据。 |
| 仅内部重构/纯头文件注释 | 除非运行时行为变化，否则运行最近单元测试即可。 |

## 板侧验证方式

### 1. 写入并订阅

在板侧用 `hisysevent` 命令行工具订阅指定域事件，再触发埋点写入，确认事件到达：

```sh
# 订阅（另起终端）
hdc shell hisysevent -r -d AAFWK
# 触发埋点（应用或测试程序调用 HiSysEvent::Write / OH_HiSysEvent_Write）
# 观察订阅输出是否包含预期事件
```

### 2. 查询历史事件

```sh
hdc shell hisysevent -l -s 0 -e <end_time_ms> -m 100 -d AAFWK -n start_app
```

### 3. 重构共享库后推库运行测试

把测试二进制和本地重构建的共享库推到同一临时目录，让该目录排在 `LD_LIBRARY_PATH` 最前：

```sh
hdc file send out/rk3568/tests/moduletest/hisysevent/moduletest <device-temp-dir>/
hdc file send out/rk3568/base/hiviewdfx/hisysevent/libhisysevent.so <device-temp-dir>/
hdc shell "cd <device-temp-dir> && LD_LIBRARY_PATH=<device-temp-dir>:/system/lib ./moduletest --gtest_filter=*HiSysEventNative*"
```

记录板侧镜像、产品、测试命令、gtest filter、通过/失败数量和已知环境问题。不要把 `hdc` 连接、权限或 hiview 服务未启动报成行为通过。

### 4. hidumper 辅助

```sh
hdc shell hidumper -s HiSysEventService
```

## PR 证据模板

```text
构建：
- 产品：rk3568
- 目标：<libhisysevent / hisysevent / moduletest 等>
- 结果：<通过/失败，已检查命令输出>

板侧：
- 设备/镜像：<相关时填写标识>
- 推送文件：<测试二进制和重构建库，如 libhisysevent.so>
- 命令：<精确的 hdc shell 命令，如 hisysevent -r -d AAFWK 或 gtest 过滤器>
- 结果：<事件到达确认 / 通过失败数量>
- 环境说明：<无，或已知 hiview 未启动/权限等环境问题>
```
