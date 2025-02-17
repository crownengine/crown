/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/fbx_document.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/hash_map.inl"
#   include "core/strings/string_id.inl"
#   include "resource/compile_options.inl"
#   include <ufbx.h>

namespace crown
{
namespace fbx
{
	s32 parse(FBXDocument &fbx, Buffer &buf, CompileOptions &opts)
	{
		// Keep in sync with mesh_resource_fbx.vala!
		ufbx_load_opts load_opts = {};
		load_opts.target_camera_axes =
		{
			UFBX_COORDINATE_AXIS_POSITIVE_X,
			UFBX_COORDINATE_AXIS_POSITIVE_Z,
			UFBX_COORDINATE_AXIS_NEGATIVE_Y
		};
		load_opts.target_light_axes =
		{
			UFBX_COORDINATE_AXIS_POSITIVE_X,
			UFBX_COORDINATE_AXIS_POSITIVE_Y,
			UFBX_COORDINATE_AXIS_POSITIVE_Z
		};
		load_opts.target_axes = ufbx_axes_right_handed_z_up;
		load_opts.target_unit_meters = 1.0f;
		load_opts.space_conversion = UFBX_SPACE_CONVERSION_TRANSFORM_ROOT;

		ufbx_error error;
		fbx.scene = ufbx_load_memory(array::begin(buf)
			, array::size(buf)
			, &load_opts
			, &error
			);
		RETURN_IF_FALSE(fbx.scene != NULL
			, opts
			, "ufbx: %s"
			, error.description.data
			);
		return 0;
	}

	///
	s32 parse(FBXDocument &fbx, const char *path, CompileOptions &opts)
	{
		RETURN_IF_FILE_MISSING(path, opts);
		Buffer buf = opts.read(path);
		return parse(fbx, buf, opts);
	}

} // namespace fbx

FBXDocument::FBXDocument(Allocator &a)
	: scene(NULL)
{
}

FBXDocument::~FBXDocument()
{
	ufbx_free_scene(scene);
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
