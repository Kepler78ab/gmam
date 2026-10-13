# src_tcp_qt

TCP 通讯模块工程目录（对齐 `src_gmam_qt`：`docs` + `src/module_*/` + `.pri`）。

| 路径 | 说明 |
|------|------|
| **[docs/使用说明.md](使用说明.md)** | **module_tcp 调用说明（API / 参数 / matcher）** |
| **[docs/v0.0.2/](v0.0.2/README.md)** | **v0.0.2 计划包（源码注释 / 古法编程，待实现）** |
| [docs/v0.0.1/analy_01_旧方案tcp模块分析.md](v0.0.1/analy_01_旧方案tcp模块分析.md) | 旧工程 `toPesTcp_maybe` 分析 |
| [docs/v0.0.1/analy_02_拆分功能与旧项目对照.md](v0.0.1/analy_02_拆分功能与旧项目对照.md) | 拆分后功能 ↔ 旧实现对照（含收发策略） |
| [docs/v0.0.1/design_01_可配置心跳与应答.md](v0.0.1/design_01_可配置心跳与应答.md) | 心跳/应答可配置启用，帧内容外置 |
| [docs/v0.0.1/design_02_默认策略与PES兼容.md](v0.0.1/design_02_默认策略与PES兼容.md) | 核心默认 + PES Profile 怎么定 |
| [docs/v0.0.1/design_03_可配置参数清单.md](v0.0.1/design_03_可配置参数清单.md) | 全部可配置参数与默认/版本 |
| [docs/v0.0.1/计划_实现方案与计划.md](v0.0.1/计划_实现方案与计划.md) | 实现方案与计划（C01～C12） |
| [docs/v0.0.1/模块文件规划.md](v0.0.1/模块文件规划.md) | 源文件清单 |
| [docs/v0.0.1/测试_P2_本地UI联调.md](v0.0.1/测试_P2_本地UI联调.md) | P2：双工程联调 |
| `src/module_tcp/` | **v0.0.1 功能已实现**；v0.0.2 注释待做 |
| `src/tcp_peer_test/src/TcpPeerTest.pro` | **P2** 真对端 |
| `src/tcp_ui_test/src/TcpUiTest.pro` | **P2** 客户端 |
| [docs/v0.0.1/变更记录.md](v0.0.1/变更记录.md) | 变更记录 |
| `src/md_tcp.pro` | 控制台冒烟 |

旧代码来源：仓库根目录 `toPesTcp_maybe/`。
