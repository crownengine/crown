#
# Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
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

NDKABI=14
NDKCC=$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-
NDKFLAGS=--sysroot $(ANDROID_NDK_ROOT)/platforms/android-$(NDKABI)/arch-arm
NDKARCH=-march=armv7-a -mfloat-abi=softfp -mfpu=neon -mthumb -Wl,--fix-cortex-a8

build/android-arm/bin/libluajit.a:
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src HOST_CC="gcc -m32" CROSS=$(NDKCC) TARGET_FLAGS="$(NDKFLAGS) $(NDKARCH)"
	mkdir -p build/android-arm/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/libluajit.a build/android-arm/bin
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/linux32/bin/luajit:
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="gcc -m32" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Linux BUILDMODE=static
	mkdir -p build/linux32/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit 3rdparty/luajit/src/libluajit.a build/linux32/bin
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean
build/linux64/bin/luajit:
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Linux BUILDMODE=static
	mkdir -p build/linux64/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit 3rdparty/luajit/src/libluajit.a build/linux64/bin
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/mingw32/bin/luajit.exe:
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="$(MINGW)/bin/x86_64-w64-mingw32-gcc -m32" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Windows BUILDMODE=static
	mkdir -p build/mingw32/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/libluajit.a build/mingw32/bin
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean
build/mingw64/bin/luajit.exe:
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src CC="$(MINGW)/bin/x86_64-w64-mingw32-gcc -m64" CCOPT="-O2 -fomit-frame-pointer -msse2" TARGET_SYS=Windows BUILDMODE=static
	mkdir -p build/mingw64/bin
	cp -r 3rdparty/luajit/src/jit 3rdparty/luajit/src/luajit.exe 3rdparty/luajit/src/libluajit.a build/mingw64/bin
	make -j$(MAKE_JOBS) -R -C 3rdparty/luajit/src clean

build/win32/bin/luajit.exe:
	mkdir build\win32\bin
	cp -r 3rdparty\luajit\src\jit 3rdparty\luajit\pre\win_x32\luajit.exe 3rdparty\luajit\pre\win_x32\lua51.dll build/win32/bin
build/win64/bin/luajit.exe:
	mkdir build\win64\bin
	cp -r 3rdparty\luajit\src\jit 3rdparty\luajit\pre\win_x64\luajit.exe 3rdparty\luajit\pre\win_x64\lua51.dll build/win64/bin

build/projects/android:
	$(GENIE) --file=scripts/genie.lua --with-luajit --compiler=android-arm gmake
android-arm-debug: build/projects/android build/android-arm/bin/libluajit.a
	make -j$(MAKE_JOBS) -R -C build/projects/android config=debug
android-arm-development: build/projects/android build/android-arm/bin/libluajit.a
	make -j$(MAKE_JOBS) -R -C build/projects/android config=development
android-arm-release: build/projects/android build/android-arm/bin/libluajit.a
	make -j$(MAKE_JOBS) -R -C build/projects/android config=release
android-arm: android-arm-debug android-arm-development android-arm-release

build/projects/linux:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-tools --compiler=linux-gcc gmake
linux-debug32: build/projects/linux build/linux32/bin/luajit
	make -j$(MAKE_JOBS) -R -C build/projects/linux config=debug32
linux-development32: build/projects/linux build/linux32/bin/luajit
	make -j$(MAKE_JOBS) -R -C build/projects/linux config=development32
linux-release32: build/projects/linux build/linux32/bin/luajit
	make -j$(MAKE_JOBS) -R -C build/projects/linux config=release32
linux-debug64: build/projects/linux build/linux64/bin/luajit
	make -j$(MAKE_JOBS) -R -C build/projects/linux config=debug64
linux-development64: build/projects/linux build/linux64/bin/luajit
	make -j$(MAKE_JOBS) -R -C build/projects/linux config=development64
linux-release64: build/projects/linux build/linux64/bin/luajit
	make -j$(MAKE_JOBS) -R -C build/projects/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64

build/projects/mingw:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-tools --compiler=mingw-gcc gmake
mingw-debug32: build/projects/mingw build/mingw32/bin/luajit.exe
	make -j$(MAKE_JOBS) -R -C build/projects/mingw config=debug32
mingw-development32: build/projects/mingw build/mingw32/bin/luajit.exe
	make -j$(MAKE_JOBS) -R -C build/projects/mingw config=development32
mingw-release32: build/projects/mingw build/mingw32/bin/luajit.exe
	make -j$(MAKE_JOBS) -R -C build/projects/mingw config=release32
mingw-debug64: build/projects/mingw build/mingw64/bin/luajit.exe
	make -j$(MAKE_JOBS) -R -C build/projects/mingw config=debug64
mingw-development64: build/projects/mingw build/mingw64/bin/luajit.exe
	make -j$(MAKE_JOBS) -R -C build/projects/mingw config=development64
mingw-release64: build/projects/mingw build/mingw64/bin/luajit.exe
	make -j$(MAKE_JOBS) -R -C build/projects/mingw config=release64
mingw: mingw-debug32 mingw-development32 mingw-release32 mingw-debug64 mingw-development64 mingw-release64

build/projects/vs2017:
	$(GENIE) --file=scripts\genie.lua --with-luajit --with-tools --no-level-editor vs2017
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
	make -j$(MAKE_JOBS) -R -C tools rebuild

tools-linux-debug64: linux-development64
	make -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=debug
tools-linux-release64: linux-development64
	make -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=release

tools-mingw-debug64: mingw-development64
	make -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=debug
tools-mingw-release64: mingw-development64
	make -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=release

.PHONY: docs
docs:
	make -C docs/ html
	doxygen docs/doxygen/Doxyfile.doxygen

.PHONY: clean
clean:
	@echo Cleaning...
	-@rm -rf build
	-@make -R -C 3rdparty/luajit/src clean -s
