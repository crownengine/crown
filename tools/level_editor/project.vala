/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	public class Project
	{
		// Data
		private string _source_dir;
		private string _toolchain_dir;
		private string _data_dir;
		private string _platform;

		public Project()
		{
			_source_dir = null;
			_toolchain_dir = null;
			_data_dir = null;
			_platform = "linux";
		}

		public void load(string source_dir, string toolchain_dir, string data_dir)
		{
			_source_dir = source_dir;
			_toolchain_dir = toolchain_dir;
			_data_dir = data_dir;
		}

		public string source_dir()
		{
			return _source_dir;
		}

		public string toolchain_dir()
		{
			return _toolchain_dir;
		}

		public string data_dir()
		{
			return _data_dir;
		}

		public string platform()
		{
			return _platform;
		}
	}
}
