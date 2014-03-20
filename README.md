crown - Lightweight, flexible, cross-platform game engine that tries to Keep It Simple.
=====

1. Dependencies

	Crown Engine depends on a number of third-party libraries.
	Some of them, like luajit, are included in this SDK, so you
	do not need to worry too much.

	However, some other libraries are not included due to their
	excessive size or for some other particular reasons.
	In order to be able to successfully build the engine, you
	have to install and/or properly configure the following packages:

	- Linux

		* zlib
		* GLEW
		* Xlib
		* Xrandr
		* nVidia PhysX

	- Android

		* Android NDK and SDK
		* nVidia PhysX

	- Windows:

		* zlib
		* GLEW
		* nVidia PhysX

2. Building

	The steps required for building the engine in a default and pretty
	safe manner are summarized below for each supported platform.

	All the commands and/or operations must be performed in the root
	directory of the project (i.e. the same as this file):

	- Linux

		1. $ mkdir build
		2. $ cd build
		3. $ cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/your/install/dir -DCROWN_BUILD=<build>
		   (read below for valid build strings)
		4. $ make
		5. $ make install

		'CROWN_BUILD' valid values right now are:

 			* linux-debug-32
 			* linux-development-32
 			* linux-release-32
 			* linux-debug-64
 			* linux-development-64
 			* linux-release-64


	- Windows

		1. Create a folder named 'build'
		2. Open a terminal and:
		3. cd build
		4. cmake.exe .. -DCMAKE_INSTALL_PREFIX=C:/your/install/dir -DCROWN_BUILD=<build>
		5. Open the generated Visual Studio solution and build/install from there

		'CROWN_BUILD' valid values right now are:

 			* windows-debug-32
 			* windows-development-32
 			* windows-release-32
 			* windows-debug-64
 			* windows-development-64
 			* windows-release-64


	- Android

		1. $ cd utils
		2. $ ruby crown-android.rb --target <android-target> --name <project-name> --path <project-path>

	You can also use the GUI front-end of CMake which can, less or more, simplify
	the overall 'complexity' of building from the terminal.

 3. System requirements

 	In order to be able to run the engine, no particular configurations are needed:

 	- Linux

 		* 32/64-bit distribution
 		* x86/x86_64 CPU
 		* The latest driver for your GPU (OpenGL >= 2.1)

 	- Windows

 		* 64-bit Windows version (Windows 7+)
 		* x86_64 CPU
 		* The latest driver for your GPU (OpenGL >= 2.1)

 	- Android

 		* Android 2.3.3+
 		* ARMv7 CPU
 		* OpenGL|ES 2 compatible GPU

 4. Known issues

 	- Linux

 		* Some open-source GPU drivers, like the Intel one, make call to 'operator new'
 		  and thus do not work at the moment with Crown Engine.
