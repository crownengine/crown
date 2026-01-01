/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/types.h"
#include "core/guid.h"
#include "core/json/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"

namespace crown
{
typedef s32 (*CompileFunction)(Buffer &output, FlatJsonObject &obj, CompileOptions &opts);

struct ComponentTypeData
{
	ALLOCATOR_AWARE;

	u32 _num;
	Array<u32> _unit_index;
	Buffer _data;
	CompileFunction _compiler;

	explicit ComponentTypeData(Allocator &a)
		: _num(0)
		, _unit_index(a)
		, _data(a)
		, _compiler(NULL)
	{
	}
};

struct ComponentTypeInfo
{
	StringId32 _type;
	float _spawn_order;

	bool operator<(const ComponentTypeInfo &a) const
	{
		return _spawn_order < a._spawn_order;
	}
};

struct Unit
{
	ALLOCATOR_AWARE;

	StringId32 _editor_name;
	JsonArray _merged_components;
	Vector<FlatJsonObject> _flattened_components;
	HashMap<Guid, Unit *> _children;
	Unit *_parent;

	///
	explicit Unit(Allocator &a);
};

struct UnitCompiler
{
	HashMap<Guid, Unit *> _units;
	Buffer _prefab_data;
	Array<u32> _prefab_offsets;
	Array<StringId64> _prefab_names;
	HashMap<StringId32, ComponentTypeData> _component_data;
	Array<ComponentTypeInfo> _component_info;
	Array<StringId32> _unit_names;
	Array<u32> _unit_parents;
	u32 _num_units;

	///
	explicit UnitCompiler(Allocator &a);

	///
	~UnitCompiler();
};

namespace unit_compiler
{
	///
	s32 parse_unit(UnitCompiler &c, const char *path, CompileOptions &opts);

	///
	s32 parse_unit_array_from_json(UnitCompiler &c, const char *units_array_json, CompileOptions &opts);

	///
	s32 blob(Buffer &output, UnitCompiler &c);

} // namespace unit_compiler

} // namespace crown

#endif // if CROWN_CAN_COMPILE
