crown - Lightweight cross-platform game engine that tries to Keep It Simple.
=====

##Dependencies

Crown Engine depends on a number of third-party libraries.
Some of them, like luajit, are included in this SDK, so you do not need to worry too much.
However, some other libraries are not included due to their excessive size or for some other particular reasons.
In order to be able to successfully build the engine, you have to install and/or properly configure the following packages:

Linux:

* zlib
* GLEW
* Xlib
* Xrandr
* nVidia PhysX 3.3.0+

Android:

* Android NDK and SDK
* nVidia PhysX 3.3.0+

Windows:

* zlib
* GLEW
* nVidia PhysX 3.3.0+

##Building

The steps required for building the engine in a default and pretty safe manner are summarized below for each supported platform.
All the commands and/or operations must be performed in the root directory of the project (i.e. the same as this file):

Linux:

	$ mkdir build
	$ cd build
	$ cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/your/install/dir -DCROWN_BUILD=<build>
		   (read below for valid build strings)
	$ make
	$ make install

'CROWN_BUILD' valid values are:

* linux-debug-32
* linux-development-32
* linux-release-32
* linux-debug-64
* linux-development-64
* linux-release-64

Windows:

	Create a folder named 'build'
	Open a terminal and:
	cd build
	cmake.exe .. -DCMAKE_INSTALL_PREFIX=C:/your/install/dir -DCROWN_BUILD=<build>
	Open the generated Visual Studio solution and build/install from there

'CROWN_BUILD' valid values are:

* windows-debug-32
* windows-development-32
* windows-release-32
* windows-debug-64
* windows-development-64
* windows-release-64

Android:

	$ cd utils
	$ ruby crown-android.rb --target <android-target> --name <project-name> --path <project-path>

You can also use the GUI front-end of CMake which can, less or more, simplify the overall 'complexity' of building from the terminal.

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
