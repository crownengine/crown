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

build/projects/android:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-openal --with-bullet --compiler=android-arm gmake
android-arm-debug: build/projects/android
	make -R -C build/projects/android config=debug
android-arm-development: build/projects/android
	make -R -C build/projects/android config=development
android-arm-release: build/projects/android
	make -R -C build/projects/android config=release
android-arm: android-arm-debug android-arm-development android-arm-release

build/projects/linux:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-openal --with-bullet --with-tools --compiler=linux-gcc gmake
linux-debug32: build/projects/linux
	make -R -C build/projects/linux config=debug32
linux-development32: build/projects/linux
	make -R -C build/projects/linux config=development32
linux-release32: build/projects/linux
	make -R -C build/projects/linux config=release32
linux-debug64: build/projects/linux
	make -R -C build/projects/linux config=debug64
linux-development64: build/projects/linux
	make -R -C build/projects/linux config=development64
linux-release64: build/projects/linux
	make -R -C build/projects/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64

build/projects/vs2013:
	$(GENIE) --file=scripts\genie.lua --with-luajit --with-openal --with-bullet --with-tools vs2013
windows-debug32: build/projects/vs2013
	devenv build/projects/vs2013/crown.sln /Build "debug|Win32"
windows-development32: build/projects/vs2013
	devenv build/projects/vs2013/crown.sln /Build "development|Win32"
windows-release32: build/projects/vs2013
	devenv build/projects/vs2013/crown.sln /Build "release|Win32"
windows-debug64: build/projects/vs2013
	devenv build/projects/vs2013/crown.sln /Build "debug|x64"
windows-development64: build/projects/vs2013
	devenv build/projects/vs2013/crown.sln /Build "development|x64"
windows-release64: build/projects/vs2013
	devenv build/projects/vs2013/crown.sln /Build "release|x64"

rebuild-glib-resources:
	make -R -C tools/ui rebuild

tools-linux-debug64: linux-development64
	make -R -C build/projects/linux level-editor config=debug
tools-linux-release64: linux-development64
	make -R -C build/projects/linux level-editor config=release

tools-mingw-debug64: mingw-development64
	make -R -C build/projects/mingw level-editor config=debug
tools-mingw-release64: mingw-development64
	make -R -C build/projects/mingw level-editor config=release

.PHONY: docs
docs:
	make -C docs/ html
	doxygen docs/doxygen/Doxyfile.doxygen

.PHONY: clean
clean:
	@echo Cleaning...
	-@rm -rf build
