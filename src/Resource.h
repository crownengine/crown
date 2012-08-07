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

namespace Crown
{

/**
	Enumerates the loading state of a resource.
*/
enum LoadState
{
	LS_UNLOADED		= 0,	//!< The resouce is not loaded
	LS_LOADING		= 1,	//!< The resource is loading
	LS_LOADED		= 2,	//!< The resource is loaded
	LS_UNLOADING	= 3		//!< The resource is unloading
};

class ResourceManager;

/**
	Abstracts a generic resource.

	Each resource is uniquely identified by its name.
	The name of a resource is simply the path to the
	corresponding file on the disk relative to the root path.

	More info about the root path in Filesystem.

	"textures/grass.tga" is a texture resource located at
	"ROOT_PATH/textures/grass.tga".

	Paths to resources follow the same conventions used
	for referring to files on disk covered in Filesystem.

	In order to save space and keep high performances, resource
	names are stored as 64-bit long sting hashes.
*/
class Resource
{

public:

						/**
							Constructor.
						*/
						Resource() : mIsReloadable(false), mLoadState(LS_UNLOADED), mSize(0) {}
						/**
							Destructor.
						*/
	virtual				~Resource() { }
						/**
							Loads the resource.
						*/
	virtual void		Load(const char* name) { (void)name; }
						/**
							Unloads the resource and then optionally reloads it.
						@param reload
							Whether to reload after unloading
						*/
	virtual void		Unload(const char* name, bool reload) { (void)name; (void)reload; }
						/**
							Returns whether the resource is reloadable.
						@return Whether is reloadable
						*/
	inline bool			IsReloadable() { return mIsReloadable; }
						/**
							Returns the loading state of the resource.
						@return The loading state
						*/
	inline LoadState	GetLoadState() { return mLoadState; }
						/**
							Returns the resource's size in bytes.
						@return The size in bytes
						*/
	inline size_t		GetSize() { return mSize; }

protected:

	bool				mIsReloadable;	//!< Whether is reloadable
	LoadState			mLoadState;		//!< The loading state
	size_t				mSize;			//!< The size in bytes
};

} // namespace Crown

