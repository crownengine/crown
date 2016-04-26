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
/// Material
///
/// @ingroup World
struct Material
{
	const MaterialResource* _resource;
	char* _data;

	void bind(ResourceManager& rm, ShaderManager& sm, u8 view = 0) const;

	/// Sets the @a value of the variable @a name.
	void set_float(StringId32 name, f32 value);

	/// Sets the @a value of the variable @a name.
	void set_vector2(StringId32 name, const Vector2& value);

	/// Sets the @a value of the variable @a name.
	void set_vector3(StringId32 name, const Vector3& value);
};

} // namespace crown
