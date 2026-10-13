# analy_01 — 旧方案 TCP 模块分析

> 来源工程：仓库根目录 `toPesTcp_maybe/`（工程名 `1450MsgToPES`）  
> 目标：为 `src_tcp_qt/src/module_tcp`（pri + 文件夹，对齐 `module_gmam`）抽取可复用通讯核心提供依据。  
> 本文只做**现状分析**，不涉及新模块接口定稿。

---

## 1. 工程定位

| 项 | 内容 |
|----|------|
| 方向 | 本机为 **TCP 客户端**，对端为 MES/PES 服务端（注释：`mes为服务端，其他为客户端`） |
| 业务 | 1450 轧辊相关电文上送（换辊 / 磨削 / 明细等） |
| 依赖 | `QT += network`（及业务侧 gui/sql/widgets，**非 TCP 核心**） |
| Server | `TcpServerWorker` **整文件注释**，运行路径不使用 |

业务落库（`DBManager`）、UI（`Widget`）、具体电文结构体（`PublicData.h` 业务体）属于应用层，**不应进入 module_tcp**。

---

## 2. 源码分层（旧工程）

```text
Widget / ConnectToDB（业务）
        │ appendSendQueue(QByteArray, msgType)
        ▼
SendThreadManager（单例）── QThread
        │
        ▼
WorkerSendThread ── 发送队列 / 心跳 / 应答状态（粗糙）
        │ invokeMethod(slotSendData)
        ▼
TcpClientWorker（再套一层 QThread）── QTcpSocket
        │
        ▼
远端 ServerIP:ServerPort
```

| 文件 | 角色 | 是否适合进 module_tcp |
|------|------|------------------------|
| `TcpClientWorker.*` | 连接、收发、断线 3s 重连 | **是（核心）** |
| `WorkerSendThread.*` | 队列、定时发送、心跳、应答处理 | **部分**（策略层，需理顺） |
| `SendThreadManager.*` | 单例挂线程 | 可选 / 可简化 |
| `TcpConfigReader.*` | 读 `tcp_config.ini` | 配置形态可保留，路径可抽象 |
| `TcpServerWorker.*` | 已废弃注释 | **否** |
| `PublicData.h` | 协议头 + 业务结构体 + 心跳/应答帧 | **协议约定进文档**；业务结构体 **否** |
| `Widget` / `DBManager` | UI、MySQL | **否** |

---

## 3. 链路与配置

### 3.1 配置（`TcpConfigReader`）

- 路径：`applicationDirPath()/tcp_config.ini`
- 分组：`[TCP_PARAMS]`

| 键 | 默认（代码内） | 实际使用情况 |
|----|----------------|--------------|
| `ServerIP` | `192.168.19.64` | 连接用 |
| `ServerPort` | `20000` | 连接用 |
| `ConnectTimeout` | `3000` | **读了，连接逻辑几乎未用** |
| `Heart` | `5000` | 心跳定时器间隔（ms） |
| `AutoReconnect` | `true` | **读了，重连实际写死开启** |
| `MsgType1/2/3` | 电文号字符串 | 业务侧标记用，非 socket 层 |

### 3.2 连接行为（`TcpClientWorker`）

- `QTcpSocket::connectToHost(ip, port)`
- 信号：`connected` / `disconnected` / `readyRead` / `error`
- 断线或错误后：`QTimer` **固定 3000ms** 调 `connectToHost` 重连
- 发送：`write` + `flush`；收到：`readAll()` 整段抛出（**无粘包拆帧**）

---

## 4. 应用层协议要点（与 TCP 字节流相关）

详见此前对 `PublicData.h` 的扒取；此处只记与通讯模块相关的部分。

### 4.1 现行外部头（`STRU_MSG_HEAD`，22 字节）+ 结束符

```text
[ m_msgLength 6 ][ m_lineNum 5 ][ m_msgNum 10 ][ m_msgType 1 ] + [ 业务体? ] + [ 0x0A ]
```

| `m_msgType` | 含义 |
|-------------|------|
| `D` | 数据电文（有业务体） |
| `A` | 正应答 |
| `N` | 负应答 |
| `H` | 心跳 |

- 定长 ASCII，`#pragma pack(1)`；空位先空格再左贴有效字符。  
- `m_msgLength`：整包总长（含头+体+结束符）的十进制字符串。  
- 心跳现行：`MsgHeartNew`，总长写 `"23"`，电文号 `9999999999`，类型 `H`。  
- **旧头 29 字节**（`msgNum[15]+lineNum[7]`）仍留在头文件中，现行收发以 **22 字节头**为准。

### 4.2 注释中的应答规则（发送节奏）

1. 数据电文初步检查只看电文号是否已定义。  
2. 收到 **正应答 A** → 可发下一条。  
3. 收到 **负应答 N** → 丢弃当前并报错。  
4. 指定时间未应答 → 重发。  
5. 重发过多 → 丢弃当前。  
6. **心跳、应答本身不再回 A/N**。  
7. 心跳约每 5s（配置 `Heart`）。

### 4.3 与 GMAM 的边界

- TCP 模块输入应为 **已拼好的 `QByteArray`**。  
- 头/体可用两套 GMAM schema（或外部拼接）产出；**组报不进 module_tcp**。

---

## 5. 发送 / 应答状态机（旧实现行为）

| 机制 | 实现要点 |
|------|----------|
| 入队 | `appendSendQueue(data, strType)`，`QMutex` + 双队列（字节 / 类型） |
| 发送节奏 | `QTimer` **200ms** 扫队列，对队首 `realSendData` |
| 重试计数 | `m_num` 初始 10，每发一次 `--`；`<0` 则丢弃队首，状态码 3 |
| 正应答 A | 仅 `m_sendTypeQueue.dequeue()`，发状态 1 |
| 负应答 N | 仅 `m_sendTypeQueue.dequeue()`，发状态 2 |
| 心跳 | 独立定时器组 `MsgHeartNew` 直接发，不走业务队列 |
| 收包判别 | `length < 23` 丢弃；`==23` 当头-only；电文号含 `9999999999` 当心跳忽略；`A`/`N` 分支；更长当数据电「暂不处理」 |

### 5.1 已知缺陷（拆模块时需正视）

1. **A/N 未稳定出队 `m_sendQueue`**，与注释「正应答后发下一条」不一致，易重复发送或队列错乱。  
2. **无按长度字段拆帧**，`readAll()` 粘包时会解析失败。  
3. **双层线程**（Send 线程 + Client 线程）偏重，可收束。  
4. `ConnectTimeout` / `AutoReconnect` 配置未真正驱动逻辑。  
5. 应答超时未用独立计时，而是靠「200ms × 约 10 次」近似。  
6. Server 代码废弃仍挂在工程里，易误导。

---

## 6. 对外交互（业务侧怎么用旧代码）

```text
启动：SendThreadManager::instance()->startSendThread()
入队：getWorker()->appendSendQueue(整包字节, 电文类型字符串)
信号：MsgFull(bool)           — 队列过长/回落
      signalSendStatus(int)   — 1=A, 2=N, 3=重试耗尽丢弃
```

业务把 `MsgHGXX` 等结构体 `reinterpret_cast` 成 `QByteArray` 入队——新架构应改为 **GMAM（或外部）组报后再入队**。

---

## 7. 建议拆入 `module_tcp` 的边界（计划口径，非定稿）

目标路径（与 `module_gmam` 对齐）：

```text
src_tcp_qt/
├── docs/
│   └── v0.0.1/
│       └── analy_01_旧方案tcp模块分析.md   ← 本文
└── src/
    └── module_tcp/
        └── module_tcp.pri
```

| 宜进模块（通讯） | 不宜进模块（业务） |
|------------------|-------------------|
| TCP 客户端连接 / 断开 / 重连 | `PublicData` 业务结构体 |
| 发送队列（线程安全 enqueue） | Widget / MySQL |
| 可选：心跳周期回调或注入心跳帧 | 具体电文号业务含义 |
| 可选：wait-ack / 重试策略（理顺后） | 29/22 头字段拼装（归 GMAM/外部） |
| 收包信号（原始字节；可选拆帧） | Server（本期不做） |
| 配置结构（IP/端口/心跳/重试） | — |

**与 GMAM 关系：**  
`module_gmam` = schema → 定长段；外部拼 `head+data+0x0A`；`module_tcp` = 发/收/队列/链路策略。

---

## 8. 后续文档建议（v0.0.1）

| 文档 | 内容 |
|------|------|
| 本文 `analy_01` | 旧方案事实与缺陷 |
| （待写）模块边界 / 计划清单 | P0 接口：连上、enqueue、收原始字节、重连 |
| （待写）协议附录 | 22 字节头 + A/N/H/D + 应答规则（供业务与拆帧） |

---

## 9. 结论摘要

1. 旧工程 **有完整 TCP 客户端路径**，可作 `module_tcp` 来源。  
2. 可复用价值集中在：**Socket 生命周期、重连、发送队列、心跳节拍、（理顺后的）应答策略**。  
3. 协议头/体组报应继续放在 **GMAM + 外部拼接**，不要塞进 TCP 模块。  
4. 落地前必须修：**应答出队、拆帧、线程模型、配置项落地**；P0 可先做「连接 + 队列发送 + 收原始字节 + 重连」，应答/心跳策略分档实现。
