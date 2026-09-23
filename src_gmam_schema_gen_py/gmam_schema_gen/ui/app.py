# -*- coding: utf-8 -*-
"""主窗口 v0.0.2：新建选项、行内双语下拉、./lang 翻译。"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk, filedialog, messagebox

from typing import Optional

from ..model import SchemaDocument, default_row, Kind
from ..format_io import (
    SchemaFormatError,
    load_file,
    save_file,
    validate_document_with_warnings,
)
from ..cpp_export import export_cpp
from ..i18n import I18n, ensure_lang_files, list_lang_files
from .table_view import InlineSchemaTable


class KindDialog(tk.Toplevel):
    """新建：用单选选择 data / header。"""

    def __init__(self, master, i18n: I18n):
        super().__init__(master)
        self.i18n = i18n
        self.result: Optional[Kind] = None
        self.title(i18n.t("dlg.new.title"))
        self.transient(master)
        self.resizable(False, False)
        self.grab_set()

        frm = ttk.LabelFrame(self, text=i18n.t("dlg.new.prompt"), padding=12)
        frm.pack(fill=tk.BOTH, expand=True, padx=12, pady=12)

        self.var = tk.StringVar(value="data")
        ttk.Radiobutton(
            frm, text=i18n.t("dlg.new.kind_data"), variable=self.var, value="data"
        ).pack(anchor="w", pady=4)
        ttk.Radiobutton(
            frm, text=i18n.t("dlg.new.kind_header"), variable=self.var, value="header"
        ).pack(anchor="w", pady=4)

        btns = ttk.Frame(self, padding=8)
        btns.pack(fill=tk.X)
        ttk.Button(btns, text=i18n.t("dlg.new.ok"), command=self._ok).pack(
            side=tk.RIGHT, padx=4
        )
        ttk.Button(btns, text=i18n.t("dlg.new.cancel"), command=self._cancel).pack(
            side=tk.RIGHT, padx=4
        )

        self.bind("<Return>", lambda e: self._ok())
        self.bind("<Escape>", lambda e: self._cancel())
        self.protocol("WM_DELETE_WINDOW", self._cancel)

        self.update_idletasks()
        if master:
            x = master.winfo_rootx() + 80
            y = master.winfo_rooty() + 80
            self.geometry(f"+{x}+{y}")
        self.wait_window(self)

    def _ok(self):
        v = self.var.get()
        if v in ("data", "header"):
            self.result = v  # type: ignore[assignment]
        self.destroy()

    def _cancel(self):
        self.result = None
        self.destroy()


class SchemaApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self._lang_path = ensure_lang_files()
        self.i18n = I18n("zh_CN", self._lang_path)
        self.geometry("1280x680")
        self.doc = SchemaDocument(kind="data", rows=[])

        self._toolbar_btns: dict = {}
        self._menus: dict = {}
        self._build_menu()
        self._build_toolbar()
        self.table = InlineSchemaTable(self, self.i18n)
        self.table.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)
        self._build_bottom()
        self._apply_ui_texts()
        self._reload_table()
        self._update_status()
        self._log(self.i18n.t("log.lang_ready", path=self._lang_path))

    def _build_menu(self):
        self.menubar = tk.Menu(self)
        self.file_m = tk.Menu(self.menubar, tearoff=0)
        self.file_m.add_command(command=self.on_new)
        self.file_m.add_command(command=self.on_open)
        self.file_m.add_command(command=self.on_save)
        self.file_m.add_command(command=self.on_save_as)
        self.file_m.add_separator()
        self.file_m.add_command(command=self.destroy)

        self.exp_m = tk.Menu(self.menubar, tearoff=0)
        self.exp_m.add_command(command=self.on_export_cpp)

        self.lang_m = tk.Menu(self.menubar, tearoff=0)

        self.help_m = tk.Menu(self.menubar, tearoff=0)
        self.help_m.add_command(command=self.on_about)

        self.config(menu=self.menubar)
        # 顶栏 cascade 标签在 _apply_ui_texts 里挂上（Windows 不能事后 entryconfig label）

    def _rebuild_lang_menu(self):
        self.lang_m.delete(0, tk.END)
        for code, name in list_lang_files(self._lang_path):
            self.lang_m.add_command(
                label=name,
                command=lambda c=code: self.on_switch_lang(c),
            )

    def _rebuild_menubar_cascades(self):
        """Windows 系统菜单栏：delete+add_cascade(label=)，勿用 entryconfig(-label)。"""
        i = self.i18n
        self.menubar.delete(0, tk.END)
        self.menubar.add_cascade(label=i.t("menu.file"), menu=self.file_m)
        self.menubar.add_cascade(label=i.t("menu.export"), menu=self.exp_m)
        self.menubar.add_cascade(label=i.t("menu.lang"), menu=self.lang_m)
        self.menubar.add_cascade(label=i.t("menu.help"), menu=self.help_m)

    def _build_toolbar(self):
        self.toolbar = ttk.Frame(self, padding=4)
        self.toolbar.pack(fill=tk.X)
        specs = (
            ("new", self.on_new),
            ("open", self.on_open),
            ("save", self.on_save),
            ("save_as", self.on_save_as),
            ("add", self.on_add),
            ("del", self.on_delete),
            ("up", self.on_up),
            ("down", self.on_down),
            ("validate", self.on_validate),
            ("export_cpp", self.on_export_cpp),
        )
        for key, cmd in specs:
            b = ttk.Button(self.toolbar, command=cmd)
            b.pack(side=tk.LEFT, padx=2)
            self._toolbar_btns[key] = b
        self.status = ttk.Label(self.toolbar, text="")
        self.status.pack(side=tk.RIGHT, padx=8)

    def _build_bottom(self):
        """底部：日志 | C++ 导出（并排，无弹窗）。"""
        bottom = ttk.Frame(self)
        bottom.pack(fill=tk.BOTH, padx=4, pady=4)
        bottom.columnconfigure(0, weight=1)
        bottom.columnconfigure(1, weight=1)
        bottom.rowconfigure(0, weight=1)

        self.log_frm = ttk.LabelFrame(bottom, padding=4)
        self.log_frm.grid(row=0, column=0, sticky="nsew", padx=(0, 2))
        self.log = tk.Text(self.log_frm, height=8, wrap=tk.WORD)
        self.log.pack(fill=tk.BOTH, expand=True)

        self.cpp_frm = ttk.LabelFrame(bottom, padding=4)
        self.cpp_frm.grid(row=0, column=1, sticky="nsew", padx=(2, 0))
        cpp_bar = ttk.Frame(self.cpp_frm)
        cpp_bar.pack(fill=tk.X)
        self.btn_copy_cpp = ttk.Button(cpp_bar, command=self.on_copy_cpp)
        self.btn_copy_cpp.pack(side=tk.RIGHT, padx=2)
        self.btn_gen_cpp = ttk.Button(cpp_bar, command=self.on_export_cpp)
        self.btn_gen_cpp.pack(side=tk.RIGHT, padx=2)

        cpp_body = ttk.Frame(self.cpp_frm)
        cpp_body.pack(fill=tk.BOTH, expand=True)
        self.cpp_text = tk.Text(cpp_body, height=8, wrap=tk.NONE)
        ys = ttk.Scrollbar(cpp_body, orient=tk.VERTICAL, command=self.cpp_text.yview)
        xs = ttk.Scrollbar(cpp_body, orient=tk.HORIZONTAL, command=self.cpp_text.xview)
        self.cpp_text.configure(yscrollcommand=ys.set, xscrollcommand=xs.set)
        self.cpp_text.grid(row=0, column=0, sticky="nsew")
        ys.grid(row=0, column=1, sticky="ns")
        xs.grid(row=1, column=0, sticky="ew")
        cpp_body.rowconfigure(0, weight=1)
        cpp_body.columnconfigure(0, weight=1)

    def _apply_ui_texts(self):
        i = self.i18n
        self.title(i.t("app.title"))
        self._rebuild_menubar_cascades()
        # 子菜单项可以安全改 label
        self.file_m.entryconfig(0, label=i.t("menu.new"))
        self.file_m.entryconfig(1, label=i.t("menu.open"))
        self.file_m.entryconfig(2, label=i.t("menu.save"))
        self.file_m.entryconfig(3, label=i.t("menu.save_as"))
        self.file_m.entryconfig(5, label=i.t("menu.exit"))
        self.exp_m.entryconfig(0, label=i.t("menu.export_cpp"))
        self.help_m.entryconfig(0, label=i.t("menu.about"))
        self._rebuild_lang_menu()

        key_map = {
            "new": "btn.new",
            "open": "btn.open",
            "save": "btn.save",
            "save_as": "btn.save_as",
            "add": "btn.add",
            "del": "btn.del",
            "up": "btn.up",
            "down": "btn.down",
            "validate": "btn.validate",
            "export_cpp": "btn.export_cpp",
        }
        for k, ui_key in key_map.items():
            self._toolbar_btns[k].config(text=i.t(ui_key))
        self.log_frm.config(text=i.t("log.title"))
        self.cpp_frm.config(text=i.t("dlg.cpp.frame"))
        self.btn_gen_cpp.config(text=i.t("btn.export_cpp"))
        self.btn_copy_cpp.config(text=i.t("dlg.cpp.copy"))

    def _log(self, msg: str):
        self.log.insert(tk.END, msg + "\n")
        self.log.see(tk.END)

    def _clear_log(self):
        self.log.delete("1.0", tk.END)

    def _sync_doc_from_table(self):
        self.doc.rows = self.table.collect_rows()

    def _reload_table(self):
        self.table.set_i18n(self.i18n)
        self.table.rebuild(self.doc.kind, self.doc.columns(), self.doc.rows)

    def _update_status(self):
        path = self.doc.path or self.i18n.t("status.unsaved")
        self._sync_doc_from_table()
        self.status.config(
            text=self.i18n.t(
                "status.line",
                path=path,
                kind=self.doc.kind,
                rows=len(self.doc.rows),
                total=self.doc.estimated_total_length(),
            )
        )

    def on_switch_lang(self, code: str):
        self._sync_doc_from_table()
        self.i18n.load(code)
        self._apply_ui_texts()
        self._reload_table()
        self._update_status()
        self._log(self.i18n.t("log.lang_switched", name=self.i18n.lang_name()))

    def on_new(self):
        dlg = KindDialog(self, self.i18n)
        if not dlg.result:
            return
        kind = dlg.result
        self.doc = SchemaDocument(kind=kind, rows=[])
        self._reload_table()
        self._update_status()
        self._clear_log()
        self._log(self.i18n.t("log.created", kind=kind))

    def on_open(self):
        path = filedialog.askopenfilename(
            title=self.i18n.t("dlg.open.title"),
            filetypes=[("GMAM schema", "*.gmam.txt"), ("Text", "*.txt"), ("All", "*.*")],
        )
        if not path:
            return
        try:
            self.doc = load_file(path)
        except SchemaFormatError as e:
            messagebox.showerror(
                self.i18n.t("msg.open_fail"), "\n".join(e.errors), parent=self
            )
            return
        except OSError as e:
            messagebox.showerror(self.i18n.t("msg.open_fail"), str(e), parent=self)
            return
        self._reload_table()
        self._update_status()
        self._clear_log()
        _, warns = validate_document_with_warnings(self.doc)
        self._log(self.i18n.t("log.opened", path=path))
        for w in warns:
            self._log(self.i18n.t("log.warn", msg=w))

    def on_save(self):
        self._sync_doc_from_table()
        if not self.doc.path:
            self.on_save_as()
            return
        try:
            save_file(self.doc, self.doc.path)
        except SchemaFormatError as e:
            messagebox.showerror(
                self.i18n.t("msg.save_fail"), "\n".join(e.errors), parent=self
            )
            return
        self._update_status()
        self._log(self.i18n.t("log.saved", path=self.doc.path))

    def on_save_as(self):
        self._sync_doc_from_table()
        path = filedialog.asksaveasfilename(
            title=self.i18n.t("dlg.save_as.title"),
            defaultextension=".gmam.txt",
            filetypes=[("GMAM schema", "*.gmam.txt"), ("Text", "*.txt")],
        )
        if not path:
            return
        try:
            save_file(self.doc, path)
        except SchemaFormatError as e:
            messagebox.showerror(
                self.i18n.t("msg.save_fail"), "\n".join(e.errors), parent=self
            )
            return
        self._update_status()
        self._log(self.i18n.t("log.saved_as", path=path))

    def on_add(self):
        self._sync_doc_from_table()
        row = default_row(self.doc.kind)
        row["name"] = f"field{len(self.doc.rows) + 1}"
        self.doc.rows.append(row)
        self.table.append_default_row(row)
        self._update_status()
        self._log(self.i18n.t("log.added", n=len(self.doc.rows)))

    def on_delete(self):
        self._sync_doc_from_table()
        i = self.table.selected_index()
        if i < 0 or i >= len(self.doc.rows):
            messagebox.showinfo(
                self.i18n.t("msg.tip"), self.i18n.t("msg.select_row_del"), parent=self
            )
            return
        if not messagebox.askyesno(
            self.i18n.t("msg.confirm"),
            self.i18n.t("msg.del_confirm", n=i + 1),
            parent=self,
        ):
            return
        del self.doc.rows[i]
        self._reload_table()
        self._update_status()

    def on_up(self):
        self._sync_doc_from_table()
        i = self.table.selected_index()
        if i <= 0:
            return
        self.doc.rows[i - 1], self.doc.rows[i] = self.doc.rows[i], self.doc.rows[i - 1]
        self._reload_table()
        self.table._select(i - 1)
        self._update_status()

    def on_down(self):
        self._sync_doc_from_table()
        i = self.table.selected_index()
        if i < 0 or i >= len(self.doc.rows) - 1:
            return
        self.doc.rows[i + 1], self.doc.rows[i] = self.doc.rows[i], self.doc.rows[i + 1]
        self._reload_table()
        self.table._select(i + 1)
        self._update_status()

    def on_validate(self):
        self._sync_doc_from_table()
        self._clear_log()
        errs, warns = validate_document_with_warnings(self.doc)
        if not errs and not warns:
            self._log(self.i18n.t("log.validate_ok"))
            messagebox.showinfo(
                self.i18n.t("msg.validate"),
                self.i18n.t("msg.validate_ok"),
                parent=self,
            )
            return
        for e in errs:
            self._log(self.i18n.t("log.err", msg=e))
        for w in warns:
            self._log(self.i18n.t("log.warn", msg=w))
        if errs:
            messagebox.showerror(
                self.i18n.t("msg.validate"),
                self.i18n.t("msg.validate_errs", n=len(errs)),
                parent=self,
            )
        else:
            messagebox.showwarning(
                self.i18n.t("msg.validate"),
                self.i18n.t("msg.validate_warns", n=len(warns)),
                parent=self,
            )

    def on_export_cpp(self):
        """生成 C++ 到右侧面板（不弹窗）。"""
        self._sync_doc_from_table()
        errs, _ = validate_document_with_warnings(self.doc)
        if errs:
            messagebox.showerror(
                self.i18n.t("msg.export_fail"),
                self.i18n.t("msg.fix_first"),
                parent=self,
            )
            return
        code = export_cpp(self.doc)
        self.cpp_text.delete("1.0", tk.END)
        self.cpp_text.insert("1.0", code)
        self._log(self.i18n.t("log.exported"))

    def on_copy_cpp(self):
        code = self.cpp_text.get("1.0", tk.END).rstrip("\n")
        if not code.strip():
            # 空则先尝试生成
            self.on_export_cpp()
            code = self.cpp_text.get("1.0", tk.END).rstrip("\n")
        if not code.strip():
            return
        self.clipboard_clear()
        self.clipboard_append(code)
        self._log(self.i18n.t("msg.copied"))

    def on_about(self):
        messagebox.showinfo(
            self.i18n.t("msg.about"),
            self.i18n.t("msg.about_body"),
            parent=self,
        )


def run_app():
    ensure_lang_files()
    app = SchemaApp()
    app.mainloop()
