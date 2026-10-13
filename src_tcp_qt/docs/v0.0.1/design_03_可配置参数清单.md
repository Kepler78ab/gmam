# 设计 — module_tcp 可配置参数清单

> 汇总我们能自己控制的全部参数（名称可在实现时微调，语义以此为准）。  
> 配套：[design_01](./design_01_可配置心跳与应答.md) · [design_02](./design_02_默认策略与PES兼容.md)

**图例**

- **层**：`核心` = 模块配置结构字段；`回调` = 应用注入；`Profile` = 可选预设包（如 PES）  
- **P0/P1**：建议落地版本  

---

## 1. 链路与连接

| 参数 | 类型（示意） | 核心默认 | 含义 | 层 | 版本 |
|------|--------------|----------|------|----|------|
| `host` | string | （必填） | 远端 IP/主机名 | 核心 | P0 |
| `port` | uint16 | （必填） | 远端端口 | 核心 | P0 |
| `autoReconnect` | bool | `true` | 断线/错误后是否自动重连 | 核心 | P0 |
| `reconnectIntervalMs` | int | `3000` | 重连间隔 | 核心 | P0 |
| `connectTimeoutMs` | int | `3000` | 单次连接超时 | 核心 | P0 |
| `localBindAddress` | string | 空=系统默认 | 可选本地绑定 | 核心 | 其后 |
| `localBindPort` | uint16 | `0` | 可选本地端口 | 核心 | 其后 |

---

## 2. 发送队列与节流

| 参数 | 类型（示意） | 核心默认 | 含义 | 层 | 版本 |
|------|--------------|----------|------|----|------|
| `sendQueueEnabled` | bool | `true` | 是否走发送队列（false 则调用即 write，慎用） | 核心 | P0 |
| `sendQueueMaxSize` | int | `100` | 队列上限；超出策略见下 | 核心 | P0 |
| `sendQueueOverflowPolicy` | enum | `Reject` | `Reject` 拒收并通知 / `DropOldest` | 核心 | P1 |
| `sendIntervalMs` | int | `0` | 两帧之间最小间隔；`0`=不额外限速（旧工程约 200ms 可作 Profile） | 核心 | P1 |
| `highWaterMark` | int | `100` | 达到后发「队列将满」类信号 | 核心 | P1 |
| `lowWaterMark` | int | `50` | 回落到此发「队列恢复」 | 核心 | P1 |

---

## 3. 心跳

| 参数 | 类型（示意） | 核心默认 | 含义 | 层 | 版本 |
|------|--------------|----------|------|----|------|
| `heartbeatEnabled` | bool | `false` | 是否周期发心跳 | 核心 | P1（字段 P0 可预留） |
| `heartbeatIntervalMs` | int | `5000` | 发送周期 | 核心 | P1 |
| `heartbeatFrame` | bytes | 空 | 固定心跳整帧；与 provider 二选一或优先 provider | 核心 | P1 |
| `heartbeatFrameProvider` | callback | 空 | 每次触发时取帧（动态内容） | 回调 | P1 |
| `heartbeatExpectReply` | bool | `false` | 是否检测对端心跳回复 | 核心 | P1 |
| `heartbeatReplyMatcher` | callback | 空 | 何谓「心跳回复」（仅 expectReply=true） | 回调 | 其后 |
| `heartbeatMissLimit` | int | `0` | 连续未收到回复次数后告警/重连；`0`=不启用 | 核心 | 其后 |
| `heartbeatOccupyAckWindow` | bool | `false` | 心跳是否占用 wait-ack 槽（默认不占） | 核心 | P1 |

PES 时：打开 `heartbeatEnabled`，`heartbeatFrame` = GMAM 组好的 H 电文。

---

## 4. 正负应答（wait-ack）

| 参数 | 类型（示意） | 核心默认 | 含义 | 层 | 版本 |
|------|--------------|----------|------|----|------|
| `ackEnabled` | bool | `false` | 数据帧是否等应答再发下一条 | 核心 | P1 |
| `ackTimeoutMs` | int | `2000` | 超时未匹配到 A/N 则重发 | 核心 | P1 |
| `ackMaxRetry` | int | `5` | 含首次在内的最大发送次数（或「重试次数」实现时定清） | 核心 | P1 |
| `ackWindow` | int | `1` | 同时未确认帧数；P1 固定 1（停等） | 核心 | P1 |
| `ackPositiveMatcher` | callback | 空 | 输入一帧 → 是否正应答 | 回调 | P1 |
| `ackNegativeMatcher` | callback | 空 | 输入一帧 → 是否负应答 | 回调 | P1 |
| `heartbeatIgnoreMatcher` | callback | 空 | 判为心跳则不驱动 ack、默认不应答 | 回调 | P1 |
| `onAckPositive` | callback | 空 | 正应答额外钩子（日志/统计） | 回调 | 其后 |
| `onAckNegative` | callback | 空 | 负应答额外钩子 | 回调 | 其后 |
| `onAckTimeoutDrop` | callback | 空 | 超次丢弃时钩子 | 回调 | 其后 |

未设 matcher 却 `ackEnabled=true`：应启动失败或退化为透传并告警（实现时二选一，建议**拒绝启用**）。

---

## 5. 接收与拆帧

| 参数 | 类型（示意） | 核心默认 | 含义 | 层 | 版本 |
|------|--------------|----------|------|----|------|
| `frameMode` | enum | `Raw` | `Raw` / `LengthPrefix` / `Custom` / （可选）`PesAscii` | 核心 | P0=`Raw`；P1 扩展 |
| `lengthPrefixOffset` | int | `0` | 长度字段起始偏移 | 核心 | P1 |
| `lengthPrefixSize` | int | `6` | 长度字段字节数（PES 为 6） | 核心 | P1 |
| `lengthPrefixEncoding` | enum | `DecimalAscii` | `DecimalAscii` / `BinaryBE` / `BinaryLE` | 核心 | P1 |
| `lengthIncludesHeader` | bool | `true` | 长度是否含头+体+结束符（PES：整包总长） | 核心 | P1 |
| `frameTerminator` | byte? | `0x0A` | 可选尾字节校验；无则仅靠长度 | 核心 | P1 |
| `customFrameSplitter` | callback | 空 | `frameMode=Custom` 时：缓冲→可选完整帧列表 | 回调 | P1 |
| `maxFrameSize` | int | `8192` 或更大 | 超长则弃帧/断开（防失控） | 核心 | P1 |
| `emitRawAlso` | bool | `false` | 拆帧同时是否仍抛原始 read 数据 | 核心 | 其后 |

---

## 6. 线程与运行（可选暴露）

| 参数 | 类型（示意） | 核心默认 | 含义 | 层 | 版本 |
|------|--------------|----------|------|----|------|
| `workerThreadMode` | enum | `Dedicated` | `Dedicated` 独立线程 / `Caller` 由调用方挪线程 | 核心 | P0 |
| `socketInWorkerThread` | bool | `true` | socket 与发送状态机同线程（避免旧双线程） | 核心 | P0 |

---

## 7. 配置来源（怎么喂参数）

| 方式 | 说明 |
|------|------|
| 代码 `TcpConfig{...}` | 编译期/启动时赋值 |
| ini/json（应用层） | 读文件后填入 `TcpConfig`；**读文件不是模块必选** |
| `PesProfile::recommended()` | 只改开关/拆帧/matcher 建议值；**不**自带心跳字节 |
| 运行时 `setHeartbeatFrame` / `setMatcher` | 热更新帧与判定 |

模块内部建议统一结构体，例如：`TcpClientConfig`（上表字段）+ 可选 `IFrameMatcher` 接口。

---

## 8. 参数总览（按我们能否控制）

| 分类 | 我们控制的 | 不由 TCP 模块控制（外部） |
|------|------------|---------------------------|
| 连谁 | host/port/超时/重连 | — |
| 怎么发 | 队列、水位、间隔、ack 窗口/超时/重试 | 业务何时 enqueue |
| 心跳 | 开关、周期、是否占 ack、是否等回复 | **心跳帧字节**（GMAM 组） |
| 应答 | 开关、超时、重试、窗口 | **A/N/H 判定逻辑**（matcher） |
| 接收 | Raw/拆帧规则、最大帧长、结束符 | 业务体解析 |
| 组报 | — | 全部字段与多段拼接 |

---

## 9. PES Profile 会改动的子集（备忘）

加载 PES 预设时，通常只动这些：

- `heartbeatEnabled=true`，`heartbeatIntervalMs=5000`
- `ackEnabled=true`，`ackTimeoutMs≈2000`，`ackMaxRetry≈5～10`
- `frameMode=PesAscii`（或 LengthPrefix + 上表 PES 数值）
- 挂上 PES 的 `ackPositiveMatcher` / `ackNegativeMatcher` / `heartbeatIgnoreMatcher`
- 应用再设 `heartbeatFrame`（GMAM）

其余保持核心默认即可。

---

## 10. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-10-13 | 初版：可配置参数全表 + 默认 + P0/P1 |
