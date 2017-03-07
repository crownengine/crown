#
# Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
# License: https://github.com/taylor001/crown/blob/master/LICENSE
#

UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
else
	OS=windows
endif

GENIE=3rdparty/bx/tools/bin/$(OS)/genie

build/engine/projects/android:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-openal --with-bullet --compiler=android-arm gmake
engine-android-arm-debug: build/engine/projects/android
	make -R -C build/engine/projects/android config=debug
engine-android-arm-development: build/engine/projects/android
	make -R -C build/engine/projects/android config=development
engine-android-arm-release: build/engine/projects/android
	make -R -C build/engine/projects/android config=release
engine-android-arm: engine-android-arm-debug engine-android-arm-development engine-android-arm-release

build/engine/projects/linux:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-openal --with-bullet --with-tools --compiler=linux-gcc gmake
engine-linux-debug32: build/engine/projects/linux
	make -R -C build/engine/projects/linux config=debug32
engine-linux-development32: build/engine/projects/linux
	make -R -C build/engine/projects/linux config=development32
engine-linux-release32: build/engine/projects/linux
	make -R -C build/engine/projects/linux config=release32
engine-linux-debug64: build/engine/projects/linux
	make -R -C build/engine/projects/linux config=debug64
engine-linux-development64: build/engine/projects/linux
	make -R -C build/engine/projects/linux config=development64
engine-linux-release64: build/engine/projects/linux
	make -R -C build/engine/projects/linux config=release64
engine-linux: engine-linux-debug32 engine-linux-development32 engine-linux-release32 engine-linux-debug64 engine-linux-development64 engine-linux-release64

build/engine/projects/vs2013:
	$(GENIE) --file=scripts\genie.lua --with-luajit --with-openal --with-bullet --with-tools vs2013
engine-windows-debug32: build/engine/projects/vs2013
	devenv build/engine/projects/vs2013/crown.sln /Build "debug|Win32"
engine-windows-development32: build/engine/projects/vs2013
	devenv build/engine/projects/vs2013/crown.sln /Build "development|Win32"
engine-windows-release32: build/engine/projects/vs2013
	devenv build/engine/projects/vs2013/crown.sln /Build "release|Win32"
engine-windows-debug64: build/engine/projects/vs2013
	devenv build/engine/projects/vs2013/crown.sln /Build "debug|x64"
engine-windows-development64: build/engine/projects/vs2013
	devenv build/engine/projects/vs2013/crown.sln /Build "development|x64"
engine-windows-release64: build/engine/projects/vs2013
	devenv build/engine/projects/vs2013/crown.sln /Build "release|x64"

rebuild-glib-resources:
	make -R -C tools/ui rebuild

build/tools/projects/linux:
	$(GENIE) --file=scripts/tools.lua --compiler=linux-gcc gmake
tools-linux-debug64: build/tools/projects/linux engine-linux-development64
	make -R -C build/tools/projects/linux config=debug
tools-linux-release64: build/tools/projects/linux engine-linux-development64
	make -R -C build/tools/projects/linux config=release

build/tools/projects/mingw:
	$(GENIE) --file=scripts/tools.lua --compiler=mingw gmake
tools-mingw-debug64: build/tools/projects/mingw
	make -R -C build/tools/projects/mingw config=debug
tools-mingw-release64: build/tools/projects/mingw
	make -R -C build/tools/projects/mingw config=release

.PHONY: docs
docs:
	make -C docs/ html
	doxygen docs/doxygen/Doxyfile.doxygen

.PHONY: clean
clean:
	@echo Cleaning...
	-@rm -rf build
