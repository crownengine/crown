/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class EntryScale : EntryVector3
{
	public EntryScale(Vector3 xyz = VECTOR3_ONE, Vector3 min = VECTOR3_ZERO, Vector3 max = VECTOR3_MAX)
	{
		base(xyz, min, max, "%.4g");
	}
}

} /* namespace Crown */
