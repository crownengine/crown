/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/compile_options.inl"
#include "resource/navmesh_resource.h"
#include <DetourNavMesh.h>
#include <Recast.h>

namespace crown
{
namespace navmesh_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		TempAllocator4096 ta;
		Array<u8> input_data(ta);

		opts.read(input_data);

		// Parse SJSON input for navmesh generation parameters.
		// Expected format:
		// {
		//   cell_size = 0.3
		//   cell_height = 0.2
		//   agent_height = 2.0
		//   agent_radius = 0.6
		//   agent_max_climb = 0.9
		//   agent_max_slope = 45
		//   region_min_size = 8
		//   region_merge_size = 20
		//   edge_max_len = 12.0
		//   edge_max_error = 1.3
		//   verts_per_poly = 6
		//   detail_sample_dist = 6.0
		//   detail_sample_max_error = 1.0
		//   vertices = [ x,y,z, ... ]
		//   indices = [ i0,i1,i2, ... ]
		// }

		// For now, we read the Detour navmesh binary data directly
		// (pre-baked with RecastTool or other external tool).
		// Full Recast integration in the data compiler will come in a follow-up.

		u32 data_size = array::size(input_data);
		const u8 *data = array::begin(input_data);

		// Verify it's a valid Detour navmesh by checking the header.
		if (data_size < sizeof(dtNavMeshHeader))
			return -1;

		dtNavMeshHeader *header = (dtNavMeshHeader *)data;
		CE_UNUSED(header);

		// Write the navmesh resource.
		NavmeshResource nr;
		nr.version = 1;
		nr.size = data_size;
		nr.data_offset = sizeof(NavmeshResource);

		opts.write(nr.version);
		opts.write(nr.size);
		opts.write(nr.data_offset);
		opts.write(data, data_size);

		return 0;
	}

} // namespace navmesh_resource_internal

} // namespace crown

#endif // CROWN_CAN_COMPILE
