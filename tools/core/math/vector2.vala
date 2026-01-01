/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

	public Vector2.from_array(Gee.ArrayList<Value?> arr)
	{
		this.x = (double)arr[0];
		this.y = (double)arr[1];
	}

	public Gee.ArrayList<Value?> to_array()
	{
		Gee.ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
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

} /* namespace Crown */
