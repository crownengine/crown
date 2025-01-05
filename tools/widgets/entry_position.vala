/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class EntryPosition : EntryVector3
{
	public EntryPosition(Vector3 xyz = VECTOR3_ZERO, Vector3 min = VECTOR3_MIN, Vector3 max = VECTOR3_MAX)
	{
		base(xyz, min, max, "%.5g");
	}
}

} /* namespace Crown */
