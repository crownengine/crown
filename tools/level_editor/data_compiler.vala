/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class DataCompiler
{
	public RuntimeInstance _runtime;
	public Guid _id;
	public bool _success;
	public SourceFunc _compile_callback;
	public SourceFunc _refresh_list_callback;
	public Gee.ArrayList<Value?> _refresh_list;
	public uint _revision;

	public signal void start();
	public signal void finished(bool success);

	public void message(Hashtable msg)
	{
		// Guid id = Guid.parse((string)msg["id"]);

		if (msg.has_key("start")) {
			start();
		} else if (msg.has_key("success")) {
			compile_finished((bool)msg["success"], (uint)(double)msg["revision"]);
		}
	}

	public DataCompiler(RuntimeInstance runtime)
	{
		_runtime = runtime;
		_id = GUID_ZERO;
		_success = false;
		_revision = 0;
		_compile_callback = null;
		_refresh_list_callback = null;
		_refresh_list = null;
	}

	// Returns true if success, false otherwise.
	public async bool compile(string data_dir, string platform)
	{
		if (_compile_callback != null)
			return false;

		_id = Guid.new_guid();
		_success = false;
		_runtime.send(DataCompilerApi.compile(_id, data_dir, platform));
		_compile_callback = compile.callback;
		yield;

		finished(_success);
		return _success;
	}

	public void compile_finished(bool success, uint revision)
	{
		_success = success;
		_revision = revision;
		if (_compile_callback != null)
			_compile_callback();
		_compile_callback = null;
	}

	/// Returns the list of resources that have changed since @a since_revision.
	public async Gee.ArrayList<Value?> refresh_list(uint since_revision)
	{
		if (_refresh_list_callback != null)
			return new Gee.ArrayList<Value?>();

		_runtime.send(DataCompilerApi.refresh_list(since_revision));
		_refresh_list_callback = refresh_list.callback;
		yield;

		return _refresh_list;
	}

	public void refresh_list_finished(Gee.ArrayList<Value?> resources)
	{
		unowned GLib.SourceFunc callback = _refresh_list_callback;
		_refresh_list_callback = null;
		_refresh_list = resources;

		if (callback != null)
			callback();
	}
}

} /* namespace Crown */
