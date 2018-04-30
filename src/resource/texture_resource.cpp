/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/filesystem/reader_writer.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include "resource/compile_options.h"
#include "resource/resource_manager.h"
#include "resource/texture_resource.h"

#if CROWN_DEVELOPMENT
	#define TEXTUREC_NAME "texturec-development"
#elif CROWN_DEBUG
	#define TEXTUREC_NAME "texturec-debug"
#else
	#define TEXTUREC_NAME "texturec-release"
#endif  // CROWN_DEBUG

#if CROWN_PLATFORM_LINUX
	#define TEXTUREC_PATH "./" TEXTUREC_NAME ""
#elif CROWN_PLATFORM_WINDOWS
	#define TEXTUREC_PATH TEXTUREC_NAME ".exe"
#else
	#define TEXTUREC_PATH ""
#endif // CROWN_PLATFORM_LINUX

namespace crown
{
namespace texture_resource_internal
{
	void compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		DynamicString name(ta);
		sjson::parse_string(object["source"], name);
		DATA_COMPILER_ASSERT_FILE_EXISTS(name.c_str(), opts);

		const bool generate_mips = sjson::parse_bool(object["generate_mips"]);
		const bool normal_map    = sjson::parse_bool(object["normal_map"]);

		DynamicString texsrc(ta);
		DynamicString texout(ta);
		opts.get_absolute_path(name.c_str(), texsrc);
		opts.get_temporary_path("ktx", texout);

		const char* argv[] =
		{
			TEXTUREC_PATH,
			"-f",
			texsrc.c_str(),
			"-o",
			texout.c_str(),
			(generate_mips ? "-m" : ""),
			(normal_map    ? "-n" : ""),
			NULL
		};

		StringStream output(ta);
		int ec = opts.run_external_compiler(argv, output);
		DATA_COMPILER_ASSERT(ec == 0
			, opts
			, "Failed to compile texture:\n%s"
			, string_stream::c_str(output)
			);

		Buffer blob = opts.read_temporary(texout.c_str());
		opts.delete_file(texout.c_str());

		// Write DDS
		opts.write(RESOURCE_VERSION_TEXTURE);
		opts.write(array::size(blob));
		opts.write(blob);
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		u32 version;
		br.read(version);
		CE_ASSERT(version == RESOURCE_VERSION_TEXTURE, "Wrong version");

		u32 size;
		br.read(size);

		TextureResource* tr = (TextureResource*)a.allocate(sizeof(TextureResource) + size);

		void* data = &tr[1];
		br.read(data, size);

		tr->mem        = bgfx::makeRef(data, size);
		tr->handle.idx = BGFX_INVALID_HANDLE;

		return tr;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		TextureResource* tr = (TextureResource*)rm.get(RESOURCE_TYPE_TEXTURE, id);
		tr->handle = bgfx::createTexture(tr->mem);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		TextureResource* tr = (TextureResource*)rm.get(RESOURCE_TYPE_TEXTURE, id);
		bgfx::destroy(tr->handle);
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

} // namespace texture_resource_internal

} // namespace crown
