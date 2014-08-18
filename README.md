Lightweight, cross-platform game engine that tries to Keep It Simple.
=====

##What is it?

Lightweight cross-platform game engine.
You can write games in Lua and/or C++ for Linux, Android and Windows with it.

##Y u do dis, there's UE4 for 19$ a month.

Because it's fun.

##Dependencies

Crown Engine depends on a number of third-party libraries.
Some of them, like luajit, are included in this SDK, so you do not need to worry too much.
However, some other libraries are not included due to their excessive size or for some other particular reasons.
In order to be able to successfully build the engine, you have to install and/or properly configure the following packages:

Linux:

* Xlib
* Xrandr
* nVidia PhysX 3.3.0+

Windows:

* nVidia PhysX 3.3.0+
* GnuWin32 make (http://gnuwin32.sourceforge.net/packages/make.htm)
* GnuWin32 coreutils (http://gnuwin32.sourceforge.net/packages/coreutils.htm)
* GnuWin32 libiconv (http://gnuwin32.sourceforge.net/packages/libiconv.htm)
* GnuWin32 libintl (http://gnuwin32.sourceforge.net/packages/libintl.htm)

Android:

* Android NDK and SDK
* nVidia PhysX 3.3.0+

##Building

The steps required for building the engine in a default and pretty safe manner are summarized below for each supported platform.
All the commands and/or operations must be performed in the root directory of the project (i.e. the same as this file):

Linux:

	$ make <CROWN_BUILD> 

'CROWN_BUILD' valid values are:

* linux-debug32
* linux-development32
* linux-release32
* linux-debug64
* linux-development64
* linux-release64
* linux

Windows:

	make.exe <CROWN_BUILD>
	Open the generated Visual Studio solution and build/install from there

'CROWN_BUILD' valid values are:

* windows-debug32
* windows-development32
* windows-release32
* windows-debug64
* windows-development64
* windows-release64

Android:

	$ cd utils
	$ ruby crown-android.rb --target <android-target> --name <project-name> --path <project-path> [--res <res-path>]


##System requirements

 In order to be able to run the engine, no particular configurations are needed:

Linux:

* 32/64-bit distribution
* x86/x86_64 CPU
* The latest driver for your GPU (OpenGL >= 2.1)

Windows:

* 64-bit Windows version (Windows 7+)
* x86_64 CPU
* The latest driver for your GPU (OpenGL >= 2.1)

Android:

* Android 2.3.3+
* ARMv7 CPU
* OpenGL|ES 2 compatible GPU

##Examples

When running a sample you current directory has to be `<install-path>/bin`.
To run a sample:

	$ ./crown-linux-debug-64 --source-dir <full-sample-path> --bundle-dir <full-destination-path> --compile --continue

###[01.hello-world](https://github.com/taylor001/crown/tree/master/samples/01.hello-world)

Engine initialization and shutdown.

