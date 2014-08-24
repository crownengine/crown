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
bgfx-vs2008-debug32:
	make -R -C third/bgfx && make -R -C third/bgfx vs2008-debug32
bgfx-vs2008-release32:
	make -R -C third/bgfx && make -R -C third/bgfx vs2008-release32
bgfx-vs2008-debug64:
	make -R -C third/bgfx && make -R -C third/bgfx vs2008-debug64
bgfx-vs2008-release64:
	make -R -C third/bgfx && make -R -C third/bgfx vs2008-release64
bgfx-android-arm:
	make -R -C third/bgfx android-arm-release
bgfx-clean:
	make -R -C third/bgfx clean

deps-linux-debug32: luajit-linux32 bgfx-linux-debug32
deps-linux-debug64: luajit-linux64 bgfx-linux-debug64
deps-linux-release32: luajit-linux32 bgfx-linux-release32
deps-linux-release64: luajit-linux64 bgfx-linux-release64
deps-windows-debug32: luajit-windows32 bgfx-vs2008-debug32
deps-windows-debug64: luajit-windows64 bgfx-vs2008-debug64
deps-windows-release32: luajit-windows32 bgfx-vs2008-release32
deps-windows-release64: luajit-windows64 bgfx-vs2008-release64
deps-android-arm: luajit-arm bgfx-android-arm
deps-clean: luajit-clean bgfx-clean

linux-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=linux-gcc gmake
linux-debug32: deps-linux-debug32 linux-build
	make -R -C .build/linux config=debug32
linux-development32: deps-linux-debug32 linux-build
	make -R -C .build/linux config=development32
linux-release32: deps-linux-release32 linux-build
	make -R -C .build/linux config=release32
linux-debug64: deps-linux-debug64 linux-build
	make -R -C .build/linux config=debug64
linux-development64: deps-linux-debug64 linux-build	
	make -R -C .build/linux config=development64
linux-release64: deps-linux-release64 linux-build
	make -R -C .build/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64


android-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=android gmake
android-debug: deps-android-arm android-build
	make -R -C .build/android config=debug
android-development: deps-android-arm android-build
	make -R -C .build/android config=development
android-release: deps-android-arm android-build
	make -R -C .build/android config=release
android: android-debug android-development android-release

windows-build:
	$(PREMAKE) --file=premake\premake4.lua vs2012
windows-debug32: windows-build
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
	doxygen Doxyfile.doxygen

clean: deps-clean
	@echo Cleaning...
	@rm -rf .build
	@rm -rf .installation

starter:	
	xbuild /p:OutputPath=$(CROWN_INSTALL_DIR)/tools tools/gui/starter/starter.sln
console:
	xbuild /p:OutputPath=$(CROWN_INSTALL_DIR)/tools tools/gui/console/console.sln
tools: starter console

