# module_gmam

按 **schema** 把「字段名 → 字符串」编成定长 **`QByteArray`**。

依赖：Qt Core，C++17。当前至 v0.0.3。

---

## 怎么接入

1. 把本目录放进工程任意位置。  
2. 在 `.pro` 里：`include(相对路径/module_gmam.pri)`。  
3. `#include "DataContentBuilder.h"` / `"GmamTypes.h"` 等。

---

## 怎么用

1. 定一份 `QVector<FieldDetail>` schema（按拼接顺序）。  
2. 准备 `QHash<QString, QString>` values。  
3. `DataContentBuilder builder(schema);`  
4. `builder.build(values, errors)` → 成功得定长字节；失败看 `errors`。  

需要多段（例如「头」和「体」）：各用一套 schema 各 build 一次，**怎么拼由外部决定**。

可选工厂：`makeNumericField` / `makeCharsField` / `makeReservedField`。

---

## 本模块文件

| 文件 | 作用 |
|------|------|
| `module_gmam.pri` | qmake 接入 |
| `GmamTypes.h` | 类型、FieldDetail、resolve、工厂 |
| `GmamError.h` | 错误文案与占位 |
| `FieldCodec.*` | 单字段编码 |
| `DataContentBuilder.*` | 顺序拼一段定长内容 |

不在本模块：`HeaderBuilder` / `composeTelegram`（若工程需要，见与演示 `main.cpp` 同级的文件）。
