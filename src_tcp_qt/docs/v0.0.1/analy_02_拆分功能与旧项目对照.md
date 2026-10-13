# 拆分后 TCP 功能 ↔ 旧项目对照表

> 旧工程：`toPesTcp_maybe`（`1450MsgToPES`）  
> 新模块计划：`src_tcp_qt/src/module_tcp`  
> 组报：改由 `module_gmam` 多段 schema → 外部拼接；**不进 TCP**。  
> 配套分析：[analy_01_旧方案tcp模块分析.md](./analy_01_旧方案tcp模块分析.md)

---

## 1. 总览

| 类别 | 旧项目 | 拆分后 |
|------|--------|--------|
| 组报（头/体/结束符） | `PublicData` 结构体 `memcpy` 整包 | **GMAM + 外部拼接**（非 TCP） |
| 链路收发 | `TcpClientWorker` + `WorkerSendThread` | **module_tcp** |
| 业务 / DB / UI | `Widget`、`DBManager` | 应用层，不进模块 |

---

## 2. 功能对照表

| # | 拆分后 TCP 功能 | 旧项目对应 | 旧实现要点（简略） | 拆分后归属 |
|---|-----------------|------------|-------------------|------------|
| 1 | 连接远端（IP/端口） | `TcpClientWorker::slotConnectToServer` | `QTcpSocket::connectToHost`；配置来自 `tcp_config.ini` | module_tcp |
| 2 | 断开连接 | `slotDisConnect` | `disconnectFromHost` + 短时 `waitForDisconnected` | module_tcp |
| 3 | 连接/断开/错误状态通知 | `sigConnectResult` / `sigDisconnected` / `sigError` | Socket 信号转发出；错误码转中文文案 | module_tcp |
| 4 | 自动重连 | `onReconnect` + 定时器 | 断线/错误后 **固定 3s** 再 `connectToHost`；配置项 `AutoReconnect` 读了未真正分支 | module_tcp（建议配置化） |
| 5 | 发送已组好的字节 | `slotSendData` ← `realSendData` | `write` + `flush`；入参已是整包 `QByteArray` | module_tcp |
| 6 | 发送队列（异步入队） | `WorkerSendThread::appendSendQueue` | `QMutex` + `QQueue`；附带并行的 `msgType` 队列 | module_tcp |
| 7 | 定时扫队发送 | `onSendTimer` | **200ms** 定时器；每次对**队首**再发一次（非真正「等 A 再出队」） | module_tcp（策略可配置） |
| 8 | 发送失败/队列满告警 | `MsgFull` | 队列 `>100` 告满，`<50` 回落 | module_tcp（可选） |
| 9 | 接收原始字节并上抛 | `sigRecvData` → `RecvMsg` | **`readAll()` 一次抛出**，无缓冲拼包 | module_tcp（P0：原始字节） |
| 10 | 收包拆帧 / 粘包处理 | （无） | **未实现**；靠「长度==23」猜头-only 帧 | module_tcp（建议增强：按长度头+`0x0A`） |
| 11 | 识别 A/N/H/D | `RecvMsg` | `len<23` 丢弃；`==23` 当头帧；电文号含 `9999999999` 当心跳忽略；看 `m_msgType` | 策略可进 TCP；字段定义属协议约定 |
| 12 | 正应答后发下一条 | 注释规则 + `RecvAnswer_A` | **只出队 `m_sendTypeQueue`，未稳定出队数据队列** → 与规则不符 | module_tcp（需按规则重做） |
| 13 | 负应答丢弃当前 | `RecvAnswer_N` | 同样只动类型队列；发 `signalSendStatus(2)` | module_tcp（需重做） |
| 14 | 超时重发 | `m_num` 递减近似 | 约 **10 次 × 200ms**，无独立应答超时定时器 | module_tcp（建议：超时 ms + 最大次数） |
| 15 | 重发耗尽丢弃 | `m_num<0` 分支 | 出队并 `signalSendStatus(3)` | module_tcp |
| 16 | 心跳周期触发 | `SendHeartTimer` | 配置 `Heart`（默认 5s）定时发 | module_tcp：只负责**节拍**；或 `enqueue` 外部组好的心跳帧 |
| 17 | 心跳帧内容 | `MsgHeartNew` 构造 | 结构体内写死 length/line/msgNum/`H`/`0x0A` | **GMAM/外部组报**，TCP 不拼字段 |
| 18 | 数据电文内容 | `MsgHGXX` 等 | 结构体填字段后整包入队 | **GMAM 多段拼接** |
| 19 | 读 ini 配置 | `TcpConfigReader` | 单例；缺文件用代码默认值 | module_tcp 可收「配置结构」；路径由应用定 |
| 20 | 线程模型 | 双线程 | `SendThreadManager` 一线程 + `TcpClientWorker` 再一线程 | module_tcp 建议收束（一连接一线程或 socket 跟发送线程） |
| 21 | TCP Server | `TcpServerWorker`（注释） | 未启用 | **本期不做** |
| 22 | UI / DB | `Widget` / `DBManager` | 查库组结构体再入队 | **应用层** |

---

## 3. 接收策略对照（单独展开）

| 步骤 | 旧实现 | 拆分后建议 |
|------|--------|------------|
| 读 socket | `readyRead` → `readAll()` | 同左，先累进接收缓冲 |
| 粘包/半包 | 无；整段当一帧猜 | 按协议：读 6 字节长度 → 凑满整包（含 `0x0A`）再出帧 |
| 过短 | `<23` 直接 return | 缓冲等待或丢弃非法帧（可配置） |
| 头-only（A/N/H） | `length==23` + 解析 `STRU_MSG_HEAD` | 拆帧后看 `m_msgType` |
| 心跳 | `msgNum` 含 `9999999999` → 忽略、不应答 | 可回调/信号「收到心跳」；默认不应答 |
| 正/负应答 | 调 `RecvAnswer_A/N`（出队不完整） | 驱动发送状态机：确认/丢弃**当前发送中的那一帧** |
| 数据电 D | `length!=23` →「暂不处理」 | 上抛完整帧给业务；TCP 不解析业务体 |
| 应答是否回包 | 注释：心跳/应答不再回 A/N | 保持该规则 |

---

## 4. 发送策略对照（单独展开）

| 步骤 | 旧实现 | 拆分后建议 |
|------|--------|------------|
| 入队 | 业务 `appendSendQueue(整包, type)` | 业务先 GMAM 组报，再 `enqueue(QByteArray)` |
| 是否等 ACK | 意图等 A；实际定时反复发队首 | P0 可「只发不等」；P1 正式 wait-ack |
| 重发 | 计数耗尽丢弃 | 独立超时 + maxRetry |
| 心跳 | 旁路直接 `write`，不占业务队列 | 旁路或低优先级；内容外部注入 |
| 组报 | 旧结构体 | **不在 TCP** |

---

## 5. 边界一句话

| 模块 | 管什么 |
|------|--------|
| **module_gmam** | schema → 定长字节段 |
| **外部应用** | 多段拼接、算 `m_msgLength`、业务与 UI/DB |
| **module_tcp** | 连接/重连、队列发送、接收（+ 可选拆帧与 A/N/心跳节拍） |

---

## 6. 设计补充（已采纳）

心跳、正负应答保留在 **module_tcp**，但以**参数开关**启用；心跳/判定规则的**帧内容外置**（可注入协议特有「伪装心跳」电文）。详见 [design_01_可配置心跳与应答.md](./design_01_可配置心跳与应答.md)。

---

## 7. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-10-13 | 初版：对照表 + 收/发策略简列 |
| 2026-10-13 | 链到 design_01（可配置心跳/应答） |
