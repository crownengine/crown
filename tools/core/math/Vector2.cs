/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using System;

namespace Crown
{
	public struct Vector2
	{
		public float x, y;

		public Vector2(float x, float y)
		{
			this.x = x;
			this.y = y;
		}

		public static Vector2 operator+(Vector2 a, Vector2 b)
		{
			return new Vector2(a.x + b.x, a.y + b.y);
		}

		public static Vector2 operator-(Vector2 a, Vector2 b)
		{
			return new Vector2(a.x - b.x, a.y - b.y);
		}

		public static Vector2 operator*(Vector2 a, float k)
		{
			return new Vector2(a.x * k, a.y * k);
		}

		public static Vector2 operator*(float k, Vector2 a)
		{
			return a * k;
		}

		public override string ToString()
		{
			return string.Format("{0}, {1}", x, y);
		}
	}
}
