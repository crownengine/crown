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

#include "Types.h"
#include "ResourceManager.h"
#include "Str.h"

namespace Crown
{

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	NameToResourceDict::Enumerator e = mNameToResourceDict.getBegin();
	while (e.next())
	{
		e.current().value->Unload(NULL, false);
		delete e.current().value;
	}
}

Resource* ResourceManager::Create(const char* name, bool& created)
{
	Resource* resource = GetByName(name);

	created = false;

	if (resource == NULL)
	{
		resource = CreateSpecific(name);
		mNameToResourceDict[Str::Hash64(name)] = resource;
		created = true;
	}

	return resource;
}

Resource* ResourceManager::Load(const char* name)
{
	bool created;
	Resource* resource = Create(name, created);

	if (resource != NULL && created)
	{
		resource->Load(name);
	}

	return resource;
}

void ResourceManager::Unload(const char* name, bool reload)
{
	Resource* resource = GetByName(name);

	if (resource != NULL)
	{
		resource->Unload(name, reload);
	}
}

void ResourceManager::Destroy(const char* name)
{
	Resource* resource = GetByName(name);

	if (resource != NULL)
	{
		resource->Unload(name, false);
		DestroySpecific(name);
	}
}

Resource* ResourceManager::GetByName(const char* name)
{
	ulong nameHash = Str::Hash64(name);

	if (mNameToResourceDict.Contains(nameHash))
	{
		return mNameToResourceDict[nameHash];
	}

	return NULL;
}

void ResourceManager::DestroySpecific(const char* name)
{
	Resource* resource = GetByName(name);

	if (resource != NULL)
	{
		mNameToResourceDict.Remove(Str::Hash64(name));
	}
}

} // namespace Crown

