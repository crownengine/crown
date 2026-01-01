/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/hash_map.inl"
#include "core/containers/queue.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/file_memory.inl"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/path.h"
#include "core/memory/globals.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/thread/scoped_mutex.inl"
#include "device/log.h"
#include "resource/package_resource.h"
#include "resource/resource_id.inl"
#include "resource/resource_loader.h"
#include "resource/resource_manager.h"
#include "resource/types.h"

LOG_SYSTEM(RESOURCE_LOADER, "resource_loader")

namespace crown
{
ResourceLoader::ResourceLoader(Filesystem &data_filesystem, bool is_bundle)
	: _data_filesystem(data_filesystem)
	, _is_bundle(is_bundle)
	, _requests(default_allocator())
	, _loaded(default_allocator())
	, _fallback(default_allocator())
	, _exit(false)
{
	_thread.start([](void *thiz) { return ((ResourceLoader *)thiz)->run(); }, this);
}

ResourceLoader::~ResourceLoader()
{
	_exit = true;
	_requests_condition.signal(); // Spurious wake to exit thread
	_thread.stop();
}

bool ResourceLoader::add_request(const ResourceRequest &rr)
{
	bool success = _requests.push(rr);
	if (success)
		_requests_condition.signal();

	return success;
}

void ResourceLoader::register_fallback(StringId64 type, StringId64 name)
{
	hash_map::set(_fallback, type, name);
}

File *ResourceLoader::open_stream(StringId64 type, StringId64 name)
{
	TempAllocator256 ta;
	DynamicString stream_dest(ta);
	stream_destination_path(stream_dest, resource_id(type, name));
	return _data_filesystem.open(stream_dest.c_str(), FileOpenMode::READ);
}

void ResourceLoader::close_stream(File *stream)
{
	_data_filesystem.close(*stream);
}

s32 ResourceLoader::run()
{
	while (1) {
		_mutex.lock();
		while (!_exit && _requests.empty())
			_requests_condition.wait(_mutex);

		_mutex.unlock();
		if (_exit)
			break;

		ResourceRequest rr;
		while (!_exit && _requests.pop(rr)) {
			ResourceId res_id = resource_id(rr.type, rr.name);

			TempAllocator128 ta;
			DynamicString path(ta);
			destination_path(path, res_id);

			if (_is_bundle) {
				if (rr.type == RESOURCE_TYPE_PACKAGE || rr.type == RESOURCE_TYPE_CONFIG) {
					File *file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);
					CE_ASSERT(file->is_open(), "Cannot load " RESOURCE_ID_FMT, res_id);

					// Load the resource.
					rr.data = rr.load_function(*file, *rr.allocator);

					_data_filesystem.close(*file);
				} else {
					// Get the package containing the resource.
					const PackageResource *pkg = (PackageResource *)rr.resource_manager->get(RESOURCE_TYPE_PACKAGE, rr.package_name);

					// Find the resource inside the package.
					for (u32 ii = 0; ii < pkg->num_resources; ++ii) {
						const ResourceOffset *offt = package_resource::resource_offset(pkg, ii);
						if (offt->type == rr.type && offt->name == rr.name) {
							const void *resource_data = package_resource::data(pkg) + offt->offset;

							// Load the resource.
							FileMemory fm(resource_data, offt->size);
							rr.data = rr.load_function(fm, *rr.allocator);
							break;
						}
					}
				}
			} else {
				File *file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);
				if (!file->is_open()) {
					logw(RESOURCE_LOADER, "Cannot load resource: " RESOURCE_ID_FMT ". Falling back...", res_id._id);

					StringId64 fallback_name;
					fallback_name = hash_map::get(_fallback, rr.type, fallback_name);
					CE_ENSURE(fallback_name._id != 0);

					res_id = resource_id(rr.type, fallback_name);
					destination_path(path, res_id);

					_data_filesystem.close(*file);
					file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);
				}
				CE_ASSERT(file->is_open(), "Cannot load fallback resource: " RESOURCE_ID_FMT, res_id._id);

				// Load the resource.
				rr.data = rr.load_function(*file, *rr.allocator);

				_data_filesystem.close(*file);
			}

#define MAX_TRIES 16
			while (1) {
				u32 num_tries = 0;
				while (num_tries++ < MAX_TRIES) {
					if (_loaded.push(rr))
						break;
				}
				if (num_tries < MAX_TRIES)
					break;

				os::sleep(16);
			}
#undef MAX_TRIES
		}
	}

	return 0;
}

} // namespace crown
