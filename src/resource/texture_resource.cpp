/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "compile_options.h"
#include "map.h"
#include "reader_writer.h"
#include "resource_manager.h"
#include "sjson.h"
#include "texture_resource.h"
#include "os.h"
#include "string_stream.h"

#if CROWN_DEVELOPMENT
	#define TEXTUREC_NAME "texturec-development-"
#elif CROWN_DEBUG
	#define TEXTUREC_NAME "texturec-debug-"
#else
	#define TEXTUREC_NAME "texturec-release-"
#endif  // CROWN_DEBUG
#if CROWN_ARCH_32BIT
	#define TEXTUREC_BITS "32"
#elif CROWN_ARCH_64BIT
	#define TEXTUREC_BITS "64"
#endif // CROWN_ARCH_32BIT
#if CROWN_PLATFORM_LINUX
	#define TEXTUREC_PATH "./" TEXTUREC_NAME "" TEXTUREC_BITS
#elif CROWN_PLATFORM_WINDOWS
	#define TEXTUREC_PATH TEXTUREC_NAME "" TEXTUREC_BITS ".exe"
#else
	#define TEXTUREC_PATH ""
#endif // CROWN_PLATFORM_LINUX

namespace crown
{
namespace texture_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		DynamicString name(ta);
		sjson::parse_string(object["source"], name);
		RESOURCE_COMPILER_ASSERT_FILE_EXISTS(name.c_str(), opts);

		const bool generate_mips = sjson::parse_bool(object["generate_mips"]);
		const bool is_normalmap  = sjson::parse_bool(object["is_normalmap"]);

		DynamicString texsrc(ta);
		DynamicString texout(ta);
		opts.get_absolute_path(name.c_str(), texsrc);
		opts.get_temporary_path("texture.ktx", texout);

		StringStream args(ta);
		args << " -f " << texsrc.c_str();
		args << " -o " << texout.c_str();
		args << (generate_mips ? " -m " : "");
		args << (is_normalmap  ? " -n " : "");

		StringStream output(ta);
		int exitcode = os::execute_process(TEXTUREC_PATH, string_stream::c_str(args), output);
		RESOURCE_COMPILER_ASSERT(exitcode == 0
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
		tr->handle.idx = bgfx::invalidHandle;

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
		bgfx::destroyTexture(tr->handle);
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

} // namespace texture_resource
} // namespace crown
