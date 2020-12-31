/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/reader_writer.inl"
#include "resource/compile_options.h"

#if CROWN_CAN_COMPILE
namespace crown
{
///
template <typename T>
void CompileOptions::write(const T& data)
{
	_binary_writer.write(data);
}

} // namespace crown

#endif // CROWN_CAN_COMPILE
