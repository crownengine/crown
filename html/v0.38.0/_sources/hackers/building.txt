========
Building
========

Getting Source
==============

.. code::

	git clone https://github.com/dbartolini/crown.git

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

Linux (Ubuntu >= 16.04)
-----------------------

.. code::

	sudo apt-get install build-essential
	sudo apt-get install mesa-common-dev libgl1-mesa-dev libpulse-dev libxrandr-dev

	sudo add-apt-repository ppa:vala-team
	sudo apt-get install libgtk-3-dev valac libgee-0.8-dev

To build documentation you will also need:

.. code::

	sudo apt-get install python3-sphinx
	sudo apt-get install doxygen

Windows (VS 2017)
-----------------

Visual Studio 2017:

	* https://visualstudio.microsoft.com/it/downloads)

GnuWin32 utilities:

	* http://gnuwin32.sourceforge.net/packages/make.htm
	* http://gnuwin32.sourceforge.net/packages/coreutils.htm
	* http://gnuwin32.sourceforge.net/packages/libiconv.htm
	* http://gnuwin32.sourceforge.net/packages/libintl.htm

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

Windows
-------

Open Visual Studio 2017 Command Prompt:

.. code::

	make tools-linux-release64
