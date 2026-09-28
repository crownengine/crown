#!/usr/bin/env python3
"""Compile the production pipeline against a recording test SDK, not a GPU.

This does not replace a Crown build: the test SDK substitutes Crown's containers,
SJSON frontend, hashing, shader manager, bgfx and the rectangle packer. The actual
pipeline compiler helper, binary layout, generic executor and native callbacks
are included directly from src and exercised by isolated.cpp.
"""
import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sanitize", action="store_true", help="Enable AddressSanitizer and UndefinedBehaviorSanitizer")
    args = parser.parse_args()
    here = Path(__file__).resolve().parent
    root = here.parent.parent
    compiler = shlex.split(os.environ.get("CXX", "c++"))
    if not compiler or shutil.which(compiler[0]) is None:
        parser.error("No C++ compiler found. Set CXX to a C++17-capable GCC or Clang command.")
    headers = (
        "config.h", "core/containers/types.h", "core/containers/array.inl",
        "core/containers/hash_map.inl", "core/math/types.h", "core/math/matrix4x4.inl",
        "core/strings/string_id.h", "core/strings/string_id.inl", "core/value.h",
        "core/types.h", "core/memory/allocator.h", "core/memory/globals.h",
        "core/error/error.h", "resource/types.h", "resource/material_resource.h",
        "resource/shader_resource.h", "world/types.h", "world/shader_manager.h",
        "bgfx/bgfx.h", "bx/math.h", "stb_rect_pack.h",
    )
    with tempfile.TemporaryDirectory(prefix="crown-pipeline-test-") as tmp:
        work = Path(tmp)
        include = work / "include"
        include.mkdir()
        shutil.copyfile(here / "test_sdk.h", include / "test_sdk.h")
        for name in headers:
            target = include / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text('#pragma once\n#include "test_sdk.h"\n', encoding="utf-8")
        binary = work / "pipeline-test"
        command = compiler + ["-std=c++17", "-Wall", "-Wextra", "-Werror"]
        if args.sanitize:
            command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        command += ["-I" + str(include), "-I" + str(root / "src"), str(here / "isolated.cpp"), "-o", str(binary)]
        print("Building production pipeline sources against test doubles.", flush=True)
        subprocess.run(command, check=True)
        subprocess.run([str(binary), str(root / "samples/core/renderer/default.render_config")], check=True)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        raise SystemExit(exc.returncode)
