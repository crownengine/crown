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

		public string command(string[] args)
		{
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < args.length; ++i)
			{
				string arg = args[i].replace("\\", "\\\\").replace("\"", "\\\"");
				sb.append("\"%s\",".printf(arg));
			}

			return "{\"type\":\"command\",\"args\":[%s]}".printf(sb.str);
		}

		public string reload(string type, string name)
		{
			return command({ "reload", type, name });
		}

		public string pause()
		{
			return command({ "pause" });
		}

		public string unpause()
		{
			return command({ "unpause" });
		}
	}
}
