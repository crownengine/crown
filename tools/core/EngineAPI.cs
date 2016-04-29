/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using System;

namespace Crown
{
	public static class EngineAPI
	{
		public static string Compile(string type, string name, string platform)
		{
			return string.Format(@"{{""type"":""compile"","
				+ @"""resource_type"":""{0}"","
				+ @"""resource_name"":""{1}"","
				+ @"""platform"":""{2}""}}"
				, type
				, name
				, platform
				);
		}

		public static string Reload(string type, string name)
		{
			return string.Format(@"{{""type"":""reload"","
				+ @"""resource_type"":""{0}"","
				+ @"""resource_name"":""{1}""}}"
				, type
				, name
				);
		}

		public static string Pause()
		{
			return @"{""type"":""pause""}";
		}

		public static string Unpause()
		{
			return @"{""type"":""unpause""}";
		}
	}
}
