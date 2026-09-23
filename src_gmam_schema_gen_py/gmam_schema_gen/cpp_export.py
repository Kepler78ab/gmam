# -*- coding: utf-8 -*-
"""导出 C++ QVector 初始化片段。"""

from __future__ import annotations

from .format_io import decode_char_cell
from .model import SchemaDocument


def _cpp_char(cell: str) -> str:
    ch = decode_char_cell(cell)
    if ch == "\0":
        return r"'\0'"
    if ch == "'":
        return r"'\''"
    if ch == "\\":
        return r"'\\'"
    if 32 <= ord(ch) < 127:
        return f"'{ch}'"
    return f"'\\x{ord(ch):02x}'"


def _qstring(name: str) -> str:
    escaped = name.replace("\\", "\\\\").replace('"', '\\"')
    return f'QStringLiteral("{escaped}")'


def export_cpp(doc: SchemaDocument) -> str:
    if doc.kind == "header":
        return _export_header(doc)
    return _export_data(doc)


def _export_data(doc: SchemaDocument) -> str:
    lines = ["QVector<FieldDetail> schema = {"]
    for row in doc.rows:
        lines.append(
            "    {{ {}, FieldType::{}, {}, {}, Rounding::{},\n"
            "      OverflowPolicy::{}, {},\n"
            "      FieldAlign::{}, {}, {}, MissingPolicy::{} }},".format(
                _qstring(str(row["name"]).strip()),
                row["type"],
                int(row["length"]),
                int(row["scale"]),
                row["rounding"],
                row["overflow"],
                _cpp_char(str(row["overflowFill"])),
                row["align"],
                _cpp_char(str(row["padFill"])),
                _cpp_char(str(row["missingFill"])),
                row["missingPolicy"],
            )
        )
    lines.append("};")
    return "\n".join(lines)


def _export_header(doc: SchemaDocument) -> str:
    lines = ["QVector<HeaderField> schema = {"]
    for row in doc.rows:
        lines.append(
            "    {{ {}, HeaderFieldType::{}, {}, {}, {}, Rounding::{},\n"
            "      OverflowPolicy::{}, {},\n"
            "      FieldAlign::{}, {}, {}, MissingPolicy::{} }},".format(
                _qstring(str(row["name"]).strip()),
                row["type"],
                int(row["offset"]),
                int(row["length"]),
                int(row["scale"]),
                row["rounding"],
                row["overflow"],
                _cpp_char(str(row["overflowFill"])),
                row["align"],
                _cpp_char(str(row["padFill"])),
                _cpp_char(str(row["missingFill"])),
                row["missingPolicy"],
            )
        )
    lines.append("};")
    return "\n".join(lines)
