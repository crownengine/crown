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

1. Install Android NDK (>= r21e LTS):

	* https://developer.android.com/ndk/downloads

2. Set NDK path and ABI version:

.. code::

	export ANDROID_NDK_ROOT=<android_ndk>
	export ANDROID_NDK_ABI=23

3. Install LuaJIT dependencies:

.. code::

	sudo apt-get install libc6-dev-i386

4. :ref:`Build for Android<build_android>`.

HTML5
-----

1. Install emsdk:

   * https://emscripten.org/docs/getting_started/downloads.html

2. Set emsdk path:

.. code::

	export EMSCRIPTEN=<emsdk>/upstream/emscripten

3. :ref:`Build for HTML5<build_html5>`.

Linux (Ubuntu >= 20.04)
-----------------------

1. Install dependencies:

.. code::

	sudo apt install build-essential mesa-common-dev libgl1-mesa-dev \
	libpulse-dev libxrandr-dev libc6-dev-i386 libgtk-3-dev

	sudo add-apt-repository ppa:vala-team
	sudo apt install valac libgee-0.8-dev

	# Optionally, to build docs:
	sudo apt-get install python3-sphinx

.. note::

	If you encounter any errors while adding the Vala PPA, please update
	``ca-certificates`` first:

	.. code::

		sudo apt update
		sudo apt install ca-certificates

2. :ref:`Build for Linux<build_linux>`.

.. _pre_windows_msys2:

Windows (MSYS2)
---------------

1. Install MSYS2:

	* https://www.msys2.org/

2. Add MSYS2 bin to your PATH; for a default install this would be:

.. code::

	C:\msys64\usr\bin

3. Open MSYS2 MINGW64 shell and install dependencies:

.. code::

	pacman -Sy make mingw-w64-x86_64-gcc mingw-w64-x86_64-pkgconf \
	mingw-w64-x86_64-gtk3 mingw-w64-x86_64-sassc \
	mingw-w64-x86_64-vala mingw-w64-x86_64-libgee

4. :ref:`Build for Windows (MSYS2)<build_msys2>`.

Windows (VS Code)
-----------------

1. Install Visual Studio Code:

	* https://code.visualstudio.com

2. Install C/C++ extension for VS Code:

   * https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

3. :ref:`Install MSYS2 <pre_windows_msys2>`.

4. :ref:`Build for Windows (VS Code) <build_vs_code>`.

Windows (VS 2022)
-----------------

1. Install Visual Studio 2022:

	* https://visualstudio.microsoft.com/downloads

2. If you want to build tools, :ref:`install Windows (MSYS2) <pre_windows_msys2>`.

3. :ref:`Build for Windows (VS 2022)<build_vs_2022>`.

Build
=====

.. _build_android:

Android
-------

.. code::

	make android-arm-release MAKE_JOBS=4

.. _build_html5:

HTML5
-----

.. code::

	make wasm-release MAKE_JOBS=4

.. _build_linux:

Linux
-----

.. code::

	make tools-linux-release64 MAKE_JOBS=4

.. _build_msys2:

Windows (MSYS2)
---------------

Open MSYS2 MINGW64 shell:

.. code::

	export MINGW=/mingw64
	make tools-mingw-release64 MAKE_JOBS=4

.. _build_vs_code:

Windows (VS Code)
-----------------

Open Visual Studio Code Shell:

.. code::

	$env:MINGW = "C:\\msys64\\mingw64"
	make tools-mingw-release64 MAKE_JOBS=4

.. _build_vs_2022:

Windows (VS 2022)
-----------------

1. Open Visual Studio 2022 Command Prompt:

.. code::

	make tools-windows-release64 MAKE_JOBS=4

2. To build tools, open a MSYS2 MINGW64 shell:

.. code::

	export MINGW=/mingw64
	make tools-mingw-release64 MAKE_JOBS=4

