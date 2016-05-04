/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using System.Collections;
using System;

namespace Crown
{
	public struct Vector2
	{
		public double x, y;

		public Vector2(double x, double y)
		{
			this.x = x;
			this.y = y;
		}

		public Vector2(ArrayList arr)
		{
			this.x = (double)arr[0];
			this.y = (double)arr[1];
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
