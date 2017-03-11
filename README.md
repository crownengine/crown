The flexible game engine. [![Build Status](https://travis-ci.org/taylor001/crown.svg?branch=master)](https://travis-ci.org/taylor001/crown) [![Build status](https://ci.appveyor.com/api/projects/status/dabkwdxjr456hl52?svg=true)](https://ci.appveyor.com/project/taylor001/crown) [![Join the chat at https://gitter.im/taylor001/crown](https://badges.gitter.im/taylor001/crown.svg)](https://gitter.im/taylor001/crown?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
=====================================

Crown is a general purpose data-driven game engine, written from scratch in [orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b) with a minimalistic and data-oriented design philosophy in mind.

It is loosely inspired by Bitsquid (now Stingray) engine and its design principles; the current Lua API is similar to that of Bitsquid but this engine is *not* meant to be its clone *nor* to be API compatible with it.

##Documentation

  * [Manual](http://taylor001.github.io/crown/html)
  * [Lua API](http://taylor001.github.io/crown/html/lua_api.html)
  * [C++ API](http://taylor001.github.io/crown/doxygen/modules)

##Download (Stable)

### Linux (Tarball)
[pepper-0.0.25-linux-x64.tar.gz](https://github.com/taylor001/crown/releases/download/v0.0.25/pepper-0.0.25-linux-x64.tar.gz)

### ArchLinux

	$ yaourt -S pepper

##Screenshots

###[Level Editor](https://github.com/taylor001/crown/tree/master/tools/level_editor)

![level-editor](https://raw.githubusercontent.com/taylor001/crown/master/docs/shots/level-editor.png)

###[00-empty](https://github.com/taylor001/crown/tree/master/samples/00-empty)

Engine initialization and shutdown.

###[01-physics](https://github.com/taylor001/crown/tree/master/samples/01-physics)
![01-physics](https://raw.githubusercontent.com/taylor001/crown/master/docs/shots/01-physics.png)

##Building

### Prerequisites

### Android

Android NDK (https://developer.android.com/tools/sdk/ndk/index.html)

	$ export ANDROID_NDK_ROOT=<path/to/android_ndk>
	$ export ANDROID_NDK_ARM=<path/to/android_ndk_arm>

### Linux (Ubuntu >= 16.04)

    $ sudo add-apt-repository ppa:vala-team
    $ sudo apt-get install libgtk-3-dev valac libgee-0.8-dev
    $ sudo apt-get install mesa-common-dev libgl1-mesa-dev libpulse-dev libxrandr-dev

### Windows

MSYS2 (http://www.msys2.org)

### Building and running Level Editor

	$ make tools-linux-release64
	$ cd build/linux64/bin
	$ ./level-editor-release ../../../samples/01-physics ../../../samples /tmp/pepper_$RANDOM

Contact
-------

Daniele Bartolini ([@aa_dani_bart](https://twitter.com/aa_dani_bart))  
Project page: https://github.com/taylor001/crown

Contributors
------------

In chronological order.

Daniele Bartolini ([@taylor001](https://github.com/taylor001))  
Simone Boscaratto ([@Xed89](https://github.com/Xed89))  
Michele Rossi ([@mikymod](https://github.com/mikymod))  
Michela Iacchelli - Pepper logo.

License
-------

	Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
