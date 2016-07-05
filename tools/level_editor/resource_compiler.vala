/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	public class ResourceCompiler
	{
		private ConsoleClient _compiler;
		private Guid _id;
		private bool _success;
		private SourceFunc _callback;

		public ResourceCompiler(ConsoleClient client)
		{
			_compiler = client;
			_id = GUID_ZERO;
			_success = false;
			_callback = null;
		}

		// Returns true if success, false otherwise.
		public async bool compile(string bundle_dir, string platform)
		{
			if (_callback != null)
				return false;

			_id = Guid.new_guid();
			_success = false;
			_compiler.send(EngineAPI.compile(_id, bundle_dir, platform));
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
