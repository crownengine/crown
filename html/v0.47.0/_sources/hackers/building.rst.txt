========
Building
========

Getting Source
==============

.. code::

	git clone https://github.com/crownengine/crown.git

Prerequisites
=============

Android
-------

Android NDK:

	* https://developer.android.com/ndk/downloads

Set NDK path and ABI version (>= 23):

.. code::

	export ANDROID_NDK_ROOT=<path/to/android_ndk>
	export ANDROID_NDK_ABI=23

Linux (Ubuntu >= 18.04)
-----------------------

.. code::

	sudo apt-get install build-essential
	sudo apt-get install mesa-common-dev libgl1-mesa-dev libpulse-dev libxrandr-dev

	sudo add-apt-repository ppa:vala-team
	sudo apt-get install libgtk-3-dev valac libgee-0.8-dev

To build documentation you will also need:

.. code::

	sudo apt-get install python3-sphinx

Windows (VS 2019 or VS Code)
----------------------------

Install either Visual Studio 2019:

	* https://visualstudio.microsoft.com/downloads

or Visual Studio Code:

	* https://code.visualstudio.com

Install MSYS2:

	* https://www.msys2.org/

Add MSYS2 bin to your PATH; for a default install this would be:

.. code::

	C:\msys2\usr\bin

Open MSYS2 shell and type:

.. code::

	pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-pkg-config mingw-w64-x86_64-gtk3 mingw-w64-x86_64-vala mingw-w64-x86_64-libgee

Build
=====

Android
-------

.. code::

	make android-arm-release

Linux
-----

.. code::

	make tools-linux-release64

Windows (VS 2019)
-----------------

Open Visual Studio 2019 Command Prompt:

.. code::

	make tools-windows-release64

To build tools, open MSYS2 MSYS:

.. code::

	export MINGW=/mingw64
	make tools-mingw-release64

Windows (VS Code)
-----------------

Add the MinGW compiler to your path:

	* https://code.visualstudio.com/docs/languages/cpp#_add-the-mingw-compiler-to-your-path

Open Visual Studio Code Shell and set MINGW path:

.. code::

	$env:MINGW = "C:\msys64\mingw64"
	make tools-mingw-release64
