/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

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
*/

#pragma once

#include <cstdlib>

namespace crown
{

/**
 * Singleton base class.
 * Implements the so-called "Phoenix Singleton".
 */
template <typename T>
class Singleton
{

public:

	static T* GetInstance()
	{
		if (!mInstance)
		{
			mInstance = new T;
		}

		return mInstance;
	}

protected:

	Singleton<T>()
	{
		atexit(&CleanUp);
	}

	~Singleton<T>() {}
	static T* mInstance;

private:

	static void CleanUp();

	// Disable copying
	Singleton<T>(const Singleton<T>&);
	Singleton<T>& operator=(const Singleton<T>&);
};

template <typename T>
T* Singleton<T>::mInstance = 0;

template <typename T>
void Singleton<T>::CleanUp()
{
	delete mInstance;
	mInstance = 0;
}

} // namespace crown

