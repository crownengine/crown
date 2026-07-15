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
	public SourceFunc _dependencies_callback;
	public SourceFunc _delete_preview_callback;
	public SourceFunc _delete_apply_callback;
	public SourceFunc _move_preview_callback;
	public SourceFunc _move_apply_callback;
	public GLib.GenericArray<Value?> _refresh_list;
	public GLib.HashTable<string, Value?> _dependencies;
	public GLib.HashTable<string, Value?> _delete_preview;
	public GLib.HashTable<string, Value?> _delete_apply;
	public GLib.HashTable<string, Value?> _move_preview;
	public GLib.HashTable<string, Value?> _move_apply;
	public uint _revision;

	public signal void start();
	public signal void finished(bool success);

	public void message(GLib.HashTable<string, Value?> msg)
	{
		// Guid id = Guid.parse((string)msg["id"]);

		if (msg.contains("start")) {
			start();
		} else if (msg.contains("success")) {
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
		_dependencies_callback = null;
		_delete_preview_callback = null;
		_delete_apply_callback = null;
		_move_preview_callback = null;
		_move_apply_callback = null;
		_refresh_list = null;
		_dependencies = null;
		_delete_preview = null;
		_delete_apply = null;
		_move_preview = null;
		_move_apply = null;
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
	public async GLib.GenericArray<Value?> refresh_list(uint since_revision)
	{
		if (_refresh_list_callback != null)
			return new GLib.GenericArray<Value?>();

		_runtime.send(DataCompilerApi.refresh_list(since_revision));
		_refresh_list_callback = refresh_list.callback;
		yield;

		return _refresh_list;
	}

	public void refresh_list_finished(GLib.GenericArray<Value?> resources)
	{
		unowned GLib.SourceFunc callback = _refresh_list_callback;
		_refresh_list_callback = null;
		_refresh_list = resources;

		if (callback != null)
			callback();
	}

	public async GLib.HashTable<string, Value?> dependencies(string path)
	{
		if (_dependencies_callback != null)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		_runtime.send(DataCompilerApi.dependencies(path));
		_dependencies_callback = dependencies.callback;
		yield;

		return _dependencies;
	}

	public void dependencies_finished(GLib.HashTable<string, Value?> dependencies)
	{
		unowned GLib.SourceFunc callback = _dependencies_callback;
		_dependencies_callback = null;
		_dependencies = dependencies;

		if (callback != null)
			callback();
	}

	public async GLib.HashTable<string, Value?> delete_preview(string[] paths)
	{
		if (_delete_preview_callback != null)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		_runtime.send(DataCompilerApi.delete_preview(paths));
		_delete_preview_callback = delete_preview.callback;
		yield;

		return _delete_preview;
	}

	public void delete_preview_finished(GLib.HashTable<string, Value?> preview)
	{
		unowned GLib.SourceFunc callback = _delete_preview_callback;
		_delete_preview_callback = null;
		_delete_preview = preview;

		if (callback != null)
			callback();
	}

	public async GLib.HashTable<string, Value?> delete_apply(string[] paths, string[]? prune_dirs = null)
	{
		if (_delete_apply_callback != null)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		_runtime.send(DataCompilerApi.delete_apply(paths, prune_dirs));
		_delete_apply_callback = delete_apply.callback;
		yield;

		return _delete_apply;
	}

	public void delete_apply_finished(GLib.HashTable<string, Value?> apply)
	{
		unowned GLib.SourceFunc callback = _delete_apply_callback;
		_delete_apply_callback = null;
		_delete_apply = apply;

		if (callback != null)
			callback();
	}

	public async GLib.HashTable<string, Value?> move_preview(string[] from, string[] to)
	{
		if (_move_preview_callback != null)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		_runtime.send(DataCompilerApi.move_preview(from, to));
		_move_preview_callback = move_preview.callback;
		yield;

		return _move_preview;
	}

	public void move_preview_finished(GLib.HashTable<string, Value?> preview)
	{
		unowned GLib.SourceFunc callback = _move_preview_callback;
		_move_preview_callback = null;
		_move_preview = preview;

		if (callback != null)
			callback();
	}

	public async GLib.HashTable<string, Value?> move_apply(string[] from, string[] to, string[]? prune_dirs = null)
	{
		if (_move_apply_callback != null)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		_runtime.send(DataCompilerApi.move_apply(from, to, prune_dirs));
		_move_apply_callback = move_apply.callback;
		yield;

		return _move_apply;
	}

	public void move_apply_finished(GLib.HashTable<string, Value?> apply)
	{
		unowned GLib.SourceFunc callback = _move_apply_callback;
		_move_apply_callback = null;
		_move_apply = apply;

		if (callback != null)
			callback();
	}
}

} /* namespace Crown */
