#!/usr/bin/env python3
"""Compile the production pipeline against a recording test SDK, not a GPU.

This does not replace a Crown build: the test SDK substitutes Crown's containers,
SJSON frontend, hashing, shader manager, bgfx and the rectangle packer. The actual
pipeline compiler helper, binary layout, generic executor and native callbacks
are included directly from src and exercised by isolated.cpp. lighting.cpp also
compiles the production ShaderData declaration and ShaderManager implementation;
only its supporting APIs and shader-binary reflection results are test doubles.
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
    parser.add_argument("--lighting-only", action="store_true", help="Run the shader-loading/lighting-binding regression only")
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
        "world/types.h",
        "bgfx/bgfx.h", "bx/math.h", "stb_rect_pack.h",
        "core/filesystem/types.h", "core/memory/types.h", "core/strings/types.h",
        "core/memory/temp_allocator.inl", "core/strings/string.inl",
        "core/filesystem/file_memory.inl", "core/filesystem/reader_writer.inl",
        "resource/resource_manager.h",
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
        manager_header = include / "world/shader_manager.h"
        manager_header.parent.mkdir(parents=True, exist_ok=True)
        manager_header.write_text(
            '#pragma once\n#include "test_sdk.h"\n'
            '#ifdef CROWN_TEST_REAL_SHADER_MANAGER\n'
            f'#include "{(root / "src/world/shader_manager.h").as_posix()}"\n'
            '#endif\n', encoding="utf-8")
        source = (root / "src/resource/render_config_resource.cpp").read_text(encoding="utf-8")
        start = source.index("\tstruct PipelineCompiler\n")
        end = source.index("\n\ts32 compile(CompileOptions &opts)", start)
        (include / "pipeline_compiler_under_test.h").write_text(source[start:end], encoding="utf-8")
        accessors = source[source.index("\nnamespace render_config_resource\n"):source.rindex("\n} // namespace crown")]
        (include / "render_config_accessors_under_test.h").write_text("namespace crown {" + accessors + "}\n", encoding="utf-8")
        suites = [] if args.lighting_only else [("isolated.cpp", [], [str(root / "samples/core/renderer/default.render_config")])]
        for reload_enabled in (0, 1):
            suites.append(("lighting.cpp", [f"-DCROWN_CAN_RELOAD={reload_enabled}"], []))
        for index, (suite, flags, arguments) in enumerate(suites):
            binary = work / f"pipeline-test-{index}"
            command = compiler + ["-std=c++17", "-Wall", "-Wextra", "-Werror"] + flags
            if args.sanitize:
                command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
            command += ["-I" + str(include), "-I" + str(root / "src"), str(here / suite), "-o", str(binary)]
            print(f"Building {suite} {flags} against test doubles.", flush=True)
            subprocess.run(command, check=True)
            subprocess.run([str(binary)] + arguments, check=True)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        raise SystemExit(exc.returncode)
