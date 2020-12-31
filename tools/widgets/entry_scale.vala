/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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

}
