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

1. Install Android NDK (>= r23):

	* https://developer.android.com/ndk/downloads

2. Set NDK path and ABI version:

.. code::

	export ANDROID_NDK_ROOT=<path/to/android_ndk>
	export ANDROID_NDK_ABI=23

3. Continue to `Build`_.

Linux (Ubuntu >= 18.04)
-----------------------

.. code::

	sudo apt install build-essential mesa-common-dev libgl1-mesa-dev libpulse-dev libxrandr-dev

	sudo add-apt-repository ppa:vala-team
	sudo apt install libgtk-3-dev valac libgee-0.8-dev

If you encounter any errors while adding the Vala PPA, update ``ca-certificates`` first:

.. code::

	sudo apt update
	sudo apt install ca-certificates

To build documentation you will also need:

.. code::

	sudo apt-get install python3-sphinx

Windows (VS 2019)
-----------------

1. Install Visual Studio 2019:

	* https://visualstudio.microsoft.com/downloads

2. Continue to `Build`_.

Windows (VS Code)
-----------------

1. Install Visual Studio Code:

	* https://code.visualstudio.com

2. Continue to :ref:`Windows (MSYS2) <pre_windows_msys2>`.

.. _pre_windows_msys2:

Windows (MSYS2)
---------------

1. Install MSYS2:

	* https://www.msys2.org/

2. Add MSYS2 bin to your PATH; for a default install this would be:

.. code::

	C:\msys64\usr\bin

3. Open MSYS2 shell and install dependencies:

.. code::

	pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-pkgconf mingw-w64-x86_64-gtk3 mingw-w64-x86_64-vala mingw-w64-x86_64-libgee

4. Continue to `Build`_.

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

1. Open Visual Studio 2019 Command Prompt:

.. code::

	make tools-windows-release64

2. To build tools, open a MSYS2 MINGW64 shell:

.. code::

	export MINGW=/mingw64
	make tools-mingw-release64

Windows (VS Code)
-----------------

1. Add the MinGW compiler to your path:

	* https://code.visualstudio.com/docs/languages/cpp#_add-the-mingw-compiler-to-your-path

2. Open Visual Studio Code Shell:

.. code::

	$env:MINGW = "C:\msys64\mingw64"
	make tools-mingw-release64

Windows (MSYS2)
---------------

Open MSYS2 MINGW64 shell:

.. code::

	export MINGW=/mingw64
	make tools-mingw-release64
