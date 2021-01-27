#
# Copyright (c) 2012-2021 Daniele Bartolini et al.
# License: https://github.com/dbartolini/crown/blob/master/LICENSE
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

GENIE=3rdparty/bx/tools/bin/$(OS)/genie
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
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src HOST_CC="gcc -m32" CROSS=$(NDKCROSS) STATIC_CC=$(NDKCC) DYNAMIC_CC="$(NDKCC) -fPIC" TARGET_LD=$(NDKCC)
	mkdir -p build/android-arm/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/libluajit.a build/android-arm/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/android-arm64/bin/libluajit.a:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CROSS=$(NDKCROSS64) STATIC_CC=$(NDKCC64) DYNAMIC_CC="$(NDKCC64) -fPIC" TARGET_LD=$(NDKCC64)
	mkdir -p build/android-arm64/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/libluajit.a build/android-arm64/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/linux64/bin/luajit:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Linux BUILDMODE=static
	mkdir -p build/linux64/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit 3rdparty/luajit/src/libluajit.a build/linux64/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/mingw64/bin/luajit.exe:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="$(MINGW)/bin/x86_64-w64-mingw32-gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Windows BUILDMODE=static
	mkdir -p build/mingw64/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/libluajit.a build/mingw64/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/windows64/bin/luajit.exe:
	-$(MKDIR) "build/windows64/bin"
	cd "3rdparty/luajit/src" && .\\msvcbuild.bat
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/lua51.dll 3rdparty/luajit/src/lua51.lib build/windows64/bin
	-@rm -f 3rdparty/luajit/src/buildvm.*
	-@rm -f 3rdparty/luajit/src/jit/vmdef.lua
	-@rm -f 3rdparty/luajit/src/lua51.*
	-@rm -f 3rdparty/luajit/src/luajit.exe
	-@rm -f 3rdparty/luajit/src/luajit.exp
	-@rm -f 3rdparty/luajit/src/luajit.lib
	-@rm -f 3rdparty/luajit/src/minilua.*

build/projects/android-arm:
	$(GENIE) --gfxapi=gles2 --with-luajit --compiler=android-arm gmake
android-arm-debug: build/projects/android-arm build/android-arm/bin/libluajit.a
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android-arm config=debug
android-arm-development: build/projects/android-arm build/android-arm/bin/libluajit.a
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android-arm config=development
android-arm-release: build/projects/android-arm build/android-arm/bin/libluajit.a
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android-arm config=release
android-arm: android-arm-debug android-arm-development android-arm-release

build/projects/android-arm64:
	$(GENIE) --file=scripts/genie.lua --with-luajit --compiler=android-arm64 gmake
android-arm64-debug: build/projects/android-arm64 build/android-arm64/bin/libluajit.a
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android-arm64 config=debug
android-arm64-development: build/projects/android-arm64 build/android-arm64/bin/libluajit.a
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android-arm64 config=development
android-arm64-release: build/projects/android-arm64 build/android-arm64/bin/libluajit.a
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android-arm64 config=release
android-arm64: android-arm64-debug android-arm64-development android-arm64-release

build/linux64/bin/texturec:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/bimg/.build/projects/gmake-linux config=release64 texturec
	cp -r 3rdparty/bimg/.build/linux64_gcc/bin/texturecRelease $@
build/linux64/bin/shaderc:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/bgfx/.build/projects/gmake-linux config=release64 shaderc
	cp -r 3rdparty/bgfx/.build/linux64_gcc/bin/shadercRelease $@

build/projects/linux:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools --gcc=linux-gcc gmake
	$(GENIE) --file=3rdparty/bimg/scripts/genie.lua --with-tools --gcc=linux-gcc gmake
	$(GENIE) --gfxapi=gl32 --with-luajit --with-tools --compiler=linux-gcc gmake
linux-debug64: build/projects/linux build/linux64/bin/luajit build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux crown config=debug64
linux-development64: build/projects/linux build/linux64/bin/luajit build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux crown config=development64
linux-release64: build/projects/linux build/linux64/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux crown config=release64
linux: linux-debug64 linux-development64 linux-release64

build/projects/mingw:
	$(GENIE) --gfxapi=d3d11 --with-luajit --with-tools --compiler=mingw-gcc gmake
mingw-debug64: build/projects/mingw build/mingw64/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=debug64
mingw-development64: build/projects/mingw build/mingw64/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=development64
mingw-release64: build/projects/mingw build/mingw64/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=release64
mingw: mingw-debug64 mingw-development64 mingw-release64

build/windows64/bin/texturec.exe:
	devenv.com 3rdparty/bimg/.build/projects/vs2019/bimg.sln $(ARG_PREFIX)Build "Release|x64" $(ARG_PREFIX)Project texturec.vcxproj
	cp -r 3rdparty/bimg/.build/win64_vs2019/bin/texturecRelease.exe $@
build/windows64/bin/shaderc.exe:
	devenv.com 3rdparty/bgfx/.build/projects/vs2019/bgfx.sln $(ARG_PREFIX)Build "Release|x64" $(ARG_PREFIX)Project shaderc.vcxproj
	cp -r 3rdparty/bgfx/.build/win64_vs2019/bin/shadercRelease.exe $@

build/projects/vs2019:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools vs2019
	$(GENIE) --file=3rdparty/bimg/scripts/genie.lua --with-tools vs2019
	$(GENIE) --gfxapi=d3d11 --with-luajit --with-tools --no-level-editor vs2019
windows-debug64: build/projects/vs2019 build/windows64/bin/luajit.exe build/windows64/bin/texturec.exe build/windows64/bin/shaderc.exe
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "debug|x64"
windows-development64: build/projects/vs2019 build/windows64/bin/luajit.exe build/windows64/bin/texturec.exe build/windows64/bin/shaderc.exe
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "development|x64"
windows-release64: build/projects/vs2019 build/windows64/bin/luajit.exe
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "release|x64"

level-editor-linux-debug64:
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=debug64
level-editor-linux-release64:
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=release64

tools-linux-debug64: linux-debug64 level-editor-linux-debug64
tools-linux-release64: linux-development64 level-editor-linux-release64

tools-windows-debug64: windows-debug64
tools-windows-release64: windows-development64

level-editor-mingw-debug64: build/projects/mingw
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=debug
level-editor-mingw-release64: build/projects/mingw
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=release

tools-mingw-debug64: level-editor-mingw-debug64
tools-mingw-release64: level-editor-mingw-release64

.PHONY: docs
docs:
	$(MAKE) -C docs/ html
	doxygen docs/doxygen/Doxyfile.doxygen

.PHONY: 00-empty
00-empty: $(OS)-development64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile
.PHONY: 01-physics
01-physics: $(OS)-development64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile
.PHONY: 02-animation
02-animation: $(OS)-development64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile

.PHONY: samples
samples: 00-empty 01-physics 02-animation

.PHONY: run-00-empty
run-00-empty: 00-empty
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-01-physics
run-01-physics: 01-physics
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-02-animation
run-02-animation: 02-animation
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))

.PHONY: clean-samples
clean-samples:
	-@rm -rf samples/00-empty_$(OS)
	-@rm -rf samples/01-physics_$(OS)
	-@rm -rf samples/02-animation_$(OS)

.PHONY: codespell
codespell:
	codespell docs src tools tools-imgui --skip "Doxyfile.doxygen,*.ttf.h,*.png,docs/_themes"

.PHONY: clean
clean: clean-samples
	@echo Cleaning...
ifeq ($(OS), linux)
	-@$(MAKE) -R -C 3rdparty/luajit/src clean -s
endif
	-@rm -rf 3rdparty/bgfx/.build
	-@rm -rf 3rdparty/bimg/.build
	-@rm -rf build
