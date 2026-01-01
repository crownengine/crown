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

} /* namespace MathUtils */

} /* namespace Crown */
