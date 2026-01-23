/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
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

	public double floor(double a)
	{
		return Math.floor(a);
	}

	public double fract(double a)
	{
		return a - floor(a);
	}

	public double lerp(double p0, double p1, double t)
	{
		return (1.0 - t) * p0 + t * p1;
	}

} /* namespace MathUtils */

public const double PI_TWO = Math.PI * 2.0;

} /* namespace Crown */
