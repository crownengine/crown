/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
[Compact]
public struct Vector2
{
	public double x;
	public double y;

	public Vector2(double x, double y)
	{
		this.x = x;
		this.y = y;
	}

	public Vector2.from_array(ArrayList<Value?> arr)
	{
		this.x = (double)arr[0];
		this.y = (double)arr[1];
	}

	public ArrayList<Value?> to_array()
	{
		ArrayList<Value?> arr = new	ArrayList<Value?>();
		arr.add(this.x);
		arr.add(this.y);
		return arr;
	}

	public string to_string()
	{
		return "%f, %f".printf(x, y);
	}
}

public const Vector2 VECTOR2_ZERO = { 0.0, 0.0 };
public const Vector2 VECTOR2_ONE  = { 1.0, 1.0 };

}
