# -*- coding: utf-8 -*-
"""行内可编辑表格：统一 grid 对齐列；枚举列为双语下拉。"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk
from typing import Callable, Dict, List, Optional

from ..i18n import I18n, ENUM_CODE_KEYS
from ..model import Kind

# 列像素宽（表头与单元格共用，保证对齐）
COL_PX = {
    "#": 36,
    "name": 110,
    "type": 150,
    "offset": 72,
    "length": 72,
    "scale": 64,
    "rounding": 150,
    "overflow": 170,
    "overflowFill": 88,
    "align": 150,
    "padFill": 72,
    "missingFill": 88,
    "missingPolicy": 190,
}

PADX = 1
PADY = 1


class InlineSchemaTable(ttk.Frame):
    """单 grid：表头 + 数据行同列宽，避免各行各自 layout 错位。"""

    def __init__(self, master, i18n: I18n, **kw):
        super().__init__(master, **kw)
        self.i18n = i18n
        self.kind: Kind = "data"
        self.columns: List[str] = []
        self._selected = -1
        self._on_select: Optional[Callable[[int], None]] = None
        # 每行: {col: StringVar}；枚举列存英文 code
        self._row_vars: List[Dict[str, tk.StringVar]] = []
        # 枚举列额外存显示用 StringVar
        self._row_disp: List[Dict[str, tk.StringVar]] = []
        self._row_bg: List[tk.Frame] = []

        body = ttk.Frame(self)
        body.pack(fill=tk.BOTH, expand=True)
        self.canvas = tk.Canvas(
            body, highlightthickness=0, borderwidth=1, relief=tk.SUNKEN, bg="#f5f5f5"
        )
        self.vsb = ttk.Scrollbar(body, orient=tk.VERTICAL, command=self.canvas.yview)
        self.hsb = ttk.Scrollbar(self, orient=tk.HORIZONTAL, command=self.canvas.xview)
        self.inner = tk.Frame(self.canvas, bg="#f5f5f5")
        self._win = self.canvas.create_window((0, 0), window=self.inner, anchor="nw")

        self.canvas.configure(yscrollcommand=self.vsb.set, xscrollcommand=self.hsb.set)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.vsb.pack(side=tk.RIGHT, fill=tk.Y)
        self.hsb.pack(side=tk.BOTTOM, fill=tk.X)

        self.inner.bind("<Configure>", self._on_inner_configure)
        self.canvas.bind("<Configure>", self._on_canvas_configure)
        self.canvas.bind(
            "<Enter>", lambda e: self.canvas.bind_all("<MouseWheel>", self._on_mousewheel)
        )
        self.canvas.bind("<Leave>", lambda e: self.canvas.unbind_all("<MouseWheel>"))

    def set_i18n(self, i18n: I18n):
        self.i18n = i18n

    def set_on_select(self, cb: Callable[[int], None]):
        self._on_select = cb

    def selected_index(self) -> int:
        return self._selected

    def _on_inner_configure(self, _e=None):
        self.canvas.configure(scrollregion=self.canvas.bbox("all"))

    def _on_canvas_configure(self, e):
        need = self.inner.winfo_reqwidth()
        self.canvas.itemconfigure(self._win, width=max(e.width, need))

    def _on_mousewheel(self, e):
        widget = self.winfo_containing(e.x_root, e.y_root)
        if widget is None:
            return
        self.canvas.yview_scroll(int(-1 * (e.delta / 120)), "units")

    def _col_keys(self) -> List[str]:
        return ["#"] + list(self.columns)

    def _clear(self):
        for w in self.inner.winfo_children():
            w.destroy()
        self._row_vars.clear()
        self._row_disp.clear()
        self._row_bg.clear()
        self._selected = -1

    def _configure_columns(self):
        keys = self._col_keys()
        for c, key in enumerate(keys):
            px = COL_PX.get(key, 90)
            self.inner.columnconfigure(c, minsize=px, weight=0)

    def rebuild(self, kind: Kind, columns: List[str], rows: List[dict]):
        self.kind = kind
        self.columns = list(columns)
        self._clear()
        self._configure_columns()

        # 表头 row=0
        for c, key in enumerate(self._col_keys()):
            label = (
                self.i18n.column_label("#")
                if key == "#"
                else self.i18n.column_label(key)
            )
            hdr = tk.Label(
                self.inner,
                text=label,
                width=1,
                anchor="w",
                padx=4,
                pady=4,
                relief=tk.GROOVE,
                bg="#e8e8e8",
                font=("Segoe UI", 9, "bold"),
            )
            hdr.grid(row=0, column=c, sticky="nsew", padx=PADX, pady=PADY)
            self.inner.grid_columnconfigure(c, minsize=COL_PX.get(key, 90))

        for i, row in enumerate(rows):
            self._place_row(i + 1, i, row)

        self._on_inner_configure()

    def _place_row(self, grid_row: int, index: int, row: dict):
        vars_map: Dict[str, tk.StringVar] = {}
        disp_map: Dict[str, tk.StringVar] = {}

        # # 序号
        num = tk.Label(
            self.inner,
            text=str(index + 1),
            anchor="center",
            relief=tk.GROOVE,
            bg="#ffffff",
            padx=2,
            pady=2,
        )
        num.grid(row=grid_row, column=0, sticky="nsew", padx=PADX, pady=PADY)
        num.bind("<Button-1>", lambda e, i=index: self._select(i))

        for c, col in enumerate(self.columns, start=1):
            raw = str(row.get(col, ""))
            code_var = tk.StringVar(value=raw)
            vars_map[col] = code_var
            px = COL_PX.get(col, 90)

            if col in ENUM_CODE_KEYS:
                display = self.i18n.enum_label(col, raw) if raw else ""
                # 若 raw 已是旧显示串，归一成 code
                if raw and " — " in raw:
                    code = self.i18n.enum_code_from_display(col, raw)
                    code_var.set(code)
                    display = self.i18n.enum_label(col, code)
                disp_var = tk.StringVar(value=display)
                disp_map[col] = disp_var
                cb = ttk.Combobox(
                    self.inner,
                    textvariable=disp_var,
                    values=self.i18n.enum_choices(col),
                    state="readonly",
                    width=max(8, px // 8),
                )
                cb.grid(row=grid_row, column=c, sticky="nsew", padx=PADX, pady=PADY)

                def on_pick(_e=None, g=col, cv=code_var, dv=disp_var, i=index):
                    cv.set(self.i18n.enum_code_from_display(g, dv.get()))
                    self._select(i)

                cb.bind("<<ComboboxSelected>>", on_pick)
                cb.bind("<Button-1>", lambda e, i=index: self._select(i))
            else:
                ent = ttk.Entry(self.inner, textvariable=code_var, width=max(4, px // 9))
                ent.grid(row=grid_row, column=c, sticky="nsew", padx=PADX, pady=PADY)
                ent.bind("<FocusIn>", lambda e, i=index: self._select(i))

        self._row_vars.append(vars_map)
        self._row_disp.append(disp_map)
        # 用序号 label 作选中高亮代理
        self._row_bg.append(num)

    def _select(self, index: int):
        self._selected = index
        for i, lab in enumerate(self._row_bg):
            if i == index:
                lab.configure(bg="#cde4ff", relief=tk.SOLID, bd=2)
            else:
                lab.configure(bg="#ffffff", relief=tk.GROOVE, bd=1)
        if self._on_select:
            self._on_select(index)

    def collect_rows(self) -> List[dict]:
        out: List[dict] = []
        for vars_map, disp_map in zip(self._row_vars, self._row_disp):
            row = {}
            for k, v in vars_map.items():
                if k in ENUM_CODE_KEYS and k in disp_map:
                    row[k] = self.i18n.enum_code_from_display(k, disp_map[k].get())
                else:
                    row[k] = v.get().strip()
            out.append(row)
        return out

    def append_default_row(self, row: dict):
        index = len(self._row_vars)
        self._place_row(index + 1, index, row)
        self._select(index)
        self._on_inner_configure()
        self.canvas.yview_moveto(1.0)

    def refresh_labels(self):
        """语言切换后：同步表头与枚举下拉显示（保留 code）。"""
        rows = self.collect_rows()
        self.rebuild(self.kind, self.columns, rows)
