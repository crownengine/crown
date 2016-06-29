The flexible game engine. [![Build Status](https://travis-ci.org/taylor001/crown.svg?branch=master)](https://travis-ci.org/taylor001/crown) [![Build status](https://ci.appveyor.com/api/projects/status/dabkwdxjr456hl52?svg=true)](https://ci.appveyor.com/project/taylor001/crown) [![PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=6FQMPUQQ8KQKW)
=====================================

Crown is a general purpose data-driven game engine, written from scratch in [orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b) with a minimalistic and data-oriented design philosophy in mind.

It is loosely inspired by Bitsquid (now Stingray) engine and its design principles; the current Lua API is similar to that of Bitsquid but this engine is *not* meant to be its clone *nor* to be API compatible with it.

##Documentation

  * [Manual](http://taylor001.github.io/crown/manual.html)  
  * [Lua API](http://taylor001.github.io/crown/lua_api.html)  
  * [C++ API](http://taylor001.github.io/crown/doxygen/modules)

##Screenshots

###[Level Editor](https://github.com/taylor001/crown/tree/master/tools/level_editor)

![level-editor](https://raw.githubusercontent.com/taylor001/crown/master/docs/shots/level-editor.png)

[WIP] Node editor.

![node-editor](https://raw.githubusercontent.com/taylor001/crown/master/docs/shots/node-editor.png)

###[00-hello-world](https://github.com/taylor001/crown/tree/master/samples/00-hello-world)

Engine initialization and shutdown.

###[01-physics](https://github.com/taylor001/crown/tree/master/samples/01-physics)
![01-physics](https://raw.githubusercontent.com/taylor001/crown/master/docs/shots/01-physics.png)

##Download

Linux: [pepper-0.0.23-linux-x64.tar.gz](https://github.com/taylor001/crown/releases/download/v0.0.23/pepper-0.0.23-linux-x64.tar.gz)

### ArchLinux

	$ yaourt -S pepper && pepper

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

GnuWin32 make (http://gnuwin32.sourceforge.net/packages/make.htm)  
GnuWin32 coreutils (http://gnuwin32.sourceforge.net/packages/coreutils.htm)  
GnuWin32 libiconv (http://gnuwin32.sourceforge.net/packages/libiconv.htm)  
GnuWin32 libintl (http://gnuwin32.sourceforge.net/packages/libintl.htm)

### Building engine

	$ make engine-<configuration>
	
Configuration is `<platform>-[arch-]<debug/development/release>[32|64]`. E.g.

	linux-debug64, linux-development64, linux-release64, android-arm-debug, windows-debug32 etc.

### Building tools

	$ make tools-linux-<debug/release>64

##Samples

To run a sample on 64-bit linux development mode:

    $ cd build/engine/linux64/bin
	$ ./linux-development-64 --source-dir <path> --bundle-dir <path> --compile --platform linux --continue

Contact
-------

Daniele Bartolini ([@aa_dani_bart](https://twitter.com/aa_dani_bart))  
Project page: https://github.com/taylor001/crown

Contributors
------------

In alphabetical order.

Michela Iacchelli - Pepper logo.  
Michele Rossi ([@mikymod](https://github.com/mikymod))  
Simone Boscaratto ([@Xed89](https://github.com/Xed89))

License (Engine)
----------------

	Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.

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

License (Tools)
---------------

	Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
