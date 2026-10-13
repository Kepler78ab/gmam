# src_tcp_qt

TCP 通讯模块工程目录（对齐 `src_gmam_qt`：`docs` + `src/module_*/` + `.pri`）。

| 路径 | 说明 |
|------|------|
| [docs/v0.0.1/analy_01_旧方案tcp模块分析.md](docs/v0.0.1/analy_01_旧方案tcp模块分析.md) | 旧工程 `toPesTcp_maybe` 分析 |
| [docs/v0.0.1/analy_02_拆分功能与旧项目对照.md](docs/v0.0.1/analy_02_拆分功能与旧项目对照.md) | 拆分后功能 ↔ 旧实现对照（含收发策略） |
| [docs/v0.0.1/design_01_可配置心跳与应答.md](docs/v0.0.1/design_01_可配置心跳与应答.md) | 心跳/应答可配置启用，帧内容外置 |
| [docs/v0.0.1/design_02_默认策略与PES兼容.md](docs/v0.0.1/design_02_默认策略与PES兼容.md) | 核心默认 + PES Profile 怎么定 |
| [docs/v0.0.1/design_03_可配置参数清单.md](docs/v0.0.1/design_03_可配置参数清单.md) | 全部可配置参数与默认/版本 |
| [docs/v0.0.1/计划_实现方案与计划.md](docs/v0.0.1/计划_实现方案与计划.md) | **实现方案与计划（待审查，C01～C12）** |
| [docs/v0.0.1/模块文件规划.md](docs/v0.0.1/模块文件规划.md) | 源文件清单 |
| [docs/v0.0.1/测试_P2_本地UI联调.md](docs/v0.0.1/测试_P2_本地UI联调.md) | P2：127.0.0.1 + Designer `.ui` 联调 |
| `src/module_tcp/` | **P0+P1 已实现**（透传 + 心跳/ack/拆帧） |
| `src/tcp_peer_test/src/TcpPeerTest.pro` | **P2** 真对端（先开；build 在 `tcp_peer_test/build`） |
| `src/tcp_ui_test/src/TcpUiTest.pro` | **P2** 客户端（build 在 `tcp_ui_test/build`） |
| [docs/v0.0.1/变更记录.md](docs/v0.0.1/变更记录.md) | 变更记录 |
| `src/md_tcp.pro` | P0 工程入口（含本机回环冒烟 `main.cpp`） |

旧代码来源：仓库根目录 `toPesTcp_maybe/`。
