/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using System.Collections;
using System;

namespace Crown
{
	public struct Quaternion
	{
		public float x, y, z, w;

		public Quaternion(float x, float y, float z, float w)
		{
			this.x = x;
			this.y = y;
			this.z = z;
			this.w = w;
		}

		public Quaternion(ArrayList arr)
		{
			this.x = (float)(double)arr[0];
			this.y = (float)(double)arr[1];
			this.z = (float)(double)arr[2];
			this.w = (float)(double)arr[3];
		}

		public Quaternion(Vector3 axis, float angle)
		{
			float ha = angle * 0.5f;
			float sa = (float)Math.Sin(ha);
			float ca = (float)Math.Cos(ha);
			this.x = axis.x * sa;
			this.y = axis.y * sa;
			this.z = axis.z * sa;
			this.w = ca;
		}

		public override string ToString()
		{
			return string.Format("{0}, {1}, {2}, {3}", x, y, z, w);
		}
	}
}
