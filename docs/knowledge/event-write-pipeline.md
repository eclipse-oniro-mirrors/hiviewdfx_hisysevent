# 事件写入流水线知识

本文记录系统事件埋点主链路中容易被改错的边界。编码二进制格式见 `event-encoding-format.md`，查询/监听见 `query-listener-model.md`，多语言绑定见 `multi-language-bindings.md`。

## 主链路

事件写入应保持阶段清晰：

1. **限流**：`WriteController::CheckLimitWritingEvent`（LRU 缓存，默认周期 `HISYSEVENT_DEFAULT_PERIOD=5`、阈值 `HISYSEVENT_DEFAULT_THRESHOLD=100`）。超频返回 `ERR_WRITE_IN_HIGH_FREQ`，事件不再编码。
2. **校验与构建**：`EventBase` 构造时校验 domain（≤16）、eventName（≤32），通过 `StringFilter::IsValidName`（FSM），用 `memcpy_s` 写入 `HiSysEventHeader`。
3. **写基础信息**：`WritebaseInfo` 分配 `RawData`，依次写入 blockSize 占位、`HiSysEventHeader`、可选 `TraceInfo`（`HIVIEWDFX_HITRACE_ENABLED`）、paramCnt 占位。
4. **编码参数**：`InnerWrite` 变参模板递归，对每个键值对 `CheckParamValidity`（CheckKey 校验参数名 ≤48、`UpdateAndCheckKeyNumIsOver` 校验参数数 ≤128），再 `AppendParam` → `EncodedParam::Encode`。
5. **发送**：`SendSysEvent` 回填 blockSize/paramCnt，`Transport::SendData` 经 Unix Domain Socket 送往 hiview 数据源。

策略决策（限流、校验、屏蔽）不要下沉到编码或传输代码。改动写入行为时，先在限流/校验层处理，再进入编码与传输。

## 两条写入路径

| 路径 | 锚点 | 特点 |
| --- | --- | --- |
| 标准 C++/C | `hisysevent.h`/`hisysevent.cpp`、`hisysevent_c.cpp`、`libhisysevent` | 变参模板 `HiSysEvent::Write`、`HiSysEventWrite` 宏；支持全类型参数；`RawData` 动态编码；`-Os`。 |
| 轻量级纯 C | `hisysevent_easy.h`、`libhisysevent_easy` | `HiSysEventEasyWrite(domain, name, type, data)` 传入 JSON 字符串；固定 1120 字节缓冲（`EVENT_BUFF_LEN`）；无 C++ 运行时、无 hilog、无 ipc；CFI 启用。 |

两条路径最终都通过 Unix Domain Socket 投递给 hiview 数据源，但编码与 Socket 写入各自独立实现，不要互相混用。

## 域名屏蔽

`DOMAIN_MASKS` 宏 + `isMasked<domain>` 模板在编译期裁剪：被屏蔽域名的 `Write` 直接返回 `ERR_DOMAIN_MASKED`，不进入编码。屏蔽是编译期机制，不要改为运行期判断，以免影响高频路径性能与二进制兼容。

可通过头文件 `#define DOMAIN_MASKS "AAFWK|ACCOUNT"` 或 `BUILD.gn` 的 `-D DOMAIN_MASKS=...` 配置。

## 传输层

`Transport` 为单例，`SendData` 顺序：

1. 空数据返回 `ERR_EMPTY_EVENT`；超 `MAX_DATA_SIZE`（384KB）返回 `ERR_OVER_SIZE`。
2. `RetrySendFailedData` 先重试历史失败队列（`RETRY_QUEUE_SIZE=10`）。
3. `SendToHiSysEventDataSource` 每次新建 `AF_UNIX` `SOCK_DGRAM` `SOCK_NONBLOCK` `SOCK_CLOEXEC` fd（避免跨线程锁竞争），`fdsan` 打标签，`setsockopt SO_SNDBUF=MAX_DATA_SIZE`，`sendto` 重试 `RETRY_TIMES=3`（仅 `EAGAIN`/`EWOULDBLOCK`/`EINTR`）。
4. 失败入 `AddFailedData`，返回 `ERR_SEND_FAIL`。

不要把 Socket 逻辑混入编码器，也不要改为复用单 fd + 大锁阻塞路径。

## 返回值约定

- `SUCCESS=0`：成功。
- 负值（`ERR_DOMAIN_NAME_INVALID` 等，见 `def.h`）：错误，事件被丢弃。
- 正值（`ERR_KEY_NAME_INVALID` 等）：告警，部分参数被忽略但事件仍发送。

新增校验沿用此约定；错误码值与含义不得变更（公共 API 兼容）。

## 高频路径

`HiSysEvent::Write` 被系统关键进程高频调用。不要在每次写入中增加全量扫描、字符串格式化、同步 IPC 或高于 `LOG_CORE` 的日志。写入路径仅允许 `HILOG_DEBUG`。`WriteController` 已实现限流，不要在主路径重复实现限流或新增阻塞等待。

## 测试指引

- 编码/校验/限流变更：使用 `test/moduletest/common/hisysevent_encoded_test.cpp`、`hisysevent_native_test.cpp`、`hisysevent_c_test.cpp`、`hisysevent_delay_test.cpp`。
- 传输/Socket 变更：补充 `board-verification.md` 中的板侧证据（写入后用 `hisysevent` 工具订阅观察）。
- 轻量级埋点：使用 `hisysevent_easy_test.cpp`。
- fuzz：`fuzztest/common/hisysevent_fuzzer`。
