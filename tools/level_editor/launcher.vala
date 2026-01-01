/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public static Gee.ArrayList<GLib.Subprocess?> subprocesses = null;

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
			logi("Created suprocess ID %u PID %s".printf(_subprocess_id
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
		int ii;
		for (ii = 0; ii < subprocesses.size; ++ii) {
			uint32 id = subprocesses[ii].get_data<uint32>("id");
			if (id == process_id)
				break;
		}

		if (ii == subprocesses.size)
			throw new SubprocessLauncherError.SUBPROCESS_NOT_FOUND("Process ID %u not found".printf(process_id));

		try {
			if (!subprocesses[ii].wait())
				throw new SubprocessLauncherError.SUBPROCESS_CANCELLED("Process ID %u was cancelled".printf(process_id));

			if (subprocesses[ii].get_if_exited()) {
				int exit_status = subprocesses[ii].get_exit_status();
				subprocesses.remove_at(ii);
				return exit_status;
			}
		} catch (GLib.Error e) {
			throw e;
		}

		return int.MAX;
	}
}

[DBus (name = "org.crownengine.SubprocessLauncher")]
public interface SubprocessLauncher : Object
{
	public abstract uint32 spawnv_async(GLib.SubprocessFlags flags, string[] argv, string working_dir) throws GLib.SpawnError, GLib.Error;
	public abstract int wait(uint32 process_id) throws GLib.Error;
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
	if (subprocesses.size == 0) {
		loop.quit();
	}
}

public static void monitored_process_exited(GLib.Pid pid, int wait_status)
{
	// The monitord process exited.
	// Terminate any leftover subprocesses it spawned.
	uint wait_timer_id = 0;
	GLib.Cancellable cancellable = new GLib.Cancellable();
	if (subprocesses.size > 0) {
		// Spawn a watchdog to cancel all wait_async() after a while.
		uint interval_ms = 500;
		wait_timer_id = GLib.Timeout.add(interval_ms, () => {
				cancellable.cancel();
				return GLib.Source.REMOVE;
			});

		logi("Waiting %ums for %d subprocesses to terminate...".printf(interval_ms, subprocesses.size));
	}

	if (wait_timer_id > 0) {
		// Wait asynchronously for children to terminate.
		foreach (var subproc in subprocesses)
			subproc.wait_async.begin(cancellable, wait_async_ready_callback);
	} else {
		// Force children to exit.
		foreach (var subproc in subprocesses)
			subproc.force_exit();

		loop.quit();
	}
}

public static int launcher_main(string[] args)
{
	loop = new GLib.MainLoop(null, false);
	subprocesses = new Gee.ArrayList<GLib.Subprocess?>();
	GLib.Pid monitored_pid = 0;

	for (int i = 0; i < args.length; ++i) {
		if (args[i] == "--launcher") {
			monitored_pid = int.parse(args[i + 1]);
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

} /* namespace Crown */
