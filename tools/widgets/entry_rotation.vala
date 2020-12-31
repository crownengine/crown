/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class EntryRotation : EntryVector3
{
	public new Quaternion value
	{
		get
		{
			double x = MathUtils.rad(_x.value);
			double y = MathUtils.rad(_y.value);
			double z = MathUtils.rad(_z.value);
			return Quaternion.from_euler(x, y, z);
		}
		set
		{
			_stop_emit = true;
			Vector3 euler = ((Quaternion)value).to_euler();
			_x.value = MathUtils.deg(euler.x);
			_y.value = MathUtils.deg(euler.y);
			_z.value = MathUtils.deg(euler.z);
			_stop_emit = false;
		}
	}

	public EntryRotation(Vector3 xyz = VECTOR3_ZERO)
	{
		base(xyz, Vector3(-180.0, -180.0, -180.0), Vector3(180.0, 180.0, 180.0), "%.4g");
	}
}

}
