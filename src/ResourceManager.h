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

#include "Types.h"
#include "Dictionary.h"
#include "Resource.h"

namespace crown
{

/**
	Resource manager.

	Keeps track of loaded resources.
*/
class ResourceManager
{

	// FIXME: replace with hashmap
	typedef Dictionary<StrId64, Resource*> NameToResourceDict;

public:

	Resource*	Create(const char* name, bool& created);
	void		Destroy(const char* name);

	Resource*	Load(const char* name);
	void		Unload(const char* name, bool reload);

	Resource*	GetByName(const char* name);
	int			GetResourceCount() const;

	/**
	 * Creates a generic resource with the given name.
	 * If a resource with the same name already exists, the
	 * already existent resource will be returned. In order
	 * to distinguish between a newly created resource or a
	 * pointer to an existing one, you have to check
	 * at the value returned by 'created'.
	 * @param name The name of the resource
	 * @param created Returns true if newly created, false otherwise
	 * @return A pointer to the created resource
	 */

	/**
	 * Loads a generic resource from file.
	 * The name of the file determines the name of the resource and vice-versa.
	 * @param name Tha name of the resource
	 * @return A pointer to the loaded resource
	 */

	/**
		Unloads a generic resource with the given name.
	@param name
		The name of the resource
	@param reload
		Whether to reload after unloading
	*/

	/**
	 * Destroys a generic resource with the given name.
	 * Causes the manager to remove the resource from its
	 * internal list.
	 * @note Destroying here is a misleading term since the
	 * resource is not destroyed if there are other references
	 * to it.
	 * @param name The name of the resource
	 */

	/**
	 * Returns a resource by its name.
	 * If the resource does not exists, returns a null pointer.
	 * @param name The name of the resource
	 * @return A pointer to the resource
	 */

	/**
	 * Returns the number of resources managed by this resource manager.
	 * @return The number of resources
	 */

protected:

						ResourceManager();		//!< Constructor
	virtual				~ResourceManager();		//!< Destructor

	virtual Resource*	CreateSpecific(const char* name) = 0;
	virtual void		DestroySpecific(const char* name);

	// Resource name -> Resource pointer
	NameToResourceDict	mNameToResourceDict;
};

} // namespace crown

