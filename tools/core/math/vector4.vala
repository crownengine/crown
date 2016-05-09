/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gee;

namespace Crown
{
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

		public string to_string()
		{
			return "%f, %f, %f, %f".printf(x, y, z, w);
		}
	}
}
