/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	namespace EngineAPI
	{
		public string compile(Guid id, string bundle_dir, string platform)
		{
			return "{\"type\":\"compile\",\"id\":\"%s\",\"bundle_dir\":\"%s\",\"platform\":\"%s\"}".printf(id.to_string(), bundle_dir, platform);
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
