# -*- coding: utf-8 -*-
"""界面与枚举翻译：相对 exe（或脚本）目录的 ./lang。"""

from __future__ import annotations

import json
import os
import sys
from copy import deepcopy
from typing import Any, Dict, List, Optional, Tuple

# 启动时若缺失则写入；用户可改 lang 下文件后重启生效
DEFAULT_LANGS: Dict[str, Dict[str, Any]] = {
    "zh_CN": {
        "meta": {"lang": "zh_CN", "name": "简体中文"},
        "ui": {
            "app.title": "GMAM Schema 生成器 v0.0.2",
            "menu.file": "文件",
            "menu.export": "导出",
            "menu.lang": "语言",
            "menu.help": "帮助",
            "menu.new": "新建",
            "menu.open": "打开…",
            "menu.save": "保存",
            "menu.save_as": "另存为…",
            "menu.exit": "退出",
            "menu.export_cpp": "导出 C++ 片段…",
            "menu.about": "关于",
            "btn.new": "新建",
            "btn.open": "打开",
            "btn.save": "保存",
            "btn.save_as": "另存为",
            "btn.add": "增行",
            "btn.del": "删行",
            "btn.up": "上移",
            "btn.down": "下移",
            "btn.validate": "校验",
            "btn.export_cpp": "导出C++",
            "log.title": "日志 / 校验",
            "status.unsaved": "(未保存)",
            "status.line": "{path} | kind={kind} | 行={rows} | totalLength≈{total}",
            "dlg.new.title": "新建 Schema",
            "dlg.new.prompt": "选择类型：",
            "dlg.new.kind_data": "data — 数据体字段",
            "dlg.new.kind_header": "header — 报文头字段",
            "dlg.new.ok": "确定",
            "dlg.new.cancel": "取消",
            "dlg.open.title": "打开 schema",
            "dlg.save_as.title": "另存为",
            "dlg.cpp.frame": "C++ 导出",
            "dlg.cpp.copy": "复制",
            "log.exported": "已生成 C++ 到右侧面板",
            "msg.open_fail": "打开失败",
            "msg.save_fail": "保存失败",
            "msg.tip": "提示",
            "msg.confirm": "确认",
            "msg.validate": "校验",
            "msg.export_fail": "导出失败",
            "msg.copy": "复制",
            "msg.about": "关于",
            "msg.select_row_del": "请先点击要删除的行",
            "msg.del_confirm": "删除第 {n} 行？",
            "msg.validate_ok": "通过",
            "msg.validate_errs": "{n} 个错误，见日志",
            "msg.validate_warns": "{n} 个警告，见日志",
            "msg.fix_first": "请先修复校验错误",
            "msg.copied": "已复制到剪贴板",
            "msg.about_body": (
                "GMAM Schema 生成器 v0.0.2\n"
                "行内下拉编辑；枚举中英对照来自 ./lang。\n"
                "底部右侧为 C++ 导出，可直接复制。\n"
                "数据文件仍保存英文枚举名。"
            ),
            "log.created": "已新建 kind={kind}；点「增行」即可。",
            "log.opened": "已打开: {path}",
            "log.saved": "已保存: {path}",
            "log.saved_as": "已另存为: {path}",
            "log.added": "已增行 #{n}",
            "log.validate_ok": "校验通过。",
            "log.err": "[错误] {msg}",
            "log.warn": "[警告] {msg}",
            "log.lang_switched": "界面语言已切换为 {name}（./lang）",
            "log.lang_ready": "语言目录: {path}",
        },
        "column": {
            "#": "#",
            "name": "name 字段名",
            "type": "type 类型",
            "offset": "offset 偏移",
            "length": "length 长度",
            "scale": "scale 小数位",
            "rounding": "rounding 舍入",
            "overflow": "overflow 超长",
            "overflowFill": "overflowFill",
            "align": "align 对齐",
            "padFill": "padFill 填充",
            "missingFill": "missingFill",
            "missingPolicy": "missingPolicy",
        },
        "enum": {
            "type.Numeric": "数值",
            "type.Chars": "字符",
            "type.Reserved": "保留位",
            "rounding.Truncate": "截断",
            "rounding.Round": "四舍五入",
            "overflow.Error": "报错",
            "overflow.Truncate": "截断",
            "overflow.FillChar": "超额填指定字符",
            "align.ByType": "按类型默认",
            "align.Left": "左对齐",
            "align.Right": "右对齐",
            "missingPolicy.TreatAsMissing": "空视为缺失",
            "missingPolicy.EmptyIsValue": "空串也是值",
        },
    },
    "en_US": {
        "meta": {"lang": "en_US", "name": "English"},
        "ui": {
            "app.title": "GMAM Schema Generator v0.0.2",
            "menu.file": "File",
            "menu.export": "Export",
            "menu.lang": "Language",
            "menu.help": "Help",
            "menu.new": "New",
            "menu.open": "Open…",
            "menu.save": "Save",
            "menu.save_as": "Save As…",
            "menu.exit": "Exit",
            "menu.export_cpp": "Export C++ snippet…",
            "menu.about": "About",
            "btn.new": "New",
            "btn.open": "Open",
            "btn.save": "Save",
            "btn.save_as": "Save As",
            "btn.add": "Add",
            "btn.del": "Delete",
            "btn.up": "Up",
            "btn.down": "Down",
            "btn.validate": "Validate",
            "btn.export_cpp": "Export C++",
            "log.title": "Log / Validate",
            "status.unsaved": "(unsaved)",
            "status.line": "{path} | kind={kind} | rows={rows} | totalLength≈{total}",
            "dlg.new.title": "New Schema",
            "dlg.new.prompt": "Choose kind:",
            "dlg.new.kind_data": "data — body fields",
            "dlg.new.kind_header": "header — header fields",
            "dlg.new.ok": "OK",
            "dlg.new.cancel": "Cancel",
            "dlg.open.title": "Open schema",
            "dlg.save_as.title": "Save As",
            "dlg.cpp.frame": "C++ Export",
            "dlg.cpp.copy": "Copy",
            "log.exported": "C++ written to right panel",
            "msg.open_fail": "Open failed",
            "msg.save_fail": "Save failed",
            "msg.tip": "Tip",
            "msg.confirm": "Confirm",
            "msg.validate": "Validate",
            "msg.export_fail": "Export failed",
            "msg.copy": "Copy",
            "msg.about": "About",
            "msg.select_row_del": "Click a row to delete first",
            "msg.del_confirm": "Delete row {n}?",
            "msg.validate_ok": "OK",
            "msg.validate_errs": "{n} error(s), see log",
            "msg.validate_warns": "{n} warning(s), see log",
            "msg.fix_first": "Fix validation errors first",
            "msg.copied": "Copied to clipboard",
            "msg.about_body": (
                "GMAM Schema Generator v0.0.2\n"
                "Inline enum dropdowns; labels from ./lang.\n"
                "C++ export is on the bottom-right panel.\n"
                "Schema files still store English enum codes."
            ),
            "log.created": "Created kind={kind}; click Add to insert rows.",
            "log.opened": "Opened: {path}",
            "log.saved": "Saved: {path}",
            "log.saved_as": "Saved as: {path}",
            "log.added": "Added row #{n}",
            "log.validate_ok": "Validation passed.",
            "log.err": "[error] {msg}",
            "log.warn": "[warn] {msg}",
            "log.lang_switched": "UI language: {name} (./lang)",
            "log.lang_ready": "Language dir: {path}",
        },
        "column": {
            "#": "#",
            "name": "name",
            "type": "type",
            "offset": "offset",
            "length": "length",
            "scale": "scale",
            "rounding": "rounding",
            "overflow": "overflow",
            "overflowFill": "overflowFill",
            "align": "align",
            "padFill": "padFill",
            "missingFill": "missingFill",
            "missingPolicy": "missingPolicy",
        },
        "enum": {
            "type.Numeric": "numeric digits",
            "type.Chars": "character bytes",
            "type.Reserved": "reserved padding",
            "rounding.Truncate": "truncate",
            "rounding.Round": "round half up",
            "overflow.Error": "error on overflow",
            "overflow.Truncate": "truncate overflow",
            "overflow.FillChar": "fill with overflowFill",
            "align.ByType": "default by type",
            "align.Left": "left align",
            "align.Right": "right align",
            "missingPolicy.TreatAsMissing": "empty → missing",
            "missingPolicy.EmptyIsValue": "empty is a value",
        },
    },
}

ENUM_CODE_KEYS = {
    "type": ("Numeric", "Chars", "Reserved"),
    "rounding": ("Truncate", "Round"),
    "overflow": ("Error", "Truncate", "FillChar"),
    "align": ("ByType", "Left", "Right"),
    "missingPolicy": ("TreatAsMissing", "EmptyIsValue"),
}


def app_base_dir() -> str:
    """exe 所在目录；开发时为 main.py 所在目录（src_gmam_schema_gen_py）。"""
    if getattr(sys, "frozen", False):
        return os.path.dirname(os.path.abspath(sys.executable))
    # gmam_schema_gen/i18n.py → 上两级到工程包根
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def lang_dir() -> str:
    return os.path.join(app_base_dir(), "lang")


def ensure_lang_files(directory: Optional[str] = None) -> str:
    """确保 ./lang 存在，并写入缺失的默认 zh_CN / en_US。返回目录路径。"""
    d = directory or lang_dir()
    os.makedirs(d, exist_ok=True)
    for code, catalog in DEFAULT_LANGS.items():
        path = os.path.join(d, f"{code}.json")
        if not os.path.isfile(path):
            with open(path, "w", encoding="utf-8") as f:
                json.dump(catalog, f, ensure_ascii=False, indent=2)
                f.write("\n")
    return d


def list_lang_files(directory: Optional[str] = None) -> List[Tuple[str, str]]:
    """返回 [(lang_code, display_name), ...]。"""
    d = directory or lang_dir()
    out: List[Tuple[str, str]] = []
    if not os.path.isdir(d):
        return out
    for name in sorted(os.listdir(d)):
        if not name.endswith(".json"):
            continue
        code = name[:-5]
        path = os.path.join(d, name)
        try:
            with open(path, encoding="utf-8") as f:
                data = json.load(f)
            display = data.get("meta", {}).get("name") or code
        except (OSError, json.JSONDecodeError, TypeError):
            display = code
        out.append((code, display))
    return out


class I18n:
    def __init__(self, lang_code: str = "zh_CN", directory: Optional[str] = None):
        self.directory = directory or lang_dir()
        self.lang_code = lang_code
        self._data: Dict[str, Any] = {}
        self.load(lang_code)

    def load(self, lang_code: str) -> None:
        path = os.path.join(self.directory, f"{lang_code}.json")
        if not os.path.isfile(path):
            # 回退内置
            self._data = deepcopy(DEFAULT_LANGS.get(lang_code) or DEFAULT_LANGS["zh_CN"])
            self.lang_code = self._data.get("meta", {}).get("lang", lang_code)
            return
        with open(path, encoding="utf-8") as f:
            self._data = json.load(f)
        self.lang_code = lang_code

    def t(self, key: str, **kwargs) -> str:
        ui = self._data.get("ui") or {}
        text = ui.get(key)
        if text is None:
            # 回退默认中文再英文
            text = (
                DEFAULT_LANGS["zh_CN"]["ui"].get(key)
                or DEFAULT_LANGS["en_US"]["ui"].get(key)
                or key
            )
        if kwargs:
            try:
                return text.format(**kwargs)
            except (KeyError, ValueError):
                return text
        return text

    def column_label(self, col: str) -> str:
        cols = self._data.get("column") or {}
        if col in cols:
            return cols[col]
        return (
            DEFAULT_LANGS["zh_CN"]["column"].get(col)
            or DEFAULT_LANGS["en_US"]["column"].get(col)
            or col
        )

    def enum_label(self, group: str, code: str) -> str:
        key = f"{group}.{code}"
        enums = self._data.get("enum") or {}
        desc = enums.get(key)
        if not desc:
            desc = DEFAULT_LANGS["zh_CN"]["enum"].get(key) or ""
        if desc:
            return f"{code} — {desc}"
        return code

    def enum_choices(self, group: str) -> List[str]:
        codes = ENUM_CODE_KEYS.get(group, ())
        return [self.enum_label(group, c) for c in codes]

    def enum_code_from_display(self, group: str, display: str) -> str:
        display = (display or "").strip()
        if " — " in display:
            return display.split(" — ", 1)[0].strip()
        # 已是裸 code
        codes = ENUM_CODE_KEYS.get(group, ())
        if display in codes:
            return display
        # 匹配当前语言标签
        for c in codes:
            if self.enum_label(group, c) == display:
                return c
        return display

    def lang_name(self) -> str:
        return (self._data.get("meta") or {}).get("name") or self.lang_code
