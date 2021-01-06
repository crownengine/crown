/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE

#include "core/containers/types.h"
#include "core/json/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"

namespace crown
{
struct UnitCompiler
{
	typedef s32 (*CompileFunction)(Buffer& output, const char* json, CompileOptions& opts);

	struct ComponentTypeData
	{
		ALLOCATOR_AWARE;

		CompileFunction _compiler;

		u32 _num;
		Array<u32> _unit_index;
		Buffer _data;

		ComponentTypeData(Allocator& a)
			: _num(0)
			, _unit_index(a)
			, _data(a)
		{
		}
	};

	struct ComponentTypeInfo
	{
		StringId32 _type;
		float _spawn_order;

		bool operator<(const ComponentTypeInfo& a) const
		{
			return _spawn_order < a._spawn_order;
		}
	};

	CompileOptions& _opts;
	u32 _num_units;
	HashMap<StringId32, ComponentTypeData> _component_data;
	Array<ComponentTypeInfo> _component_info;
	Array<StringId32> _unit_names;
	Array<u32> _unit_parents;

	///
	void register_component_compiler(const char* type, CompileFunction fn, f32 spawn_order);

	///
	void register_component_compiler(StringId32 type, CompileFunction fn, f32 spawn_order);

	///
	s32 compile_component(Buffer& output, StringId32 type, const char* json);

	///
	UnitCompiler(CompileOptions& opts);

	///
	~UnitCompiler();


	///
	Buffer read_unit(const char* name);

	///
	s32 compile_unit(const char* path);

	///
	s32 compile_unit_from_json(const char* json, const u32 parent);

	///
	s32 compile_units_array(const JsonArray& units, const u32 parent);

	///
	s32 compile_units_array(const char* json, const u32 parent);

	///
	s32 collect_units(Buffer& data, Array<u32>& prefabs, const char* json);

	///
	Buffer blob();
};

} // namespace crown

#endif // CROWN_CAN_COMPILE
