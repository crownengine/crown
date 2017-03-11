/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

namespace Crown
{
	namespace EngineApi
	{
		public string compile(Guid id, string data_dir, string platform)
		{
			return "{\"type\":\"compile\",\"id\":\"%s\",\"data_dir\":\"%s\",\"platform\":\"%s\"}".printf(id.to_string()
				, data_dir.replace("\\", "\\\\").replace("\"", "\\\"")
				, platform
				);
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
