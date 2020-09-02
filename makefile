#
# Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
# License: https://github.com/dbartolini/crown/blob/master/LICENSE
#

UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
	EXE_PREFIX=./
	EXE_SUFFIX=
else
	OS=windows
	EXE_PREFIX=
	EXE_SUFFIX=.exe
endif

GENIE=3rdparty/bx/tools/bin/$(OS)/genie
MAKE_JOBS=1

NDKABI=$(ANDROID_NDK_ABI)
NDKDIR=$(ANDROID_NDK_ROOT)
NDKBIN=$(NDKDIR)/toolchains/llvm/prebuilt/linux-x86_64/bin
NDKCROSS=$(NDKBIN)/arm-linux-androideabi-
NDKCC=$(NDKBIN)/armv7a-linux-androideabi$(NDKABI)-clang

build/android-arm/bin/libluajit.a:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src HOST_CC="gcc -m32" CROSS=$(NDKCROSS) STATIC_CC=$(NDKCC) DYNAMIC_CC="$(NDKCC) -fPIC" TARGET_LD=$(NDKCC)
	mkdir -p build/android-arm/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/libluajit.a build/android-arm/bin
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
	-mkdir "build/windows64/bin"
	cd "3rdparty/luajit/src" && msvcbuild.bat
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
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=debug64
linux-development64: build/projects/linux build/linux64/bin/luajit build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=development64
linux-release64: build/projects/linux build/linux64/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=release64
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
	devenv 3rdparty/bimg/.build/projects/vs2017/bimg.sln /Build "Release|x64" /Project texturec.vcxproj
	cp -r 3rdparty/bimg/.build/win64_vs2017/bin/texturecRelease.exe $@
build/windows64/bin/shaderc.exe:
	devenv 3rdparty/bgfx/.build/projects/vs2017/bgfx.sln /Build "Release|x64" /Project shaderc.vcxproj
	cp -r 3rdparty/bgfx/.build/win64_vs2017/bin/shadercRelease.exe $@

build/projects/vs2017:
	$(GENIE) --file=3rdparty\\bgfx\\scripts\\genie.lua --with-tools vs2017
	$(GENIE) --file=3rdparty\\bimg\\scripts\\genie.lua --with-tools vs2017
	$(GENIE) --gfxapi=d3d11 --with-luajit --with-tools --no-level-editor vs2017
windows-debug64: build/projects/vs2017 build/windows64/bin/luajit.exe build/windows64/bin/texturec.exe build/windows64/bin/shaderc.exe
	devenv build/projects/vs2017/crown.sln /Build "debug|x64"
windows-development64: build/projects/vs2017 build/windows64/bin/luajit.exe build/windows64/bin/texturec.exe build/windows64/bin/shaderc.exe
	devenv build/projects/vs2017/crown.sln /Build "development|x64"
windows-release64: build/projects/vs2017 build/windows64/bin/luajit.exe
	devenv build/projects/vs2017/crown.sln /Build "release|x64"

.PHONY: rebuild-glib-resources
rebuild-glib-resources:
	$(MAKE) -j$(MAKE_JOBS) -R -C tools/level_editor/resources rebuild

tools-linux-debug64: linux-debug64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=debug
tools-linux-release64: linux-development64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=release

tools-windows-debug64: windows-debug64
tools-windows-release64: windows-development64

tools-mingw-debug64: build/projects/mingw
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=debug
tools-mingw-release64: build/projects/mingw
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=release

.PHONY: docs
docs:
	$(MAKE) -C docs/ html
	doxygen docs/doxygen/Doxyfile.doxygen

.PHONY: 00-empty
00-empty: $(OS)-development64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --continue
.PHONY: 01-physics
01-physics: $(OS)-development64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --continue
.PHONY: 02-animation
02-animation: $(OS)-development64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --continue

.PHONY: clean-samples
clean-samples:
	-@rm -rf samples/00-empty_$(OS)
	-@rm -rf samples/01-physics_$(OS)
	-@rm -rf samples/02-animation_$(OS)

.PHONY: clean
clean: clean-samples
	@echo Cleaning...
ifeq ($(OS), linux)
	-@$(MAKE) -R -C 3rdparty/luajit/src clean -s
endif
	-@rm -rf 3rdparty/bgfx/.build
	-@rm -rf 3rdparty/bimg/.build
	-@rm -rf build
