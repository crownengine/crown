#
# Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
# License: https://github.com/taylor001/crown/blob/master/LICENSE
#

UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
else
	OS=windows
endif

GENIE=third/bx/tools/bin/$(OS)/genie

engine-android-build:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-openal --with-bullet --compiler=android-arm gmake
engine-android-arm-debug: engine-android-build
	make -R -C build/engine/projects/android config=debug
engine-android-arm-development: engine-android-build
	make -R -C build/engine/projects/android config=development
engine-android-arm-release: engine-android-build
	make -R -C build/engine/projects/android config=release
engine-android-arm: engine-android-arm-debug engine-android-arm-development engine-android-arm-release

engine-linux-build:
	$(GENIE) --file=scripts/genie.lua --with-luajit --with-openal --with-bullet --with-tools --compiler=linux-gcc gmake
engine-linux-debug32: engine-linux-build
	make -R -C build/engine/projects/linux config=debug32
engine-linux-development32: engine-linux-build
	make -R -C build/engine/projects/linux config=development32
engine-linux-release32: engine-linux-build
	make -R -C build/engine/projects/linux config=release32
engine-linux-debug64: engine-linux-build
	make -R -C build/engine/projects/linux config=debug64
engine-linux-development64: engine-linux-build
	make -R -C build/engine/projects/linux config=development64
engine-linux-release64: engine-linux-build
	make -R -C build/engine/projects/linux config=release64
engine-linux: engine-linux-debug32 engine-linux-development32 engine-linux-release32 engine-linux-debug64 engine-linux-development64 engine-linux-release64

engine-windows-build:
	$(GENIE) --file=scripts\genie.lua --with-luajit --with-openal --with-bullet --with-tools vs2013
engine-windows-debug32: engine-windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "debug|Win32"
engine-windows-development32: engine-windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "development|Win32"
engine-windows-release32: engine-windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "release|Win32"
engine-windows-debug64: engine-windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "debug|x64"
engine-windows-development64: engine-windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "development|x64"
engine-windows-release64: engine-windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "release|x64"

tools-build:
	$(GENIE) --file=scripts/tools.lua gmake
tools-linux-debug64: tools-build engine-linux-development64
	make -R -C build/projects config=debug
	cp -r tools/ui build/tools
tools-linux-release64: tools-build engine-linux-development64
	make -R -C build/projects config=release
	cp -r tools/ui build/tools

.PHONY: docs
docs:
	doxygen docs/doxygen/Doxyfile.doxygen
	rst2html2 --stylesheet=html4css1.css,docs/style.css docs/lua_api.txt build/docs/lua_api.html
	rst2html2 --stylesheet=html4css1.css,docs/style.css docs/manual.txt build/docs/manual.html

.PHONY: clean
clean:
	@echo Cleaning...
	-@rm -rf build
