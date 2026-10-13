# 设计 — 默认策略：常见惯例 + 兼容 PES

> 回答：设计时如何在「通用默认」与「兼容本厂/旧 PES 协议」之间取值。  
> 配套：[design_01](./design_01_可配置心跳与应答.md) · [analy_02](./analy_02_拆分功能与旧项目对照.md)

---

## 1. 设计思路（两层）

```text
┌─────────────────────────────────────────┐
│  Profile / 应用配置（PES、其它协议）      │  ← 打开心跳/应答、注入帧、matcher
├─────────────────────────────────────────┤
│  module_tcp 核心默认（通用、偏安全）      │  ← 透传优先，策略默认关或保守
└─────────────────────────────────────────┘
```

| 层 | 职责 |
|----|------|
| **核心默认** | 任何工程 include 后即可当「带重连的 TCP 客户端 + 发送队列」用；不假设电文格式 |
| **PES Profile（可选）** | 一套预设：`ackEnabled`、拆帧、`A`/`N`/`H` 判定、建议超时/重试；**心跳帧仍由 GMAM 组好再注入** |

核心**不**写死 `1450M`、`9999999999`；这些只出现在 Profile 或业务配置里。

---

## 2. 核心默认值（建议写进模块）

面向「常见惯例」、且不绑 PES：

| 参数 | 核心默认 | 理由 |
|------|----------|------|
| `autoReconnect` | `true` | 现场链路常抖，默认恢复 |
| `reconnectIntervalMs` | `3000` | 对齐旧工程重连间隔 |
| `connectTimeoutMs` | `3000` | 常见量级 |
| `sendQueueEnabled` | `true` | 业务线程与 socket 解耦 |
| `heartbeatEnabled` | `false` | 无注入帧时乱发无意义 |
| `heartbeatIntervalMs` | `5000` | 启用后的常用周期（与旧一致） |
| `heartbeatExpectReply` | `false` | 心跳默认不应答 |
| `ackEnabled` | `false` | 默认透传，免误挡发送 |
| `ackTimeoutMs` | `2000` | 启用应答后的常用超时（优于「次数×200ms」糊弄） |
| `ackMaxRetry` | `5` | 介于 3～10；旧约 10，通用略收敛 |
| `ackWindow` | `1` | 停等，简单可靠 |
| `frameMode` | `Raw` | P0 不上复杂拆帧 |
| 心跳与 ack 队列 | 心跳**不占** wait-ack 窗口 | 常见惯例 |

P0 最小可用：**连接 + 重连 + enqueue 发送 + 收 Raw 字节**；心跳/应答代码可先留接口，默认关。

---

## 3. PES 兼容 Profile（建议作为「配置预设」，非核心硬编码）

对接旧 `toPesTcp_maybe` / 现行 22 字节头时，应用侧加载类似：

| 参数 | PES 建议值 | 说明 |
|------|------------|------|
| `heartbeatEnabled` | `true` | 旧工程有周期心跳 |
| `heartbeatIntervalMs` | `5000` | 同旧 |
| `heartbeatFrame` | GMAM 组：`length=23` 头 + `H` + 电文号约定 + `0x0A` | **伪装成心跳电文**，模块只负责定时发 |
| `heartbeatExpectReply` | `false` | 旧注释：心跳不应答 |
| `ackEnabled` | `true` | 数据电等 A/N |
| `ackTimeoutMs` | `2000`～`3000` | 替代旧「200ms×10」 |
| `ackMaxRetry` | `5`～`10` | 现场可调；兼容旧可取 10 |
| `frameMode` | `LengthPrefix` 或 `PesAscii` 预设 | 前 6 字节十进制总长，尾 `0x0A` |
| `ackPositiveMatcher` | 完整帧且类型字节为 `'A'`（且非心跳哨兵） | 回调或预设 |
| `ackNegativeMatcher` | 类型字节 `'N'` | 同上 |
| 心跳忽略 | 电文号为约定哨兵（如 `9999999999`）→ 不驱动 ack | 放在 matcher/拆帧后分发 |

**组报：** Profile 只引用「如何识别」；帧字节一律 GMAM/外部生成后注入。

---

## 4. 状态机（启用策略时怎么跑）

```text
[业务] GMAM 组数据电 → enqueue
         │
         ▼
   ackEnabled?
    │no → 发出后即可出队（或发完出队）
    │yes→ 发出 → 等 matcher
              ├ A → 出队 → 下一条
              ├ N → 出队丢弃 → 错误信号
              └ 超时 → 重发同一帧；超 maxRetry → 丢弃 + 错误信号

[定时] heartbeatEnabled?
         → 取 heartbeatFrame/provider → 直接 send
         → 不进入「当前等待应答」槽
```

收包路径：

```text
socket →（可选拆帧）→ 分发
         ├ 判心跳 → 忽略或信号 heartbeatReceived（默认不应答）
         ├ 判 A/N → 喂给发送状态机（仅 ackEnabled）
         └ 其它 → bytesReceived / frameReceived 给业务
```

---

## 5. API 使用形态（示意）

```text
// 通用
TcpClient::setConfig(coreDefaults);
TcpClient::connect();
TcpClient::enqueue(gmamBuiltFrame);

// 兼容 PES
config = PesProfile::recommended();          // 打开 ack/heartbeat/拆帧建议值
config.heartbeatFrame = buildPesHeartbeat(); // GMAM
config.ackPositiveMatcher = ...;             // 或用 PesProfile 自带 matcher
TcpClient::setConfig(config);
```

---

## 6. 版本切分建议

| 版本 | 范围 |
|------|------|
| **v0.0.1 P0** | 核心默认：连接/重连/队列发送/Raw 接收；配置结构预留心跳/ack 字段但可先不跑通状态机 |
| **v0.0.1 P1** | 落地 `heartbeatEnabled` + 注入帧；`ackEnabled` + 超时重发 + matcher；可选 PES 拆帧预设 |
| **其后** | 多窗口、双向心跳检测、Server 等 |

---

## 7. 决策摘要

| 问题 | 怎么定 |
|------|--------|
| 默认像不像 PES？ | **不像**；核心默认透传 + 重连 |
| PES 怎么兼容？ | **Profile + 注入帧 + matcher**，不写进核心常量 |
| 心跳是不是电文？ | **可以是**；模块当「定时发送的 QByteArray」 |
| 应答默认开吗？ | **关**；PES Profile 再开 |
| 和 GMAM？ | 只收成品字节；head/data/心跳帧都在外组 |

---

## 8. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-10-13 | 两层默认：核心通用 + PES Profile；P0/P1 切分 |
