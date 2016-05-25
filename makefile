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

android-build:
	$(GENIE) --file=genie/genie.lua --with-luajit --with-openal --with-bullet --compiler=android-arm gmake
android-arm-debug: android-build
	make -R -C build/engine/projects/android config=debug
android-arm-development: android-build
	make -R -C build/engine/projects/android config=development
android-arm-release: android-build
	make -R -C build/engine/projects/android config=release
android-arm: android-arm-debug android-arm-development android-arm-release

linux-build:
	$(GENIE) --file=genie/genie.lua --with-luajit --with-openal --with-bullet --with-tools --compiler=linux-gcc gmake
linux-debug32: linux-build
	make -R -C build/engine/projects/linux config=debug32
linux-development32: linux-build
	make -R -C build/engine/projects/linux config=development32
linux-release32: linux-build
	make -R -C build/engine/projects/linux config=release32
linux-debug64: linux-build
	make -R -C build/engine/projects/linux config=debug64
linux-development64: linux-build
	make -R -C build/engine/projects/linux config=development64
linux-release64: linux-build
	make -R -C build/engine/projects/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64

windows-build:
	$(GENIE) --file=genie\genie.lua --with-luajit --with-openal --with-bullet --with-tools vs2013
windows-debug32: windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "debug|Win32"
windows-development32: windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "development|Win32"
windows-release32: windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "release|Win32"
windows-debug64: windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "debug|x64"
windows-development64: windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "development|x64"
windows-release64: windows-build
	devenv build/engine/projects/vs2013/crown.sln /Build "release|x64"

tools-build:
	$(GENIE) --file=genie/tools.lua gmake
tools-debug64: tools-build
	make -R -C build/projects config=debug
	cp -r tools/ui build/tools
tools-release64: tools-build
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
