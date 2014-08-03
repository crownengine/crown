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

linux-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=linux-gcc gmake
linux-debug32: luajit-linux32 linux-build
	make -R -C .build/linux config=debug32
linux-development32: luajit-linux32 linux-build
	make -R -C .build/linux config=development32
linux-release32: luajit-linux32 linux-build
	make -R -C .build/linux config=release32
linux-debug64: luajit-linux64 linux-build
	make -R -C .build/linux config=debug64
linux-development64: luajit-linux64 linux-build	
	make -R -C .build/linux config=development64
linux-release64: luajit-linux64 linux-build
	make -R -C .build/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64


android-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=android gmake
android-debug: luajit-arm android-build
	make -R -C .build/android config=debug
android-development: luajit-arm android-build
	make -R -C .build/android config=development
android-release: luajit-arm android-build
	make -R -C .build/android config=release
android: android-debug android-development android-release

windows-build:
	$(PREMAKE) --file=premake\premake4.lua vs2008
windows-debug32: windows-build
	devenv .build/windows/crown.sln /Build "debug|x32"
windows-development32: windows-build
	devenv .build/windows/crown.sln /Build "development|x32"
windows-release32: windows-build
	devenv .build/windows/crown.sln /Build "release|x32"
windows-debug64: windows-build
	devenv .build/windows/crown.sln /Build "debug|x64"
windows-development64: windows-build
	devenv .build/windows/crown.sln /Build "development|x64"
windows-release64: windows-build
	devenv .build/windows/crown.sln /Build "release|x64"

docs:
	doxygen Doxyfile.doxygen

clean: luajit-clean
	@echo Cleaning...
	@rm -rf .build
	@rm -rf .installation


starter:	
	xbuild /p:OutputPath=$(CROWN_INSTALL_DIR)/tools tools/gui/starter/starter.sln
console:
	xbuild /p:OutputPath=$(CROWN_INSTALL_DIR)/tools tools/gui/console/console.sln
tools: starter console

