Lightweight and flexible cross-platform game engine.
=====

##What is it?

It is a general purpose data-driven game engine, written from scratch with a minimalistic and data-oriented design philosophy in mind.

##Screenshots

TCP/IP console with autocomplete and color-coded output highlighting.

![console](https://raw.githubusercontent.com/taylor001/crown/master/shots/console.png)

[WIP] Node editor.

![node-editor](https://raw.githubusercontent.com/taylor001/crown/master/shots/node-editor.png)

##Dependencies

PhysX 3.3.x (https://developer.nvidia.com/physx-sdk)

##Building

### Getting source

	$ git clone https://github.com/taylor001/crown.git
	$ cd crown
	$ git submodule init
	$ git submodule update

### Prerequisites

### Prerequisites for Android

Android NDK (https://developer.android.com/tools/sdk/ndk/index.html)

	$ export ANDROID_NDK_ROOT=<path/to/android_ndk>
	$ export ANDROID_NDK_ARM=<path/to/android_ndk_arm>
	$ export PHYSX_SDK_ANDROID=<path/to/physx_sdk>
	
### Prerequisites for Linux

	$ export PHYSX_SDK_LINUX=<path/to/physx_sdk>

### Prerequisites for Windows

GnuWin32 make (http://gnuwin32.sourceforge.net/packages/make.htm)  
GnuWin32 coreutils (http://gnuwin32.sourceforge.net/packages/coreutils.htm)  
GnuWin32 libiconv (http://gnuwin32.sourceforge.net/packages/libiconv.htm)  
GnuWin32 libintl (http://gnuwin32.sourceforge.net/packages/libintl.htm)

	$ setx PHYSX_SDK_WINDOWS <path/to/physx_sdk>
	$ setx DXSDK_DIR <path/to/dxsdk>
	
### Building

	$ make <configuration>
	
Configuration is `<platform>-<debug/development/release>[32|64]`. E.g.

	linux-debug64, linux-development64, linux-release64, android-debug, windows-debug32 etc.
## Documentation

There is pretty extensive documentation for both C++ and Lua API.

### Prerequisites

Doxygen (http://www.doxygen.org)  
Docutils/reStructuredText (http://docutils.sourceforge.net)

### Building

	$ make docs
	
##Samples

When running a sample your current directory has to be `<install-path>/bin/<platform[32|64]>`.
To run a sample on 64-bit linux debug mode:

	$ ./linux-debug-64 --source-dir <full/sample/path> --bundle-dir <full/destination/path> --compile --platform linux --continue

###[01.hello-world](https://github.com/taylor001/crown/tree/master/samples/01.hello-world)

Engine initialization and shutdown.

##Tools

A number of tools can be found at https://github.com/taylor001/crown-tools

