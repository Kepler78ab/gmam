# 测试 — P2 本地 UI 联调

> 状态：**已落地**（两个独立工程：客户端 + 真对端）  
> 对应确认项：**C09 / C10 / C12**  
> 目的：验证 `module_tcp`；**不是**生产界面。

---

## 1. 两个工程（开两个项目即可）

| 工程 | 路径 | 角色 | 构建目录（相对 .pro） |
|------|------|------|----------------------|
| **TcpPeerTest** | `src/tcp_peer_test/src/TcpPeerTest.pro` | 真对端 Server（先开） | `tcp_peer_test/build` |
| **TcpUiTest** | `src/tcp_ui_test/src/TcpUiTest.pro` | 客户端（连 `module_tcp`） | `tcp_ui_test/build` |

每个工程自带 `src/`，`.pro` 在 `src/` 内，避免两个项目共用同一 `../build` 冲突。

联调步骤：

1. Qt Creator 打开 `tcp_peer_test/src/TcpPeerTest.pro` → 运行 → **开始监听**（默认 `127.0.0.1:9000`）。  
2. 再打开 `tcp_ui_test/src/TcpUiTest.pro` → 运行 → 同一 IP/端口 → **连接** → **发送**。  
3. 两边彩色日志对照；对端可选 Echo / 回 A / 回 N / 只收不回，也可手动回包。
---

## 2. 范围

| 做 | 不做 |
|----|------|
| 两个独立 `.pro` + Designer `.ui` | 把 UI 编进 `module_tcp` |
| 默认 `127.0.0.1`，端口可改 | 完整 PES 业务 |

---

## 3. 测试用例

| 编号 | 步骤 | 期望 |
|------|------|------|
| T01 | 对端监听，客户端连 | 两边显示已连接/已接入 |
| T02 | 客户端发字符串，对端 Echo | 客户端收到 `echo:…` |
| T03 | 对端选手动发 | 客户端日志有收包 |
| T04 | 断开 / 停对端 | 断开或重连符合配置 |
| T05 | 改端口再联 | 可连新端口 |
| T06 | 客户端开心跳 | 对端周期看到收包 |
| T07 | 客户端开 ack，对端回 A/N | 客户端 `ACK√` / `ACK×` |

---

## 4. 清单

- [x] `tcp_ui_test/src` 客户端（独立 build）  
- [x] `tcp_peer_test/src` 真对端（独立 build）  
- [x] Designer `.ui`  
- [x] 默认 `127.0.0.1`，端口可编辑  

---

## 5. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-10-13 | 初版 P2 |
| 2026-10-13 | 落地客户端 UI |
| 2026-10-13 | 改为双工程：TcpUiTest + TcpPeerTest 真对端 |
