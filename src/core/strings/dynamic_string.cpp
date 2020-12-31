/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/guid.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_id.h"

namespace crown
{
void DynamicString::from_guid(const Guid& guid)
{
	array::resize(_data, GUID_BUF_LEN);
	guid::to_string(array::begin(_data), GUID_BUF_LEN, guid);
	array::pop_back(_data);
}

void DynamicString::from_string_id(const StringId32& id)
{
	array::resize(_data, STRING_ID32_BUF_LEN);
	id.to_string(array::begin(_data), STRING_ID32_BUF_LEN);
	array::pop_back(_data);
}

void DynamicString::from_string_id(const StringId64& id)
{
	array::resize(_data, STRING_ID64_BUF_LEN);
	id.to_string(array::begin(_data), STRING_ID64_BUF_LEN);
	array::pop_back(_data);
}

} // namespace crown
