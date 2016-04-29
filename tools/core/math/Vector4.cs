/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using System.Collections;
using System;

namespace Crown
{
	public struct Vector4
	{
		public double x, y, z, w;

		public Vector4(double x, double y, double z, double w)
		{
			this.x = x;
			this.y = y;
			this.z = z;
			this.w = w;
		}

		public Vector4(ArrayList arr)
		{
			this.x = (double)arr[0];
			this.y = (double)arr[1];
			this.z = (double)arr[2];
			this.w = (double)arr[3];
		}

		public static Vector4 operator+(Vector4 a, Vector4 b)
		{
			return new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
		}

		public static Vector4 operator-(Vector4 a, Vector4 b)
		{
			return new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
		}

		public static Vector4 operator*(Vector4 a, float k)
		{
			return new Vector4(a.x * k, a.y * k, a.z * k, a.w * k);
		}

		public static Vector4 operator*(float k, Vector4 a)
		{
			return a * k;
		}

		public override string ToString()
		{
			return string.Format("{0}, {1}, {2}, {3}", x, y, z, w);
		}
	}
}
