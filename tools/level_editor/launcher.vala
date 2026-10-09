/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public static GLib.GenericArray<GLib.Subprocess?> subprocesses = null;

[DBus (name = "org.crownengine.SubprocessLauncherError")]
public errordomain SubprocessLauncherError
{
	SUBPROCESS_NOT_FOUND,
	SUBPROCESS_CANCELLED
}

[DBus (name = "org.crownengine.SubprocessLauncher")]
public class SubprocessLauncherServer : Object
{
	public uint32 _subprocess_id;

	public SubprocessLauncherServer()
	{
		_subprocess_id = 0;
	}

	/// Spawns a subprocess and returns its ID which is *not* its PID.
	public uint32 spawnv_async(GLib.SubprocessFlags flags, string[] argv, string working_dir) throws GLib.SpawnError, GLib.Error
	{
		GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(flags);
		sl.set_cwd(working_dir);

		try {
			GLib.Subprocess subproc = sl.spawnv(argv);
			subproc.set_data<uint32>("id", _subprocess_id);

			subprocesses.add(subproc);
			logi("Created subprocess ID %u PID %s".printf(_subprocess_id
				, (string)subproc.get_identifier())
				);

			return _subprocess_id++;
		} catch (GLib.Error e) {
			throw e;
		}
	}

	/// Waits for @a process_id to terminate and returns its exit status.
	public int wait(uint32 process_id) throws GLib.Error
	{
		int ind = subprocess_index(process_id);

		if (ind == subprocesses.length)
			throw new SubprocessLauncherError.SUBPROCESS_NOT_FOUND("Process ID %u not found".printf(process_id));

		try {
			if (!subprocesses[ind].wait())
				throw new SubprocessLauncherError.SUBPROCESS_CANCELLED("Process ID %u was cancelled".printf(process_id));

			if (subprocesses[ind].get_if_exited()) {
				int exit_status = subprocesses[ind].get_exit_status();
				subprocesses.remove_index(ind);
				return exit_status;
			}
		} catch (GLib.Error e) {
			throw e;
		}

		return int.MAX;
	}

	/// Kills the process identified by @a process_id.
	public void kill(uint32 process_id) throws GLib.Error
	{
		int ind = subprocess_index(process_id);

		if (ind == subprocesses.length)
			throw new SubprocessLauncherError.SUBPROCESS_NOT_FOUND("Process ID %u not found".printf(process_id));

		subprocesses[ind].force_exit();
	}

	public int subprocess_index(uint32 process_id) throws GLib.Error
	{
		int ind;
		for (ind = 0; ind < subprocesses.length; ++ind) {
			uint32 id = subprocesses[ind].get_data<uint32>("id");
			if (id == process_id)
				break;
		}

		return ind;
	}
}

[DBus (name = "org.crownengine.SubprocessLauncher")]
public interface SubprocessLauncher : Object
{
	public abstract uint32 spawnv_async(GLib.SubprocessFlags flags, string[] argv, string working_dir) throws GLib.SpawnError, GLib.Error;
	// 7200000 = 2 hours = 2*3600*1000 ms.
	[DBus (timeout = 7200000)]
	public abstract int wait(uint32 process_id) throws GLib.Error;
	public abstract void kill(uint32 process_id) throws GLib.Error;
}

public static SubprocessLauncher _subprocess_launcher;

public bool connect_subprocess_launcher(out GLib.Error? error)
{
	error = null;

	try {
		_subprocess_launcher = GLib.Bus.get_proxy_sync(GLib.BusType.SESSION
			, CROWN_SUBPROCESS_LAUNCHER
			, "/org/crownengine/subprocess_launcher"
			);
		return true;
	} catch (GLib.Error e) {
		error = e;
		return false;
	}
}

public async int wait_subprocess(uint32 process_id) throws GLib.Error
{
	SourceFunc callback = wait_subprocess.callback;
	int exit_status = int.MAX;
	GLib.Error? error = null;

	new Thread<int>("wait-subprocess", () => {
			try {
				exit_status = _subprocess_launcher.wait(process_id);
			} catch (GLib.Error e) {
				error = e;
			}

			GLib.Idle.add((owned)callback);
			return 0;
		});

	yield;

	if (error != null)
		throw error;

	return exit_status;
}

public static void on_bus_acquired(GLib.DBusConnection conn)
{
	try {
		conn.register_object("/org/crownengine/subprocess_launcher", new SubprocessLauncherServer());
	} catch (GLib.IOError e) {
		logi("Could not register DBus service.");
	}
}

public static void on_name_acquired(GLib.DBusConnection conn, string name)
{
	logi("DBus name acquired: %s".printf(name));
}

public static void on_name_lost(GLib.DBusConnection conn, string name)
{
	logi("DBus name lost: %s".printf(name));
}

public static GLib.MainLoop loop = null;

public static void wait_async_ready_callback(Object? source_object, AsyncResult res)
{
	GLib.Subprocess subproc = (GLib.Subprocess)source_object;

	try {
		subproc.wait_async.end(res);

		uint32 subproc_id = subproc.get_data<uint32>("id");

		if (subproc.get_if_exited()) {
			int ec = subproc.get_exit_status();
			logi("Process ID %u exited with status %d".printf(subproc_id, ec));
		} else {
			logi("Process ID %u exited abnormally".printf(subproc_id));
		}
	} catch (GLib.Error e) {
		if (e.code == 19) {
			subproc.force_exit();
			// Assume subproc is dead now.
		} else {
			loge(e.message);
		}
	}

	// The last subprocess to exit quits the app.
	subprocesses.remove(subproc);
	if (subprocesses.length == 0) {
		loop.quit();
	}
}

public static void monitored_process_exited(GLib.Pid pid, int wait_status)
{
	// The monitord process exited.
	// Terminate any leftover subprocesses it spawned.
	uint wait_timer_id = 0;
	GLib.Cancellable cancellable = new GLib.Cancellable();
	if (subprocesses.length > 0) {
		// Spawn a watchdog to cancel all wait_async() after a while.
		uint interval_ms = 500;
		wait_timer_id = GLib.Timeout.add(interval_ms, () => {
				cancellable.cancel();
				return GLib.Source.REMOVE;
			});

		logi("Waiting %ums for %d subprocesses to terminate...".printf(interval_ms, subprocesses.length));
	}

	if (wait_timer_id > 0) {
		// Wait asynchronously for children to terminate.
		for (int i = 0; i < subprocesses.length; ++i)
			subprocesses[i].wait_async.begin(cancellable, wait_async_ready_callback);
	} else {
		// Force children to exit.
		for (int i = 0; i < subprocesses.length; ++i)
			subprocesses[i].force_exit();

		loop.quit();
	}
}

public static int launcher_main(string[] args)
{
	loop = new GLib.MainLoop(null, false);
	subprocesses = new GLib.GenericArray<GLib.Subprocess?>();
	GLib.Pid monitored_pid = 0;

	for (int i = 0; i < args.length; ++i) {
		if (args[i] == "--launcher") {
			monitored_pid = (GLib.Pid)int.parse(args[i + 1]);
			break;
		}
	}

#if CROWN_PLATFORM_LINUX
	// Signal handlers.
	GLib.Unix.signal_add(Posix.Signal.INT, () => {
			if (monitored_pid != 0)
				Posix.kill(monitored_pid, Posix.Signal.INT);
			// Try to not terminate prior to the watched process.
			return GLib.Source.CONTINUE;
		});
	GLib.Unix.signal_add(Posix.Signal.TERM, () => {
			if (monitored_pid != 0)
				Posix.kill(monitored_pid, Posix.Signal.TERM);
			// Try to not terminate prior to the watched process.
			return GLib.Source.CONTINUE;
		});
#endif

	// Connect to DBus.
	GLib.Bus.own_name(GLib.BusType.SESSION
		, "org.crownengine.SubprocessLauncher"
		, GLib.BusNameOwnerFlags.NONE
		, on_bus_acquired
		, on_name_acquired
		, on_name_lost
		);

	uint event_source = 0;
	if (monitored_pid != 0) {
		// It is unclear whether ChildWatch is intended to work with PIDs
		// not coming from GLib.Process.spawn*(). It seems to work fine regardless
		// but we get a suspicious warning at exit. We might have to replace it
		// with something else in the future:
		//
		// WARN launcher: ../glib/glib/gmain.c:5933: waitid(pid:988061, pidfd=8) failed: No child processes (10).
		event_source = GLib.ChildWatch.add(monitored_pid, monitored_process_exited);
	}

	if (event_source <= 0) {
		loge("Failed to start monitoring PID %d".printf(monitored_pid));
		return -1;
	}

	loop.run();
	return 0;
}

public bool parse_port_from_string(out uint16 port, string str)
{
	port = 0;

	string trimmed = str.strip();
	if (trimmed == "")
		return false;

	for (int i = 0; i < trimmed.length; ++i) {
		if (trimmed[i] < '0' || trimmed[i] > '9')
			return false;
	}

	int parsed = int.parse(trimmed);
	if (parsed < 1 || parsed > 65535)
		return false;

	port = (uint16)parsed;
	return true;
}

public bool wait_port_file(out uint16 port, string file_path, int num_tries, int interval)
{
	port = 0;
	for (int tries = 0; tries < num_tries; ++tries) {
		try {
			string contents = null;
			GLib.FileUtils.get_contents(file_path, out contents);
			if (parse_port_from_string(out port, contents))
				return true;
		} catch (FileError e) {
		}

		GLib.Thread.usleep(interval*1000);
	}

	return false;
}

public bool create_port_file_path(out string file_path)
{
	file_path = "";

	try {
		GLib.FileIOStream io;
		file_path = GLib.File.new_tmp("crown_port_file_XXXXXX", out io).get_path();
		return true;
	} catch (Error e) {
		loge(e.message);
		return false;
	}
}

public void cleanup_port_file_path(string file_path)
{
	try {
		GLib.File.new_for_path(file_path).delete();
	} catch (Error e) {
		// Ignore.
	}
}

public class RuntimeInstance
{
	public const int QUIT_TIMEOUT_MS = 4000;

	public string _name;
	public uint32 _process_id;
	public bool _stuck;
	public uint _revision;
	public GLib.SourceFunc _stop_callback;
	public GLib.SourceFunc _refresh_callback;
	public bool _refresh_success;
	public ConsoleClient _client;
	public DataCompiler? _data_compiler;

	public signal void connected(RuntimeInstance ri, string address, int port);
	public signal void disconnected(RuntimeInstance ri);
	public signal void disconnected_unexpected(RuntimeInstance ri);
	public signal void message_received(RuntimeInstance ri, ConsoleClient client, uint8[] json);

	public RuntimeInstance(string name, DataCompiler? dc)
	{
		_name = name;
		_process_id = uint32.MAX;
		_stuck = false;
		_revision = 0;
		_stop_callback = null;
		_refresh_callback = null;
		_refresh_success = false;
		_client = new ConsoleClient();
		_client.connected.connect(on_client_connected);
		_client.message_received.connect(on_client_message_received);
		_data_compiler = dc;
	}

	public void on_client_connected(string address, int port)
	{
		if (_data_compiler != null)
			_revision = _data_compiler._revision;

		connected(this, address, port);
	}

	public void on_client_disconnected()
	{
		disconnected(this);

		if (_stop_callback != null)
			_stop_callback();
	}

	public void on_client_disconnected_unexpected()
	{
		disconnected_unexpected(this);

		try {
			if (_process_id != uint32.MAX) {
				_subprocess_launcher.wait(_process_id);
				_process_id = uint32.MAX;
			}
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	public void on_client_message_received(ConsoleClient client, uint8[] json)
	{
		message_received(this, client, json);
	}

	// Tries to connect to the @a client. Return the number of tries after
	// it succeeded or @a num_tries if failed.
	public async int connect_async(string address, int port, int num_tries, int interval)
	{
		// It is an error if the client disconnects after here.
		_client.disconnected.disconnect(on_client_disconnected);
		_client.disconnected.connect(on_client_disconnected_unexpected);

		// Try to connect to the client.
		int tries;
		for (tries = 0; tries < num_tries; ++tries) {
			_client.connect(address, port);
			if (_client.is_connected())
				break;

			GLib.Thread.usleep(interval*1000);
		}
		return tries;
	}

	public async void stop()
	{
		if (_client != null) {
			// Reset "disconnected" signal.
			_client.disconnected.disconnect(on_client_disconnected);
			_client.disconnected.disconnect(on_client_disconnected_unexpected);

			// Explicit call to this function should not produce error messages.
			_client.disconnected.connect(on_client_disconnected);

			if (_client.is_connected()) {
				_stop_callback = stop.callback;
				_client.send(RuntimeApi.quit());

				// Call it stuck if not disconnected before a while.
				GLib.Timeout.add_full(GLib.Priority.HIGH, QUIT_TIMEOUT_MS, () => {
						if (_stop_callback != null) {
							_stuck = true;
							_stop_callback();
						}

						return GLib.Source.REMOVE;
					});

				yield; // Wait for _client to disconnect.
				_stop_callback = null;
			}
		}

		try {
			if (_process_id != uint32.MAX) {
				if (_stuck) {
					_subprocess_launcher.kill(_process_id);
					_stuck = false;
					logw("Process %u took more than %d ms to quit: killed".printf(_process_id, QUIT_TIMEOUT_MS));
				}

				_subprocess_launcher.wait(_process_id);
			}
			_process_id = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	public void send(string json)
	{
		_client.send(json);
	}

	public void send_script(string lua)
	{
		_client.send_script(lua);
	}

	public bool is_connected()
	{
		return _client.is_connected();
	}

	public async bool refresh(DataCompiler dc)
	{
		if (_refresh_callback != null)
			return false;

		if (!is_connected())
			return false;

		var compiler_revision = dc._revision;

		if (_revision != compiler_revision) {
			var refresh_list = yield dc.refresh_list(_revision);
			_client.send(DeviceApi.refresh(refresh_list));
			_client.send(DeviceApi.frame());
			_refresh_callback = refresh.callback;
			yield; // Wait for client to refresh the resources.

			if (_refresh_success)
				_revision = compiler_revision;

			return _refresh_success;
		}

		return true;
	}

	public void refresh_finished(bool success)
	{
		_refresh_success = success;
		if (_refresh_callback != null)
			_refresh_callback();
		_refresh_callback = null;
	}
}

public void open_directory(string directory)
{
#if CROWN_PLATFORM_LINUX
	try {
		GLib.AppInfo.launch_default_for_uri("file://" + directory, Gdk.Display.get_default().get_app_launch_context());
	} catch (Error e) {
		loge(e.message);
	}
#else
	GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(subprocess_flags());
	try {
		sl.spawnv({ "explorer.exe", directory, null });
	} catch (Error e) {
		loge(e.message);
	}
#endif
}

public void open_text_editor(string path)
{
#if CROWN_PLATFORM_WINDOWS
	GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(subprocess_flags());
	try {
		sl.spawnv({ "notepad.exe", path, null });
	} catch (Error e) {
		loge(e.message);
	}
#endif
}

public static GLib.SubprocessFlags subprocess_flags()
{
	GLib.SubprocessFlags flags = SubprocessFlags.NONE;
#if !CROWN_DEBUG
	flags |= SubprocessFlags.STDOUT_SILENCE | SubprocessFlags.STDERR_SILENCE;
#endif
	return flags;
}

} /* namespace Crown */
