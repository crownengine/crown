/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "Types.h"

namespace crown
{

struct AtomicInt
{
	explicit AtomicInt(int32_t value)
	{
		__sync_lock_test_and_set(&m_value, value);
	}

	AtomicInt& operator+=(int32_t value)
	{
		__sync_add_and_fetch(&m_value, value);
		return *this;
	}

	AtomicInt& operator++(void)
	{
		__sync_add_and_fetch(&m_value, 1);
		return *this;
	}

	AtomicInt& operator--(void)
	{
		__sync_sub_and_fetch(&m_value, 1);
		return *this;
	}

	AtomicInt& operator=(int32_t value)
	{
		__sync_lock_test_and_set(&m_value, value);
		return *this;
	}

	operator int32_t()
	{
		int32_t value = __sync_fetch_and_add(&m_value, 0);
		return value;
	}

private:

	int32_t m_value;
};

} // namespace crown
