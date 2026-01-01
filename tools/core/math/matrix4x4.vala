/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
[Compact]
public struct Matrix4x4
{
	public Vector4 x;
	public Vector4 y;
	public Vector4 z;
	public Vector4 t;

	public Matrix4x4(Vector4 x, Vector4 y, Vector4 z, Vector4 t)
	{
		this.x = x;
		this.y = y;
		this.z = z;
		this.t = t;
	}

	public Matrix4x4.from_array(Gee.ArrayList<Value?> arr)
	{
		this.x.x = (double)arr[ 0];
		this.x.y = (double)arr[ 1];
		this.x.z = (double)arr[ 2];
		this.x.w = (double)arr[ 3];

		this.y.x = (double)arr[ 4];
		this.y.y = (double)arr[ 5];
		this.y.z = (double)arr[ 6];
		this.y.w = (double)arr[ 7];

		this.z.x = (double)arr[ 8];
		this.z.y = (double)arr[ 9];
		this.z.z = (double)arr[10];
		this.z.w = (double)arr[11];

		this.t.x = (double)arr[12];
		this.t.y = (double)arr[13];
		this.t.z = (double)arr[14];
		this.t.w = (double)arr[15];
	}

	public Vector3 scale()
	{
		double sx = this.x.to_vector3().length();
		double sy = this.y.to_vector3().length();
		double sz = this.z.to_vector3().length();
		return Vector3(sx, sy, sz);
	}

	public Quaternion rotation()
	{
		double lx = this.x.length();
		double ly = this.y.length();
		double lz = this.z.length();

		if (MathUtils.equal(lx, 0.0f) || MathUtils.equal(ly, 0.0f) || MathUtils.equal(lz, 0.0f))
			return QUATERNION_IDENTITY;

		Matrix4x4 rot = this;
		rot.x.normalize();
		rot.y.normalize();
		rot.z.normalize();
		Quaternion q = Quaternion.from_matrix(rot);
		q.normalize();
		return q;
	}
}

} /* namespace Crown */
