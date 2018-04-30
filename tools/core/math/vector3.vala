/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
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

		public Vector3.from_array(ArrayList<Value?> arr)
		{
			this.x = (double)arr[0];
			this.y = (double)arr[1];
			this.z = (double)arr[2];
		}

		public ArrayList<Value?> to_array()
		{
			ArrayList<Value?> arr = new	ArrayList<Value?>();
			arr.add(this.x);
			arr.add(this.y);
			arr.add(this.z);
			return arr;
		}

		public string to_string()
		{
			return "%f, %f, %f".printf(x, y, z);
		}
	}

	const Vector3 VECTOR3_ZERO = { 0.0, 0.0, 0.0 };
	const Vector3 VECTOR3_ONE  = { 1.0, 1.0, 1.0 };
}
