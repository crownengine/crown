#
# Copyright (c) 2012-2026 Daniele Bartolini et al.
# SPDX-License-Identifier: MIT
#

UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
	EXE_PREFIX=./
	EXE_SUFFIX=
else
ifeq ($(UNAME), $(filter $(UNAME), windows32))
	OS=windows
	EXE_PREFIX=
	EXE_SUFFIX=.exe
	ARG_PREFIX=/
	MKDIR=mkdir
else
	OS=windows
	EXE_PREFIX=
	EXE_SUFFIX=.exe
	ARG_PREFIX=//
	MKDIR=mkdir -p
endif
endif

GENIE=scripts/genie/bin/$(OS)/genie
MAKE_JOBS=1

# LuaJIT
NDKABI=$(ANDROID_NDK_ABI)
NDKDIR=$(ANDROID_NDK_ROOT)
NDKBIN=$(NDKDIR)/toolchains/llvm/prebuilt/linux-x86_64/bin
NDKCROSS=$(NDKBIN)/arm-linux-androideabi-
NDKCC=$(NDKBIN)/armv7a-linux-androideabi$(NDKABI)-clang
NDKCROSS64=$(NDKBIN)/aarch64-linux-android-
NDKCC64=$(NDKBIN)/aarch64-linux-android$(NDKABI)-clang

build/android-arm/bin/libluajit.a:
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src \
		HOST_CC="gcc -m32"                             \
		CROSS=$(NDKCROSS)                              \
		STATIC_CC=$(NDKCC)                             \
		DYNAMIC_CC="$(NDKCC) -fPIC"                    \
		TARGET_LD=$(NDKCC)                             \
		TARGET_STRIP=$(NDKBIN)/llvm-strip              \
		TARGET_AR="$(NDKBIN)/llvm-ar rcus"
	-@install -m775 -D 3rdparty/luajit/src/libluajit.a $@
	-@"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/android-arm64/bin/libluajit.a:
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src \
		CROSS=$(NDKCROSS64)                            \
		STATIC_CC=$(NDKCC64)                           \
		DYNAMIC_CC="$(NDKCC64) -fPIC"                  \
		TARGET_LD=$(NDKCC64)                           \
		TARGET_STRIP=$(NDKBIN)/llvm-strip              \
		TARGET_AR="$(NDKBIN)/llvm-ar rcus"
	-@install -m775 -D 3rdparty/luajit/src/libluajit.a $@
	-@"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/linux32/bin/luajit:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="gcc -m32" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Linux BUILDMODE=static
	-@install -m775 -D 3rdparty/luajit/src/luajit $@
	-@cp -r 3rdparty/luajit/src/jit build/linux32/bin
	-@$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean
build/linux64/bin/luajit:
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Linux BUILDMODE=static
	-@install -m775 -D 3rdparty/luajit/src/luajit $@
	-@install -m664 -D 3rdparty/luajit/src/libluajit.a build/linux64/bin/libluajit.a
	-@cp -r 3rdparty/luajit/src/jit build/linux64/bin
	-@"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/mingw64/bin/luajit.exe:
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="$(MINGW)/bin/x86_64-w64-mingw32-gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" BUILDMODE=static
	-@install -m775 -D 3rdparty/luajit/src/luajit.exe $@
	-@install -m664 -D 3rdparty/luajit/src/libluajit.a build/mingw64/bin/libluajit.a
	-@cp -r 3rdparty/luajit/src/jit build/mingw64/bin
	-@"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean
	-@rm -f 3rdparty/luajit/src/libluajit*

build/windows64/bin/luajit.exe:
	cd "3rdparty/luajit/src" && .\\msvcbuild.bat
	-@install -m775 -D 3rdparty/luajit/src/luajit.exe $@
	-@install -m664 -D 3rdparty/luajit/src/lua51.dll build/windows64/bin/lua51.dll
	-@install -m664 -D 3rdparty/luajit/src/lua51.lib build/windows64/bin/lua51.lib
	-@cp -r 3rdparty/luajit/src/jit build/windows64/bin
	-@rm -f 3rdparty/luajit/src/buildvm.*
	-@rm -f 3rdparty/luajit/src/jit/vmdef.lua
	-@rm -f 3rdparty/luajit/src/lua51.*
	-@rm -f 3rdparty/luajit/src/luajit.exe
	-@rm -f 3rdparty/luajit/src/luajit.exp
	-@rm -f 3rdparty/luajit/src/luajit.lib
	-@rm -f 3rdparty/luajit/src/minilua.*

build/linux32/bin/luac: \
	build/projects/linux
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux luac config=release32
	-@install -m775 -D $@-release $@

build/mingw32/bin/luac: \
	build/projects/mingw32
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw luac config=release32
	-@install -m775 -D $@-release $@

build/linux64/bin/texturec: \
	build/projects/linux
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/bimg/.build/projects/gmake-linux-gcc config=release64 texturec
	-@install -m775 -D 3rdparty/bimg/.build/linux64_gcc/bin/texturecRelease $@
build/linux64/bin/shaderc: \
	build/projects/linux
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/bgfx/.build/projects/gmake-linux-gcc config=release64 shaderc
	-@install -m775 -D 3rdparty/bgfx/.build/linux64_gcc/bin/shadercRelease $@

build/mingw64/bin/texturec.exe: \
	build/projects/mingw
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/bimg/.build/projects/gmake-mingw-gcc config=release64 texturec
	-@install -m775 -D 3rdparty/bimg/.build/win64_mingw-gcc/bin/texturecRelease.exe $@
build/mingw64/bin/shaderc.exe: \
	build/projects/mingw
	"$(MAKE)" -j$(MAKE_JOBS) -R -C 3rdparty/bgfx/.build/projects/gmake-mingw-gcc config=release64 shaderc
	-@install -m775 -D 3rdparty/bgfx/.build/win64_mingw-gcc/bin/shadercRelease.exe $@

build/windows64/bin/texturec.exe: \
	build/projects/vs2022
	devenv.com 3rdparty/bimg/.build/projects/vs2022/bimg.sln $(ARG_PREFIX)Build "Release|x64" $(ARG_PREFIX)Project texturec.vcxproj
	-@install -m775 -D 3rdparty/bimg/.build/win64_vs2022/bin/texturecRelease.exe $@
build/windows64/bin/shaderc.exe: \
	build/projects/vs2022
	devenv.com 3rdparty/bgfx/.build/projects/vs2022/bgfx.sln $(ARG_PREFIX)Build "Release|x64" $(ARG_PREFIX)Project shaderc.vcxproj
	-@install -m775 -D 3rdparty/bgfx/.build/win64_vs2022/bin/shadercRelease.exe $@

build/projects/android-arm:
	$(GENIE) --gfxapi=gles3 --compiler=android-arm gmake
android-arm-debug:             \
	build/projects/android-arm \
	build/android-arm/bin/libluajit.a
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/android-arm crown config=debug
android-arm-development:       \
	build/projects/android-arm \
	build/android-arm/bin/libluajit.a
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/android-arm crown config=development
android-arm-release:           \
	build/projects/android-arm \
	build/android-arm/bin/libluajit.a
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/android-arm crown config=release
android-arm:                \
	android-arm-debug       \
	android-arm-development \
	android-arm-release

build/projects/android-arm64:
	$(GENIE) --gfxapi=gles3 --file=scripts/genie.lua --compiler=android-arm64 gmake
android-arm64-debug:             \
	build/projects/android-arm64 \
	build/android-arm64/bin/libluajit.a
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/android-arm64 crown config=debug
android-arm64-development:       \
	build/projects/android-arm64 \
	build/android-arm64/bin/libluajit.a
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/android-arm64 crown config=development
android-arm64-release:           \
	build/projects/android-arm64 \
	build/android-arm64/bin/libluajit.a
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/android-arm64 crown config=release
android-arm64:                \
	android-arm64-debug       \
	android-arm64-development \
	android-arm64-release

build/projects/linux:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools --gcc=linux-gcc gmake
	$(GENIE) --file=3rdparty/bimg/scripts/genie.lua --with-tools --gcc=linux-gcc gmake
	$(GENIE) --gfxapi=gl32 --with-tools --compiler=linux-gcc gmake
linux-debug64:           \
	build/projects/linux \
	build/linux64/bin/luajit
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux crown config=debug64
linux-development64:     \
	build/projects/linux \
	build/linux64/bin/luajit
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux crown config=development64
linux-release64:         \
	build/projects/linux \
	build/linux64/bin/luajit
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux crown config=release64
linux:                  \
	linux-debug64       \
	linux-development64 \
	linux-release64

build/projects/wasm:
	$(GENIE) --no-luajit --gfxapi=gles3 --compiler=wasm gmake
wasm-debug: \
	build/projects/wasm
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/wasm crown config=debug
wasm-development: \
	build/projects/wasm
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/wasm crown config=development
wasm-release: \
	build/projects/wasm
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/wasm crown config=release
wasm:                \
	wasm-debug       \
	wasm-development \
	wasm-release

build/projects/mingw32:
	$(GENIE) --gfxapi=d3d11 --with-tools --compiler=mingw-gcc --with-32bit-compiler gmake
build/projects/mingw:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools --gcc=mingw-gcc gmake
	$(GENIE) --file=3rdparty/bimg/scripts/genie.lua --with-tools --gcc=mingw-gcc gmake
	$(GENIE) --gfxapi=d3d11 --with-tools --compiler=mingw-gcc gmake
mingw-debug64:           \
	build/projects/mingw \
	build/mingw64/bin/luajit.exe
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw crown config=debug64
mingw-development64:     \
	build/projects/mingw \
	build/mingw64/bin/luajit.exe
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw crown config=development64
mingw-release64:         \
	build/projects/mingw \
	build/mingw64/bin/luajit.exe
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw crown config=release64
mingw:                  \
	mingw-debug64       \
	mingw-development64 \
	mingw-release64

build/projects/vs2022:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools vs2022
	$(GENIE) --file=3rdparty/bimg/scripts/genie.lua --with-tools vs2022
	$(GENIE) --gfxapi=d3d11 --with-tools --no-editor vs2022
windows-debug64:          \
	build/projects/vs2022 \
	build/windows64/bin/luajit.exe
	devenv.com build/projects/vs2022/crown.sln $(ARG_PREFIX)Build "debug|x64" $(ARG_PREFIX)Project crown
windows-development64:    \
	build/projects/vs2022 \
	build/windows64/bin/luajit.exe
	devenv.com build/projects/vs2022/crown.sln $(ARG_PREFIX)Build "development|x64" $(ARG_PREFIX)Project crown
windows-release64:        \
	build/projects/vs2022 \
	build/windows64/bin/luajit.exe
	devenv.com build/projects/vs2022/crown.sln $(ARG_PREFIX)Build "release|x64" $(ARG_PREFIX)Project crown

crown-editor-theme:
	cd tools/level_editor/resources/theme/Adwaita && ./parse-sass.sh
	cd tools/level_editor/resources && ./generate-resources.sh > org.crownengine.Crown.gresource.xml

crown-editor-linux-debug64: \
	build/projects/linux
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux crown-editor config=debug64
crown-editor-linux-release64: \
	build/projects/linux
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux crown-editor config=release64

crown-editor-mingw-debug64: \
	build/projects/mingw
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw crown-editor config=debug64
crown-editor-mingw-release64: \
	build/projects/mingw
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw crown-editor config=release64

crown-launcher-linux-debug64: \
	build/projects/linux
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux crown-launcher config=debug64
crown-launcher-linux-release64: \
	build/projects/linux
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/linux crown-launcher config=release64

crown-launcher-mingw-debug64: \
	build/projects/mingw
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw crown-launcher config=debug64
crown-launcher-mingw-release64: \
	build/projects/mingw
	"$(MAKE)" -j$(MAKE_JOBS) -R -C build/projects/mingw crown-launcher config=release64

tools-linux-release32:       \
	build/linux32/bin/luajit \
	build/linux32/bin/luac
tools-linux-debug64:           \
	build/linux64/bin/texturec \
	build/linux64/bin/shaderc  \
	linux-debug64              \
	crown-editor-linux-debug64
tools-linux-release64:         \
	build/linux64/bin/texturec \
	build/linux64/bin/shaderc  \
	linux-development64        \
	crown-editor-linux-release64

tools-windows-debug64:               \
	build/windows64/bin/texturec.exe \
	build/windows64/bin/shaderc.exe  \
	windows-debug64
tools-windows-release64:             \
	build/windows64/bin/texturec.exe \
	build/windows64/bin/shaderc.exe  \
	windows-development64

tools-mingw-release32: \
	build/mingw32/bin/luac
tools-mingw-debug64:               \
	build/mingw64/bin/texturec.exe \
	build/mingw64/bin/shaderc.exe  \
	mingw-debug64                  \
	crown-editor-mingw-debug64
tools-mingw-release64:             \
	build/mingw64/bin/texturec.exe \
	build/mingw64/bin/shaderc.exe  \
	mingw-development64            \
	crown-editor-mingw-release64

.PHONY: docs
docs:
	"$(MAKE)" -C docs/ html

SAMPLES_PLATFORM=$(OS)

.PHONY: 00-empty
00-empty: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)
.PHONY: 01-physics
01-physics: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)
.PHONY: 02-animation
02-animation: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)
.PHONY: 03-joypad
03-joypad: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)

.PHONY: samples
samples: 00-empty 01-physics 02-animation 03-joypad

.PHONY: run-00-empty
run-00-empty: 00-empty
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-01-physics
run-01-physics: 01-physics
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-02-animation
run-02-animation: 02-animation
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-03-joypad
run-03-joypad: 03-joypad
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))

.PHONY: clean-samples
clean-samples:
	-@rm -rf samples/00-empty_*
	-@rm -rf samples/01-physics_*
	-@rm -rf samples/02-animation_*
	-@rm -rf samples/03-joypad_*

.PHONY: codespell
codespell:
	@codespell docs src tools \
		--ignore-words=scripts/codespell-dictionary.txt \
		--skip "*.ttf.h,*.png,docs/_themes,tools/level_editor/resources/theme/Adwaita" \
		-q4 # 4: omit warnings about automatic fixes that were disabled in the dictionary.

.PHONY: cppcheck
cppcheck:
	@cppcheck src \
		--includes-file=scripts/cppcheck/includes.txt \
		--suppressions-list=scripts/cppcheck/suppressions.txt \
		--enable=all \
		--quiet \
		--force # Check all configurations.

.PHONY: format-sources
format-sources:
	@scripts/uncrustify/format-all.sh -j $(MAKE_JOBS) --enable-tools

.PHONY: clean
clean: clean-samples
	@echo Cleaning...
ifeq ($(OS), linux)
	-@"$(MAKE)" -R -C 3rdparty/luajit/src clean -s
endif
	-@rm -rf 3rdparty/bgfx/.build
	-@rm -rf 3rdparty/bimg/.build
	-@rm -rf build
