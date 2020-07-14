#
# Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
# License: https://github.com/dbartolini/crown/blob/master/LICENSE
#

UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
else
	OS=windows
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

build/linux32/bin/luajit:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="gcc -m32" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Linux BUILDMODE=static
	mkdir -p build/linux32/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit 3rdparty/luajit/src/libluajit.a build/linux32/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean
build/linux64/bin/luajit:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Linux BUILDMODE=static
	mkdir -p build/linux64/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit 3rdparty/luajit/src/libluajit.a build/linux64/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/mingw32/bin/luajit.exe:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="$(MINGW)/bin/x86_64-w64-mingw32-gcc -m32" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Windows BUILDMODE=static
	mkdir -p build/mingw32/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/libluajit.a build/mingw32/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean
build/mingw64/bin/luajit.exe:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="$(MINGW)/bin/x86_64-w64-mingw32-gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Windows BUILDMODE=static
	mkdir -p build/mingw64/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/libluajit.a build/mingw64/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/win32/bin/luajit.exe:
	-mkdir "build/win32/bin"
	cd "3rdparty/luajit/src" && msvcbuild.bat
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/lua51.dll 3rdparty/luajit/src/lua51.lib build/win32/bin
	-@rm -f 3rdparty/luajit/src/buildvm.*
	-@rm -f 3rdparty/luajit/src/jit/vmdef.lua
	-@rm -f 3rdparty/luajit/src/lua51.*
	-@rm -f 3rdparty/luajit/src/luajit.exe
	-@rm -f 3rdparty/luajit/src/luajit.exp
	-@rm -f 3rdparty/luajit/src/luajit.lib
	-@rm -f 3rdparty/luajit/src/minilua.*
build/win64/bin/luajit.exe:
	-mkdir "build/win64/bin"
	cd "3rdparty/luajit/src" && msvcbuild.bat
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/lua51.dll 3rdparty/luajit/src/lua51.lib build/win64/bin
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
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/bgfx/.build/projects/gmake-linux config=release64 texturec
	cp -r 3rdparty/bgfx/.build/linux64_gcc/bin/texturecRelease $@
build/linux64/bin/shaderc:
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/bgfx/.build/projects/gmake-linux config=release64 shaderc
	cp -r 3rdparty/bgfx/.build/linux64_gcc/bin/shadercRelease $@

build/projects/linux:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools --gcc=linux-gcc gmake
	$(GENIE) --gfxapi=gl32 --with-luajit --with-tools --compiler=linux-gcc gmake
linux-debug32: build/projects/linux build/linux32/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=debug32
linux-development32: build/projects/linux build/linux32/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=development32
linux-release32: build/projects/linux build/linux32/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=release32
linux-debug64: build/projects/linux build/linux64/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=debug64
linux-development64: build/projects/linux build/linux64/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=development64
linux-release64: build/projects/linux build/linux64/bin/luajit
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64

build/projects/mingw:
	$(GENIE) --gfxapi=d3d11 --with-luajit --with-tools --compiler=mingw-gcc gmake
mingw-debug32: build/projects/mingw build/mingw32/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=debug32
mingw-development32: build/projects/mingw build/mingw32/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=development32
mingw-release32: build/projects/mingw build/mingw32/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=release32
mingw-debug64: build/projects/mingw build/mingw64/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=debug64
mingw-development64: build/projects/mingw build/mingw64/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=development64
mingw-release64: build/projects/mingw build/mingw64/bin/luajit.exe
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=release64
mingw: mingw-debug32 mingw-development32 mingw-release32 mingw-debug64 mingw-development64 mingw-release64

build/win64/bin/texturec.exe:
	devenv 3rdparty/bgfx/.build/projects/vs2017/bgfx.sln /Build "Release|x64" /Project texturec.vcxproj
	cp -r 3rdparty/bgfx/.build/win64_vs2017/bin/texturecRelease.exe $@
build/win64/bin/shaderc.exe:
	devenv 3rdparty/bgfx/.build/projects/vs2017/bgfx.sln /Build "Release|x64" /Project shaderc.vcxproj
	cp -r 3rdparty/bgfx/.build/win64_vs2017/bin/shadercRelease.exe $@

build/projects/vs2017:
	$(GENIE) --file=3rdparty\\bgfx\\scripts\\genie.lua --with-tools vs2017
	$(GENIE) --gfxapi=d3d11 --with-luajit --with-tools --no-level-editor vs2017
windows-debug32: build/projects/vs2017 build/win32/bin/luajit.exe
	devenv build/projects/vs2017/crown.sln /Build "debug|Win32"
windows-development32: build/projects/vs2017 build/win32/bin/luajit.exe
	devenv build/projects/vs2017/crown.sln /Build "development|Win32"
windows-release32: build/projects/vs2017 build/win32/bin/luajit.exe
	devenv build/projects/vs2017/crown.sln /Build "release|Win32"
windows-debug64: build/projects/vs2017 build/win64/bin/luajit.exe
	devenv build/projects/vs2017/crown.sln /Build "debug|x64"
windows-development64: build/projects/vs2017 build/win64/bin/luajit.exe
	devenv build/projects/vs2017/crown.sln /Build "development|x64"
windows-release64: build/projects/vs2017 build/win64/bin/luajit.exe
	devenv build/projects/vs2017/crown.sln /Build "release|x64"

.PHONY: rebuild-glib-resources
rebuild-glib-resources:
	$(MAKE) -j$(MAKE_JOBS) -R -C tools/level_editor/resources rebuild

tools-linux-debug64: linux-debug64 build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=debug
tools-linux-release64: linux-development64 build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=release

tools-windows-debug64: windows-debug64 build/win64/bin/texturec.exe build/win64/bin/shaderc.exe
tools-windows-release64: windows-development64 build/win64/bin/texturec.exe build/win64/bin/shaderc.exe

tools-mingw-debug64: mingw-development64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=debug
tools-mingw-release64: mingw-development64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=release

.PHONY: docs
docs:
	$(MAKE) -C docs/ html
	doxygen docs/doxygen/Doxyfile.doxygen

.PHONY: clean
clean:
	@echo Cleaning...
ifeq ($(OS), linux)
	-@$(MAKE) -R -C 3rdparty/luajit/src clean -s
endif
	-@rm -rf 3rdparty/bgfx/.build
	-@rm -rf build
