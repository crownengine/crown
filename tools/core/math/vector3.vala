/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
[Compact]
public struct Vector3
{
	public double x;
	public double y;
	public double z;

	public Vector3(double x, double y, double z)
	{
		this.x = x;
		this.y = y;
		this.z = z;
	}

	public Vector3.from_array(Gee.ArrayList<Value?> arr)
	{
		this.x = (double)arr[0];
		this.y = (double)arr[1];
		this.z = (double)arr[2];
	}

	public Gee.ArrayList<Value?> to_array()
	{
		Gee.ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
		arr.add(this.x);
		arr.add(this.y);
		arr.add(this.z);
		return arr;
	}

	public double dot(Vector3 b)
	{
		return this.x * b.x + this.y * b.y + this.z * b.z;
	}

	public Vector3 cross(Vector3 b)
	{
		return Vector3(this.y * b.z - this.z * b.y
			, this.z * b.x - this.x * b.z
			, this.x * b.y - this.y * b.x
			);
	}

	public double length_squared()
	{
		return dot(this);
	}

	public double length()
	{
		return Math.sqrt(length_squared());
	}

	public Vector3 normalize()
	{
		double len = length();
		double inv_len = 1.0 / len;
		this.x *= inv_len;
		this.y *= inv_len;
		this.z *= inv_len;
		return this;
	}

	public void set_length(double len)
	{
		normalize();
		this.x *= len;
		this.y *= len;
		this.z *= len;
	}

	public string to_string()
	{
		return "%f, %f, %f".printf(x, y, z);
	}

	public static bool equal_func(Vector3? a, Vector3? b)
	{
		return a.x == b.x
			&& a.y == b.y
			&& a.z == b.z
			;
	}
}

public const Vector3 VECTOR3_ZERO = { 0.0, 0.0, 0.0 };
public const Vector3 VECTOR3_ONE  = { 1.0, 1.0, 1.0 };
public const Vector3 VECTOR3_MIN  = {-double.MAX, -double.MAX, -double.MAX };
public const Vector3 VECTOR3_MAX  = { double.MAX,  double.MAX,  double.MAX };

} /* namespace Crown */
