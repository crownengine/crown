UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
else
	OS=windows
endif

PREMAKE=bin/$(OS)/premake4

luajit-linux32:
	make -R -C third/luajit CC="gcc -m32" BUILDMODE="static"
luajit-linux64:
	make -R -C third/luajit CC="gcc -m64" BUILDMODE="static"
luajit-windows32:
	cd third/luajit/src && msvcbuild
luajit-windows64:
	cd third/luajit/src && msvcbuild
luajit-arm:
	make -R -C third/luajit HOST_CC="gcc -m32" \
	CROSS=$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi- \
	TARGET_FLAGS="--sysroot $(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm \
		-march=armv7-a -mfloat-abi=softfp -Wl,--fix-cortex-a8"
luajit-clean:
	make -R -C third/luajit clean

bgfx-linux-debug32:
	make -R -C third/bgfx linux-debug32
bgfx-linux-debug64:
	make -R -C third/bgfx linux-debug64
bgfx-linux-release32:
	make -R -C third/bgfx linux-release32
bgfx-linux-release64:
	make -R -C third/bgfx linux-release64
bgfx-vs2012-debug32:
	make -R -C third/bgfx && make -R -C third/bgfx .build/projects/vs2012
bgfx-vs2012-release32:
	make -R -C third/bgfx && make -R -C third/bgfx .build/projects/vs2012
bgfx-vs2012-debug64:
	make -R -C third/bgfx && make -R -C third/bgfx .build/projects/vs2012
bgfx-vs2012-release64:
	make -R -C third/bgfx && make -R -C third/bgfx .build/projects/vs2012
bgfx-android-arm-debug:
	make -R -C third/bgfx android-arm-debug
bgfx-android-arm-release:
	make -R -C third/bgfx android-arm-release
bgfx-clean:
	make -R -C third/bgfx clean

deps-linux-debug32: luajit-linux32 bgfx-linux-debug32
deps-linux-debug64: luajit-linux64 bgfx-linux-debug64
deps-linux-release32: luajit-linux32 bgfx-linux-release32
deps-linux-release64: luajit-linux64 bgfx-linux-release64
deps-windows-debug32: luajit-windows32 bgfx-vs2012-debug32
deps-windows-debug64: luajit-windows64 bgfx-vs2012-debug64
deps-windows-release32: luajit-windows32 bgfx-vs2012-release32
deps-windows-release64: luajit-windows64 bgfx-vs2012-release64
deps-android-arm-debug: luajit-arm bgfx-android-arm-debug
deps-android-arm-release: luajit-arm bgfx-android-arm-release
deps-clean: luajit-clean bgfx-clean

linux-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=linux-gcc gmake
linux-debug32: deps-linux-debug32 linux-build
	make -R -C .build/projects/linux config=debug32
linux-development32: deps-linux-debug32 linux-build
	make -R -C .build/projects/linux config=development32
linux-release32: deps-linux-release32 linux-build
	make -R -C .build/projects/linux config=release32
linux-debug64: deps-linux-debug64 linux-build
	make -R -C .build/projects/linux config=debug64
linux-development64: deps-linux-debug64 linux-build	
	make -R -C .build/projects/linux config=development64
linux-release64: deps-linux-release64 linux-build
	make -R -C .build/projects/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64

android-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=android-arm gmake
android-arm-debug: deps-android-arm-debug android-build
	make -R -C .build/projects/android config=debug
android-arm-development: deps-android-arm-debug android-build
	make -R -C .build/projects/android config=development
android-arm-release: deps-android-arm-release android-build
	make -R -C .build/projects/android config=release
android-arm: android-arm-debug android-arm-development android-arm-release

windows-build:
	$(PREMAKE) --file=premake\premake4.lua vs2012
windows-debug32: deps-windows-debug32 windows-build
	devenv .build/windows/crown.sln /Build "debug|x32"
windows-development32: deps-windows-debug32 windows-build
	devenv .build/windows/crown.sln /Build "development|x32"
windows-release32: deps-windows-release32 windows-build
	devenv .build/windows/crown.sln /Build "release|x32"
windows-debug64: deps-windows-debug64 windows-build
	devenv .build/windows/crown.sln /Build "debug|x64"
windows-development64: deps-windows-debug64 windows-build
	devenv .build/windows/crown.sln /Build "development|x64"
windows-release64: deps-windows-release64 windows-build
	devenv .build/windows/crown.sln /Build "release|x64"

docs:
	doxygen documentation/Doxyfile.doxygen
	rst2html2 --stylesheet=html4css1.css,documentation/style.css documentation/lua_api.txt .build/documentation/lua_api.html

clean: deps-clean
	@echo Cleaning...
	@rm -rf .build
