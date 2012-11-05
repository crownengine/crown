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

#include "Weak.h"
#include "Dictionary.h"
#include "Shared.h"
#include "List.h"
#include "Log.h"

namespace crown
{

typedef List<WeakReferenced**> ObjectPPList;
typedef Shared<ObjectPPList> SharedObjectPPList;

//Dictionary<WeakReferenced*, SharedObjectPPList> mWeakAssociations;

WeakReferenced::WeakReferenced()
{

}

int count = 0;

WeakReferenced::~WeakReferenced()
{
	//Set all Weak pointers that point to this Object to NULL
	WeakAssociationsStorage::UnRegisterAllByObject(this);
	//Log::I(Str(count));
	count += 1;
}

void WeakAssociationsStorage::Register(WeakReferenced** ptr)
{
	if (*ptr == NULL)
	{
		return;
	}

//	SharedObjectPPList& list = mWeakAssociations[*ptr];
//	if (list.IsNull())
//	{
//		list = new ObjectPPList();
//	}
//	list->Append(ptr);
}

void WeakAssociationsStorage::UnRegister(WeakReferenced** ptr)
{
	if (*ptr == NULL)
	{
		return;
	}

//	SharedObjectPPList& list = mWeakAssociations[*ptr];

//	int index = list->Find(ptr);
//	if (index != -1)
//	{
//		list->Remove(index);
//	}
}

void WeakAssociationsStorage::UnRegisterAllByObject(WeakReferenced* object)
{
	if (object == NULL)
	{
		return;
	}

	//SB NOTE: Here, when CRT does the cleanup of the whole dictionary upon application exit, we look for
	//				 Weak pointers that point to the lists contained in the dictionary, since lists are themselves Objects
	//				 The underlying RBTree has been designed to be re-entrant on its Clear method.
//	SharedObjectPPList& list = mWeakAssociations[object];
//	if (list.IsNull())
//	{
//		return;
//	}

//	ObjectPPList::Enumerator e = list->getBegin();
//	while(e.next())
//	{
//		*e.current() = NULL;
//	}
//	mWeakAssociations.Remove(object);
}

}
