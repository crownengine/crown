/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using System.Collections;
using System;

namespace Crown
{
	public struct Vector3
	{
		public double x, y, z;

		public Vector3(double x, double y, double z)
		{
			this.x = x;
			this.y = y;
			this.z = z;
		}

		public Vector3(ArrayList arr)
		{
			this.x = (double)arr[0];
			this.y = (double)arr[1];
			this.z = (double)arr[2];
		}

		public static Vector3 operator+(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
		}

		public static Vector3 operator-(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
		}

		public static Vector3 operator*(Vector3 a, float k)
		{
			return new Vector3(a.x * k, a.y * k, a.z * k);
		}

		public static Vector3 operator*(float k, Vector3 a)
		{
			return a * k;
		}

		public override string ToString()
		{
			return string.Format("{0}, {1}, {2}", x, y, z);
		}
	}
}
