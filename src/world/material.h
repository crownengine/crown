/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "resource_types.h"
#include "world_types.h"

namespace crown
{
struct Material
{
	const MaterialResource* _resource;
	char* _data;

	void bind(ResourceManager& rm, ShaderManager& sm, u8 view = 0) const;
	void set_float(const char* name, f32 val);
	void set_vector2(const char* name, const Vector2& val);
	void set_vector3(const char* name, const Vector3& val);
};

} // namespace crown
