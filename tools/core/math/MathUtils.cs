/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using System;

namespace Crown
{
	public static class MathUtils
	{
		public static bool Fequal(float a, float b, float epsilon = 0.00001f)
		{
			return b <= (a + epsilon)
				&& b >= (a - epsilon)
				;
		}

		public static float Frad(float deg)
		{
			return deg * (float)Math.PI / 180.0f;
		}

		public static float Fdeg(float rad)
		{
			return rad * 180.0f / (float)Math.PI;
		}
	}
}
