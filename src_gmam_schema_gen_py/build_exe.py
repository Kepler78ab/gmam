# -*- coding: utf-8 -*-
"""
使用 PyInstaller 打包 GMAM Schema 生成器为 exe。

用法（在项目根目录 src_gmam_schema_gen_py 下）:
    python build_exe.py

依赖:
    pip install pyinstaller
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "main.py"
NAME = "GmamSchemaGen"


def main() -> int:
    if not MAIN.is_file():
        print(f"找不到入口: {MAIN}")
        return 1

    try:
        import PyInstaller  # noqa: F401
    except ImportError:
        print("未安装 PyInstaller，请执行: pip install pyinstaller")
        return 1

    cmd = [
        sys.executable,
        "-m",
        "PyInstaller",
        "--noconfirm",
        "--clean",
        "-F",  # 单文件
        "-w",  # 无控制台
        f"--name={NAME}",
        "--paths",
        str(ROOT),
        str(MAIN),
    ]
    print("运行:", " ".join(cmd))
    r = subprocess.call(cmd, cwd=str(ROOT))
    if r != 0:
        print("打包失败，退出码", r)
        return r

    dist = ROOT / "dist" / f"{NAME}.exe"
    print("完成:", dist if dist.is_file() else ROOT / "dist")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
