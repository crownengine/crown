/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	namespace EngineAPI
	{
		public string compile(string type, string name, string platform)
		{
			return "{\"type\":\"compile\",\"resource_type\":\"%s\",\"resource_name\":\"%s\",\"platform\":\"%s\"}".printf(type, name, platform);
		}

		public string reload(string type, string name)
		{
			return "{\"type\":\"reload\",\"resource_type\":\"%s\",\"resource_name\":\"%s\"}".printf(type, name);
		}

		public string pause()
		{
			return "{\"type\":\"pause\"}";
		}

		public string unpause()
		{
			return "{\"type\":\"unpause\"}";
		}
	}
}
