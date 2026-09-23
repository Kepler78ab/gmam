# -*- coding: utf-8 -*-
"""行编辑对话框。"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk, messagebox
from typing import Dict, Optional

from ..model import (
    ALIGNS,
    FIELD_TYPES,
    MISSING_POLICIES,
    OVERFLOWS,
    ROUNDINGS,
    Kind,
)
from ..format_io import decode_char_cell


class RowEditDialog(tk.Toplevel):
    def __init__(self, master, kind: Kind, row: Optional[dict] = None, title: str = "编辑字段"):
        super().__init__(master)
        self.title(title)
        self.resizable(False, False)
        self.result: Optional[dict] = None
        self.kind = kind
        self.transient(master)
        self.grab_set()

        frm = ttk.Frame(self, padding=10)
        frm.grid(row=0, column=0, sticky="nsew")

        self.vars: Dict[str, tk.Variable] = {}
        fields = [
            ("name", "name", None),
            ("type", "type", FIELD_TYPES),
        ]
        if kind == "header":
            fields.append(("offset", "offset", None))
        fields.extend(
            [
                ("length", "length", None),
                ("scale", "scale", None),
                ("rounding", "rounding", ROUNDINGS),
                ("overflow", "overflow", OVERFLOWS),
                ("overflowFill", "overflowFill (单字符或 \\0 \\s)", None),
                ("align", "align", ALIGNS),
                ("padFill", "padFill", None),
                ("missingFill", "missingFill", None),
                ("missingPolicy", "missingPolicy", MISSING_POLICIES),
            ]
        )

        src = row or {}
        for i, (key, label, choices) in enumerate(fields):
            ttk.Label(frm, text=label).grid(row=i, column=0, sticky="w", pady=2)
            if choices:
                var = tk.StringVar(value=str(src.get(key, choices[0])))
                cb = ttk.Combobox(frm, textvariable=var, values=choices, state="readonly", width=28)
                cb.grid(row=i, column=1, sticky="ew", pady=2)
            else:
                default = src.get(key, "")
                if key in ("length", "scale", "offset") and default == "":
                    default = "0" if key != "length" else "1"
                if key in ("padFill", "missingFill", "overflowFill") and default == "":
                    default = r"\0" if key != "overflowFill" else "9"
                var = tk.StringVar(value=str(default))
                ttk.Entry(frm, textvariable=var, width=30).grid(row=i, column=1, sticky="ew", pady=2)
            self.vars[key] = var

        btns = ttk.Frame(frm)
        btns.grid(row=len(fields), column=0, columnspan=2, pady=8)
        ttk.Button(btns, text="确定", command=self._ok).pack(side=tk.LEFT, padx=4)
        ttk.Button(btns, text="取消", command=self.destroy).pack(side=tk.LEFT, padx=4)

        self.bind("<Return>", lambda e: self._ok())
        self.wait_window(self)

    def _ok(self):
        data = {k: v.get().strip() for k, v in self.vars.items()}
        if not data.get("name"):
            messagebox.showerror("校验", "name 不能为空", parent=self)
            return
        try:
            int(data["length"])
            int(data["scale"])
            if self.kind == "header":
                int(data["offset"])
            for k in ("overflowFill", "padFill", "missingFill"):
                decode_char_cell(data[k])
        except ValueError as e:
            messagebox.showerror("校验", str(e), parent=self)
            return
        self.result = data
        self.destroy()
