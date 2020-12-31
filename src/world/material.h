/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/types.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
/// Material
///
/// @ingroup World
struct Material
{
	const MaterialResource* _resource;
	char* _data;

	///
	void bind(ResourceManager& rm, ShaderManager& sm, u8 view, s32 depth = 0) const;

	/// Sets the @a value of the variable @a name.
	void set_float(StringId32 name, f32 value);

	/// Sets the @a value of the variable @a name.
	void set_vector2(StringId32 name, const Vector2& value);

	/// Sets the @a value of the variable @a name.
	void set_vector3(StringId32 name, const Vector3& value);

	/// Sets the @a value of the variable @a name.
	void set_vector4(StringId32 name, const Vector4& value);

	/// Sets the @a value of the variable @a name.
	void set_matrix4x4(StringId32 name, const Matrix4x4& value);
};

} // namespace crown
