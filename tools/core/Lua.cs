/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using System;

namespace Crown
{
	/// <summary>
	/// Functions to encode C# types to Lua.
	/// </summary>
	public static class Lua
	{
		public static string FromBool(bool b)
		{
			return b == true ? "true" : "false";
		}

		public static string FromVector2(Vector2 v)
		{
			return string.Format("Vector2({0}, {1})", v.x, v.y);
		}

		public static string FromVector3(Vector3 v)
		{
			return string.Format("Vector3({0}, {1}, {2})", v.x, v.y, v.z);
		}

		public static string FromQuaternion(Quaternion q)
		{
			return string.Format("Quaternion.from_elements({0}, {1}, {2}, {3})", q.x, q.y, q.z, q.w);
		}
	}
}
