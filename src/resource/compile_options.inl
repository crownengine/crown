/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/reader_writer.inl"
#include "resource/compile_options.h"

#if CROWN_CAN_COMPILE
namespace crown
{
///
template<typename T>
void CompileOptions::write(const T &data)
{
	_binary_writer.write(data);
}

} // namespace crown

#endif
