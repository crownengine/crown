/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/guid.h"
#include "core/strings/dynamic_string.h"

namespace crown
{
void DynamicString::from_guid(const Guid& guid)
{
	array::resize(_data, 36);
	guid::to_string(array::begin(_data), 36, guid);
}

void DynamicString::from_string_id(const StringId32& id)
{
	array::resize(_data, 8);
	id.to_string(array::begin(_data), 8);
}

void DynamicString::from_string_id(const StringId64& id)
{
	array::resize(_data, 16);
	id.to_string(array::begin(_data), 16);
}

} // namespace crown
