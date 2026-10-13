# module_tcp

可配置 TCP **客户端**：发/收已组好的 `QByteArray`（组报不在本模块）。

依赖：`QT += core network`，C++17。当前 **P0+P1**。

---

## 接入

```pro
include($$PWD/相对路径/module_tcp/module_tcp.pri)
```

```cpp
#include "TcpClient.h"
#include "TcpClientConfig.h"
// 可选 PES 辅助：
#include "PesFrameHints.h"
```

---

## P0：透传

1. 填 `TcpClientConfig`（默认 host=`127.0.0.1`）。  
2. `setConfig` → 连信号 → `connectToHost`。  
3. `enqueue(frame)`；连上后自动冲刷队列。  
4. 收包看 `bytesReceived`（Raw 下 `frameReceived` 同步有一份）。

## P1：心跳 / 应答 / 拆帧

| 能力 | 用法 |
|------|------|
| 心跳 | `cfg.heartbeatEnabled=true`；`setHeartbeatFrame(...)` 或 `setHeartbeatFrameProvider` |
| wait-ack | `cfg.ackEnabled=true`；**必须** `setAckMatchers(pos, neg[, hbIgnore])`，否则自动关 ack 并 `errorOccurred` |
| 拆帧 | `frameMode=LengthPrefix` 或 `PesAscii`；完整帧走 `frameReceived` |
| 队列 | `DropOldest` / `highWaterMark` / `lowWaterMark` / `sendIntervalMs` |

`ackMaxRetry` = **含首次在内的最大发送次数**。心跳默认**不占** ack 窗口。

PES 快捷：

```cpp
cfg = tcp_pes::recommendedRuntimeFlags(cfg);
client.setAckMatchers(tcp_pes::matcherPositiveA(),
                      tcp_pes::matcherNegativeN(),
                      tcp_pes::matcherHeartbeatIgnore());
client.setHeartbeatFrame(/* GMAM 组好的 H 帧 */);
```

---

## 文件

| 文件 | 作用 |
|------|------|
| `TcpClient.*` | 门面 |
| `TcpClientWorker.*` | 工作线程 |
| `SendQueue.*` | 发送队列 |
| `FrameAssembler.*` | 拆帧 |
| `IFrameMatcher.h` | matcher / splitter 类型 |
| `profiles/PesFrameHints.h` | 可选 PES 预设 |
| `TcpClientConfig.h` / `TcpTypes.h` | 配置与枚举 |

P2 联调（开两个工程；各自 `../build` 不冲突）：
1. `src/tcp_peer_test/src/TcpPeerTest.pro` — 真对端，先监听  
2. `src/tcp_ui_test/src/TcpUiTest.pro` — 客户端  

说明见 `docs/v0.0.1/测试_P2_本地UI联调.md`。
