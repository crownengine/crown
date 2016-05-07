/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	/// <summary>
	/// Functions to encode Vala types to Lua.
	/// </summary>
	namespace Lua
	{
		public string bool(bool b)
		{
			return b == true ? "true" : "false";
		}

		public string vector2(Vector2 v)
		{
			return "Vector2(%f, %f)".printf(v.x, v.y);
		}

		public string vector3(Vector3 v)
		{
			return "Vector3(%f, %f, %f)".printf(v.x, v.y, v.z);
		}

		public string quaternion(Quaternion q)
		{
			return "Quaternion.from_elements(%f, %f, %f, %f)".printf(q.x, q.y, q.z, q.w);
		}
	}
}
