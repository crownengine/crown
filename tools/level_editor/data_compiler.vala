/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

namespace Crown
{
	public class DataCompiler
	{
		private ConsoleClient _compiler;
		private Guid _id;
		private bool _success;
		private SourceFunc _callback;

		public DataCompiler(ConsoleClient client)
		{
			_compiler = client;
			_id = GUID_ZERO;
			_success = false;
			_callback = null;
		}

		// Returns true if success, false otherwise.
		public async bool compile(string data_dir, string platform)
		{
			if (_callback != null)
				return false;

			_id = Guid.new_guid();
			_success = false;
			_compiler.send(DataCompilerApi.compile(_id, data_dir, platform));
			_callback = compile.callback;
			yield;

			return _success;
		}

		public void finished(bool success)
		{
			_success = success;
			_callback();
			_callback = null;
		}
	}
}
