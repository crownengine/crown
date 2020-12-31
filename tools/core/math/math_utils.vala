/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

namespace Crown
{
namespace MathUtils
{
	public bool equal(double a, double b, double epsilon = 0.00001f)
	{
		return b <= (a + epsilon)
			&& b >= (a - epsilon)
			;
	}

	public double rad(double deg)
	{
		return deg * Math.PI / 180.0;
	}

	public double deg(double rad)
	{
		return rad * 180.0 / Math.PI;
	}
}

}
