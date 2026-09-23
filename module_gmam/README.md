# module_gmam

通用报文组装：用 schema 描述定长字段，把「字段名 → 字符串」编成 `QByteArray`。

依赖：Qt Core，C++17。当前至 v0.0.3。

---

## 怎么接入

1. 把本目录放进工程任意位置。
2. 在 `.pro` 里：`include(相对路径/module_gmam.pri)`。
3. 需要时 `#include` 本目录头文件（`INCLUDEPATH` 已由 `.pri` 加上）。

---

## 怎么用

1. **定 schema**  
   - 数据段：`QVector<FieldDetail>`，按拼接顺序排列。  
   - 头部：`QVector<HeaderField>`，多一个 `offset`；类型用 `HeaderFieldType`。  
   - 也可用 `makeNumericField` / `makeCharsField` / `makeReservedField`（及 Header 对应工厂）。

2. **准备值**  
   - `QHash<QString, QString>`，key 与字段 `name` 一致。  
   - CRC、长度等由你算好后当普通字段写入即可。

3. **编数据段**  
   - `DataContentBuilder builder(schema);`（可选第二参 `maxLength`，默认 8162）  
   - `builder.build(values, errors)` → 成功得定长字节；失败得空数组，看 `errors`。

4. **编头部（可选）**  
   - `HeaderBuilder builder(schema, 头部总长度, 空洞填充字符);`  
   - 同样 `build(values, errors)`。未覆盖区间用空洞字符填。

5. **拼整包（可选）**  
   - `composeTelegram(head, data)`，默认末尾加 `0x0A`；第三参可改结束符。  
   - 多层 / 多套 schema：各自 `build` 后再自行 `append`。

6. **看结果**  
   - `errors` 为空才可用返回的 `QByteArray`。  
   - 字段级错误会尽量收齐；失败字段先占位再继续扫，最终仍因有错返回空 buffer。  
   - `totalLength()` 可查定长总字节数。

---

## schema 里要关心什么

| 项 | 说明 |
|----|------|
| `name` | 与 values 的 key 对应 |
| `type` | Numeric / Chars / Reserved |
| `length` | 该字段固定字节数 |
| `scale` / `rounding` | 仅 Numeric：小数放大与舍入 |
| `overflow` | 超长：默认 Error；可 Truncate / FillChar |
| `overflowFill` | 仅 FillChar 时用 |
| `align` | 默认 ByType（N 右、C 左）；可 Left / Right |
| `padFill` / `missingFill` | `'\0'` 表示按类型默认（N→`'0'`，C→空格） |
| `missingPolicy` | 空串算不算缺值 |

`HeaderField` 在 `type` 后多 `offset`，编码规则与数据段相同。

---

## 文件

| 文件 | 作用 |
|------|------|
| `module_gmam.pri` | qmake 接入 |
| `GmamTypes.h` | 类型、FieldDetail / HeaderField、工厂 |
| `GmamError.h` | 错误文案与占位 |
| `FieldCodec.*` | 单字段编码 |
| `DataContentBuilder.*` | 顺序拼数据段 |
| `HeaderBuilder.*` | 按 offset 拼头部 |
| `TelegramCompose.*` | head + data + 结束符 |
