# 事件编码格式与二进制协议知识

本文记录事件编码的二进制格式——这是与 hiview 数据源之间的跨进程二进制协议，字段布局不可变。写入主链路见 `event-write-pipeline.md`。

## 协议层

事件编码分三层，均在 `interfaces/native/innerkits/hisysevent/` 下，命名空间 `OHOS::HiviewDFX::Encoded`：

1. **`RawData`**（`raw_data.h`）：动态字节缓冲，提供 `Append`/`Update`/`GetData`/`GetDataLength`。编码产物和 Socket 负载都基于它。
2. **`RawDataEncoder`**（`raw_data_encoder.cpp`）：底层编码原语——`ValueTypeEncoded`（写 tag）、`UnsignedVarintEncoded`/`SignedVarintEncoded`（varint）、`FloatingNumberEncoded`、`StringValueEncoded`（length-delimited）、`EncodedTag`。
3. **`EncodedParam`**（`encoded_param.h`）：参数封装，`Encode()` = `EncodeKey()` + `EncodeValueType()` + `EncodeValue()`；子类按类型选择编码原语。

## 紧凑结构体（不可变布局）

`raw_data_base_def.h` 中 `#pragma pack(1)` 定义的头与类型标识，直接 `memcpy` 进 `RawData`，是 hiview 解码端依赖的二进制协议：

| 结构体 | 字段 | 说明 |
| --- | --- | --- |
| `HiSysEventHeader` | `domain[17]`、`name[33]`、`timestamp u64`、`timeZone u8`、`uid u32`、`pid u32`、`tid u32`、`id u64`、`type:2`、`isTraceOpened:1` | 事件头，顺序、类型、位域不可变 |
| `TraceInfo` | `traceFlag u8`、`traceId u64`、`spanId u32`、`pSpanId u32` | 仅 `isTraceOpened==1` 时写入 |
| `ParamValueType` | `isArray:1`、`valueType:4`、`valueByteCnt:3` | 每个参数前的类型 tag |

**禁止**修改这些结构体的字段顺序、类型、位域布局、定长数组大小——会破坏与 hiview 的二进制兼容。

## 类型与编码枚举（不可变值）

- `ValueType`：`UNKNOWN=0, BOOL, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, FLOAT, DOUBLE, STRING`。
- `EncodeType`：`VARINT=0, LENGTH_DELIMITED=1, INVALID=4`。

枚举值被写进二进制流，修改会导致 hiview 解码错误。

## 参数编码映射

| 参数类型 | EncodedParam 子类 | ValueType | 编码 |
| --- | --- | --- | --- |
| bool/int8/int16/int32/int64 | `SignedVarintEncodedParam` | `INT64` | varint |
| uint8/uint16/uint32/uint64 | `UnsignedVarintEncodedParam` | `UINT64` | varint |
| float | `FloatingNumberEncodedParam` | `FLOAT` | 定长 |
| double | `FloatingNumberEncodedParam` | `DOUBLE` | 定长 |
| string | `StringEncodedParam` | `STRING` | length-delimited |
| 上述数组 | `*EncodedArrayParam` | 同标量 + isArray | length-delimited(个数) + 元素序列 |

浮点非有限值（`!std::isfinite`）被规整为 `0.0`。数组构造时超过 `MAX_ARRAY_SIZE=100` 截断。

## 名字校验与转义

`StringFilter`（`stringfilter.h`，单例）：

- `IsValidName(text, maxSize)`：有限状态机校验 domain/eventName/key，仅允许 `0-9 a-z A-Z _` 且以字母开头。
- `EscapeToRaw(text)`：查表转义特殊字符为原始形式。

校验上限常量（`def.h`）不可变：domain ≤16、eventName ≤32、paramName ≤48、paramNumber ≤128、arraySize ≤100、stringLen ≤256KB、totalData ≤384KB。

## ABI 控制

`libhisysevent` 与 `libhisyseventmanager` 使用版本脚本（`libhisysevent.map`/`libhisyseventmanager.map`）控制导出符号，`local: *` 隐藏其余。新增/删除导出符号必须同步更新 `.map`，否则破坏下游链接。`innerapi_tags` 为 `chipsetsdk_sp`/`platformsdk`/`sasdk`（libhisysevent）、`platformsdk`（libhisyseventmanager）。

## 测试指引

- 编码格式变更：`hisysevent_encoded_test.cpp`，覆盖各类型标量与数组。
- 校验/转义：`hisysevent_native_test.cpp`、`hisysevent_c_test.cpp`。
- 导出符号：构建后用 `nm`/`readelf` 对照 `.map` 确认。
