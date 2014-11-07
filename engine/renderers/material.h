/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "resource_types.h"
#include <bgfx.h>

namespace crown
{

struct MaterialManager;

struct Material
{
	void create(const MaterialResource* mr, MaterialManager& mm);
	void destroy() const;
	void bind() const;

	void set_float(const char* name, float val);
	void set_vector2(const char* name, const Vector2& val);
	void set_vector3(const char* name, const Vector3& val);

	const MaterialResource* resource;
	char* data;
};

} // namespace crown
