/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using System;

namespace Crown
{
	public static class MathUtils
	{
		public static bool Equal(double a, double b, double epsilon = 0.00001f)
		{
			return b <= (a + epsilon)
				&& b >= (a - epsilon)
				;
		}

		public static double Rad(double deg)
		{
			return deg * Math.PI / 180.0;
		}

		public static double Deg(double rad)
		{
			return rad * 180.0 / Math.PI;
		}
	}
}
