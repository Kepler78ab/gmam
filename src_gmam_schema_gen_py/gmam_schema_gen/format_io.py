# -*- coding: utf-8 -*-
"""读写 / 校验 .gmam.txt"""

from __future__ import annotations

from typing import List, Tuple

from .model import (
    ALIGNS,
    DATA_COLUMNS,
    FIELD_TYPES,
    HEADER_COLUMNS,
    MISSING_POLICIES,
    OVERFLOWS,
    ROUNDINGS,
    SchemaDocument,
)


class SchemaFormatError(Exception):
    def __init__(self, errors: List[str]):
        self.errors = errors
        super().__init__("\n".join(errors))


def encode_char_cell(ch: str) -> str:
    """内部单字符 → 文件单元格。"""
    if ch == "\0":
        return r"\0"
    if ch == " ":
        return r"\s"
    if ch == "\\":
        return r"\\"
    if len(ch) == 1:
        return ch
    raise ValueError(f"非法字符值: {ch!r}")


def decode_char_cell(cell: str) -> str:
    """文件单元格 → 单字符（校验用）；写回时仍用转义串。"""
    s = cell.strip()
    if s == r"\0":
        return "\0"
    if s == r"\s":
        return " "
    if s == r"\\":
        return "\\"
    if len(s) == 1:
        return s
    if s.startswith(r"\x") and len(s) == 4:
        return chr(int(s[2:], 16))
    raise ValueError(f"字符列非法: {cell!r}（请用单字符或 \\0 \\s \\\\）")


def _split_row(line: str) -> List[str]:
    return [c.strip() for c in line.split("|")]


def parse_text(text: str) -> SchemaDocument:
    errors: List[str] = []
    lines = text.splitlines()
    kind = None
    version = 1
    header_cols: List[str] = []
    rows: List[dict] = []
    data_started = False

    for i, raw in enumerate(lines, start=1):
        line = raw.strip()
        if not line:
            continue
        if line.startswith("#"):
            continue
        if "=" in line and not data_started and "|" not in line:
            key, _, val = line.partition("=")
            key, val = key.strip(), val.strip()
            if key == "kind":
                if val not in ("data", "header"):
                    errors.append(f"第 {i} 行：kind 非法（data|header）")
                else:
                    kind = val
            elif key == "version":
                try:
                    version = int(val)
                except ValueError:
                    errors.append(f"第 {i} 行：version 非法")
            continue

        cols = _split_row(line)
        if not header_cols:
            header_cols = cols
            data_started = True
            continue

        if len(cols) != len(header_cols):
            errors.append(
                f"第 {i} 行：列数 {len(cols)} 与表头 {len(header_cols)} 不一致"
            )
            continue
        rows.append(dict(zip(header_cols, cols)))

    if kind is None:
        errors.append("缺少 kind=data|header")
    expected = HEADER_COLUMNS if kind == "header" else DATA_COLUMNS
    if header_cols and header_cols != expected:
        errors.append(
            f"表头不匹配 kind={kind}，期望: {'|'.join(expected)}"
        )

    if errors or kind is None:
        raise SchemaFormatError(errors or ["解析失败"])

    doc = SchemaDocument(kind=kind, version=version, rows=rows)  # type: ignore[arg-type]
    errs, _warns = validate_document_with_warnings(doc)
    if errs:
        raise SchemaFormatError(errs)
    return doc


def validate_document_with_warnings(
    doc: SchemaDocument,
) -> Tuple[List[str], List[str]]:
    """返回 (错误, 警告)。"""
    raw: List[str] = []
    names: List[str] = []
    for idx, row in enumerate(doc.rows):
        line_no = idx + 1
        name = str(row.get("name", "")).strip()
        if not name:
            raw.append(f"第 {line_no} 行：name 不能为空")
        elif name in names:
            raw.append(f"第 {line_no} 行：name 重复 '{name}'")
        else:
            names.append(name)

        t = str(row.get("type", "")).strip()
        if t not in FIELD_TYPES:
            raw.append(f"第 {line_no} 行：type 非法 '{t}'")

        try:
            length = int(str(row.get("length", "")).strip())
            if length <= 0:
                raw.append(f"第 {line_no} 行：length 必须为正整数")
            elif t == "Numeric" and length > 18:
                raw.append(f"第 {line_no} 行：Numeric length={length} > 18（警告）")
        except ValueError:
            raw.append(f"第 {line_no} 行：length 必须是整数")

        try:
            scale = int(str(row.get("scale", "0")).strip())
            if scale < 0:
                raw.append(f"第 {line_no} 行：scale 不能为负")
        except ValueError:
            raw.append(f"第 {line_no} 行：scale 必须是整数")

        if doc.kind == "header":
            try:
                off = int(str(row.get("offset", "")).strip())
                if off < 0:
                    raw.append(f"第 {line_no} 行：offset 不能为负")
            except ValueError:
                raw.append(f"第 {line_no} 行：offset 必须是整数")

        for key, allowed in (
            ("rounding", ROUNDINGS),
            ("overflow", OVERFLOWS),
            ("align", ALIGNS),
            ("missingPolicy", MISSING_POLICIES),
        ):
            v = str(row.get(key, "")).strip()
            if v not in allowed:
                raw.append(f"第 {line_no} 行：{key} 非法 '{v}'")

        for key in ("overflowFill", "padFill", "missingFill"):
            try:
                decode_char_cell(str(row.get(key, "")))
            except ValueError as e:
                raw.append(f"第 {line_no} 行：{key} {e}")

    errs = [e for e in raw if "（警告）" not in e]
    warns = [e for e in raw if "（警告）" in e]
    return errs, warns


def dump_text(doc: SchemaDocument) -> str:
    cols = doc.columns()
    lines = [
        "# GMAM Schema",
        f"version={doc.version}",
        f"kind={doc.kind}",
        "|".join(cols),
    ]
    for row in doc.rows:
        lines.append("|".join(str(row.get(c, "")) for c in cols))
    lines.append("")
    return "\n".join(lines)


def load_file(path: str) -> SchemaDocument:
    with open(path, "r", encoding="utf-8") as f:
        doc = parse_text(f.read())
    doc.path = path
    return doc


def save_file(doc: SchemaDocument, path: str) -> None:
    errs, _ = validate_document_with_warnings(doc)
    if errs:
        raise SchemaFormatError(errs)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(dump_text(doc))
    doc.path = path
