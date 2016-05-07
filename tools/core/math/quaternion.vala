/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gee;

namespace Crown
{
	public struct Quaternion
	{
		public double x;
		public double y;
		public double z;
		public double w;

		public Quaternion(double x, double y, double z, double w)
		{
			this.x = x;
			this.y = y;
			this.z = z;
			this.w = w;
		}

		public Quaternion.from_array(ArrayList<double?> arr)
		{
			this.x = arr[0];
			this.y = arr[1];
			this.z = arr[2];
			this.w = arr[3];
		}

		public Quaternion.from_axis_angle(Vector3 axis, float angle)
		{
			double ha = angle * 0.5;
			double sa = Math.sin(ha);
			double ca = Math.cos(ha);
			this.x = axis.x * sa;
			this.y = axis.y * sa;
			this.z = axis.z * sa;
			this.w = ca;
		}

		public Quaternion.from_euler(double rx, double ry, double rz)
		{
			// http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
			double c1 = Math.cos(ry*0.5);
			double s1 = Math.sin(ry*0.5);
			double c2 = Math.cos(rz*0.5);
			double s2 = Math.sin(rz*0.5);
			double c3 = Math.cos(rx*0.5);
			double s3 = Math.sin(rx*0.5);
			double c1c2 = c1*c2;
			double s1s2 = s1*s2;

			double nw = c1c2*c3 - s1s2*s3;
			double nx = c1c2*s3 + s1s2*c3;
			double ny = s1*c2*c3 + c1*s2*s3;
			double nz = c1*s2*c3 - s1*c2*s3;

			this.x = nx;
			this.y = ny;
			this.z = nz;
			this.w = nw;
		}

		public Vector3 to_euler()
		{
			// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
			double test = x*y + z*w;
			if (test > 0.499)
			{ // singularity at north pole
				double rx = 0.0;
				double ry = 2.0 * Math.atan2(x, w);
				double rz = Math.PI*0.5;
				return Vector3(rx, ry, rz);
			}
			if (test < -0.499)
			{ // singularity at south pole
				double rx = +0.0;
				double ry = -2.0 * Math.atan2(x, w);
				double rz = -Math.PI*0.5;
				return Vector3(rx, ry, rz);
			}
			double xx = x*x;
			double yy = y*y;
			double zz = z*z;

			double rrx = Math.atan2(2.0*x*w - 2.0*y*z, 1.0 - 2.0*xx - 2.0*zz);
			double rry = Math.atan2(2.0*y*w - 2.0*x*z, 1.0 - 2.0*yy - 2.0*zz);
			double rrz = Math.asin(2.0*test);

			return Vector3(rrx, rry, rrz);
		}

		public string to_string()
		{
			return "%f, %f, %f, %f".printf(x, y, z, w);
		}
	}
}
