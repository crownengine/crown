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
		public double x, y, z, w;

		public Quaternion(double x, double y, double z, double w)
		{
			this.x = x;
			this.y = y;
			this.z = z;
			this.w = w;
		}

		public Quaternion(ArrayList arr)
		{
			this.x = (double)arr[0];
			this.y = (double)arr[1];
			this.z = (double)arr[2];
			this.w = (double)arr[3];
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

		public static Quaternion FromEuler(double rx, double ry, double rz)
		{
			// http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
			double c1 = Math.Cos(ry*0.5);
			double s1 = Math.Sin(ry*0.5);
			double c2 = Math.Cos(rz*0.5);
			double s2 = Math.Sin(rz*0.5);
			double c3 = Math.Cos(rx*0.5);
			double s3 = Math.Sin(rx*0.5);
			double c1c2 = c1*c2;
			double s1s2 = s1*s2;

			double nw = c1c2*c3 - s1s2*s3;
			double nx = c1c2*s3 + s1s2*c3;
			double ny = s1*c2*c3 + c1*s2*s3;
			double nz = c1*s2*c3 - s1*c2*s3;

			return new Quaternion(nx, ny, nz, nw);
		}

		public Vector3 ToEuler()
		{
			// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
			double test = x*y + z*w;
			if (test > 0.499)
			{ // singularity at north pole
				double rx = 0.0;
				double ry = 2.0 * Math.Atan2(x, w);
				double rz = Math.PI*0.5;
				return new Vector3(rx, ry, rz);
			}
			if (test < -0.499)
			{ // singularity at south pole
				double rx = +0.0;
				double ry = -2.0 * Math.Atan2(x, w);
				double rz = -Math.PI*0.5;
				return new Vector3(rx, ry, rz);
			}
			double xx = x*x;
			double yy = y*y;
			double zz = z*z;

			double rrx = Math.Atan2(2.0*x*w - 2.0*y*z, 1.0 - 2.0*xx - 2.0*zz);
			double rry = Math.Atan2(2.0*y*w - 2.0*x*z, 1.0 - 2.0*yy - 2.0*zz);
			double rrz = Math.Asin(2.0*test);

			return new Vector3(rrx, rry, rrz);
		}

		public override string ToString()
		{
			return string.Format("{0}, {1}, {2}, {3}", x, y, z, w);
		}
	}
}
