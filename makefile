UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
else ifeq ($(UNAME), $(filter $(UNAME), Windows))
	OS=windows
endif

PREMAKE=third/premake/$(OS)/bin/premake4

linux-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=linux-gcc gmake
linux-debug32:	linux-build
	make -R -C .build/linux config=debug32
linux-development32:	linux-build
	make -R -C .build/linux config=development32
linux-release32:	linux-build
	make -R -C .build/linux config=release32
linux-debug64:	linux-build
	make -R -C .build/linux config=debug64
linux-development64:	linux-build	
	make -R -C .build/linux config=development64
linux-release64:	linux-build	
	make -R -C .build/linux config=release64
linux:	linux-debug32	linux-development32	linux-release32	linux-debug64	linux-development64	linux-release64

android-build:
	$(PREMAKE) --file=premake/premake4.lua --compiler=android gmake
android-debug:	android-build
	make -R -C .build/android config=debug
android-development:	android-build
	make -R -C .build/android config=development
android-release:	android-build
	make -R -C .build/android config=release
android:	android-debug	android-development	android-release

# docs:
# 	doxygen premake/crown.doxygen
# 	# markdown README.md > .build/docs/readme.html

clean:
	@echo Cleaning...
	-@rm -rf .build