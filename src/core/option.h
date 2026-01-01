/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace crown
{
/// Tracks whether the option's value has been changed from its default.
/// @note Changing a value to its default still counts as a change.
///
/// @ingroup Core
template<typename T>
struct Option
{
	T _value;
	bool _changed;

	///
	explicit Option(const T &default_val);

	///
	Option(const Option &opt) = delete;

	///
	const Option<T> &operator=(const Option &opt) = delete;

	///
	const T &value() const;

	///
	void set_value(const T &val);

	/// Returns whether the option's value has been changed from its default.
	bool has_changed() const;
};

} // namespace crown
