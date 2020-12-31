/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
[Compact]
public struct Vector4
{
	public double x;
	public double y;
	public double z;
	public double w;

	public Vector4(double x, double y, double z, double w)
	{
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
	}

	public Vector4.from_array(ArrayList<Value?> arr)
	{
		this.x = (double)arr[0];
		this.y = (double)arr[1];
		this.z = (double)arr[2];
		this.w = (double)arr[3];
	}

	public ArrayList<Value?> to_array()
	{
		ArrayList<Value?> arr = new	ArrayList<Value?>();
		arr.add(this.x);
		arr.add(this.y);
		arr.add(this.z);
		arr.add(this.w);
		return arr;
	}

	public string to_string()
	{
		return "%f, %f, %f, %f".printf(x, y, z, w);
	}
}

public const Vector4 VECTOR4_ZERO = { 0.0, 0.0, 0.0, 0.0 };
public const Vector4 VECTOR4_ONE  = { 1.0, 1.0, 1.0, 1.0 };

}
