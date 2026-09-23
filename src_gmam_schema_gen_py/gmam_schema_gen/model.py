# -*- coding: utf-8 -*-
"""Schema 文档与字段行数据模型。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Literal, Optional

Kind = Literal["data", "header"]

FIELD_TYPES = ("Numeric", "Chars", "Reserved")
ROUNDINGS = ("Truncate", "Round")
OVERFLOWS = ("Error", "Truncate", "FillChar")
ALIGNS = ("ByType", "Left", "Right")
MISSING_POLICIES = ("TreatAsMissing", "EmptyIsValue")

DATA_COLUMNS = [
    "name",
    "type",
    "length",
    "scale",
    "rounding",
    "overflow",
    "overflowFill",
    "align",
    "padFill",
    "missingFill",
    "missingPolicy",
]

HEADER_COLUMNS = [
    "name",
    "type",
    "offset",
    "length",
    "scale",
    "rounding",
    "overflow",
    "overflowFill",
    "align",
    "padFill",
    "missingFill",
    "missingPolicy",
]


def default_row(kind: Kind = "data") -> dict:
    row = {
        "name": "",
        "type": "Chars",
        "length": "1",
        "scale": "0",
        "rounding": "Truncate",
        "overflow": "Error",
        "overflowFill": "9",
        "align": "ByType",
        "padFill": r"\0",
        "missingFill": r"\0",
        "missingPolicy": "TreatAsMissing",
    }
    if kind == "header":
        row["offset"] = "0"
    return row


@dataclass
class SchemaDocument:
    kind: Kind = "data"
    version: int = 1
    rows: List[dict] = field(default_factory=list)
    path: Optional[str] = None

    def columns(self) -> List[str]:
        return HEADER_COLUMNS if self.kind == "header" else DATA_COLUMNS

    def estimated_total_length(self) -> int:
        total = 0
        for r in self.rows:
            try:
                n = int(str(r.get("length", "0")).strip())
                if n > 0:
                    total += n
            except ValueError:
                pass
        return total
