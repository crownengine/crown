/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/filesystem/reader_writer.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.h"
#include "core/process.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include "resource/compile_options.h"
#include "resource/resource_manager.h"
#include "resource/texture_resource.h"

namespace crown
{
namespace texture_resource_internal
{
	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);

		u32 version;
		br.read(version);
		CE_ASSERT(version == RESOURCE_HEADER(RESOURCE_VERSION_TEXTURE), "Wrong version");

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

#if CROWN_CAN_COMPILE
namespace texture_resource_internal
{
	static const char* texturec_paths[] =
	{
		EXE_PATH("texturec"),
	#if CROWN_DEBUG
		EXE_PATH("texturec-debug")
	#elif CROWN_DEVELOPMENT
		EXE_PATH("texturec-development")
	#else
		EXE_PATH("texturec-release")
	#endif
	};

	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(obj, buf);

		DynamicString name(ta);
		sjson::parse_string(name, obj["source"]);
		DATA_COMPILER_ASSERT_FILE_EXISTS(name.c_str(), opts);
		opts.fake_read(name.c_str());

		const bool generate_mips = sjson::parse_bool(obj["generate_mips"]);
		const bool normal_map    = sjson::parse_bool(obj["normal_map"]);

		DynamicString texsrc(ta);
		DynamicString texout(ta);
		opts.absolute_path(name.c_str(), texsrc);
		opts.get_temporary_path("ktx", texout);

		const char* texturec = opts.exe_path(texturec_paths, countof(texturec_paths));
		DATA_COMPILER_ASSERT(texturec != NULL
			, opts
			, "texturec not found"
			);

		const char* argv[] =
		{
			texturec,
			"-f",
			texsrc.c_str(),
			"-o",
			texout.c_str(),
			(generate_mips ? "-m" : ""),
			(normal_map    ? "-n" : ""),
			NULL
		};
		Process pr;
		s32 sc = pr.spawn(argv, ProcessFlags::STDOUT_PIPE | ProcessFlags::STDERR_MERGE);
		DATA_COMPILER_ASSERT(sc == 0
			, opts
			, "Failed to spawn `%s`"
			, argv[0]
			);
		StringStream output(ta);
		// Read error messages if any
		{
			char err[512];
			while (pr.fgets(err, sizeof(err)) != NULL)
				output << err;
		}
		s32 ec = pr.wait();
		DATA_COMPILER_ASSERT(ec == 0
			, opts
			, "Failed to compile texture:\n%s"
			, string_stream::c_str(output)
			);

		Buffer blob = opts.read_temporary(texout.c_str());
		opts.delete_file(texout.c_str());

		// Write DDS
		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_TEXTURE));
		opts.write(array::size(blob));
		opts.write(blob);

		return 0;
	}

} // namespace texture_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
