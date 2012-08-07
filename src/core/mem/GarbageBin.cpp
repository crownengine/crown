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

#include "GarbageBin.h"
#include "Device.h"
#include "Types.h"

namespace Crown
{

void IGarbageable::Trash()
{
	GetDevice()->GetGarbageBin()->Trash(this);
}

GarbageBin::GarbageBin()
{
}

GarbageBin::~GarbageBin()
{
	Empty();
}

void GarbageBin::Trash(IGarbageable* object)
{
	if (mGarbage.Find(object) == -1)
	{
		mGarbage.Append(object);
	}
}

void GarbageBin::Empty()
{
	for (int i=0; i<mGarbage.GetSize(); i++)
	{
		delete mGarbage[i];
	}

	mGarbage.Clear();
}

} //namespace Crown
