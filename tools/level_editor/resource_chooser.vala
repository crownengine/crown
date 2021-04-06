/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
// Returns true if the item should be filtered out
private bool user_filter(string type, string name)
{
	return (type == "unit" || type == "sound") && !name.has_prefix("core/");
}

public delegate bool UserFilter(string type, string name);

public class ResourceChooser : Gtk.Box
{
	// Data
	public Project _project;
	public GLib.Subprocess _editor_process;
	public ConsoleClient _editor;
	private GLib.SourceFunc _stop_editor_callback = null;
	public Gtk.ListStore _list_store;
	public bool _preview;
	public unowned UserFilter _user_filter;
	public string _name;

	// Widgets
	public EntrySearch _filter_entry;
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.ScrolledWindow _scrolled_window;
	public Gtk.Label _no_preview_label;
	public Gtk.Label _disconnected_label;
	public Gtk.Label _oops_label;
	public Gtk.Stack _editor_stack;
	public EditorView _editor_view;

	// Signals
	public signal void resource_selected(string type, string name);

	public ResourceChooser(Project? project, ProjectStore project_store, bool preview)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_project = project;

		_editor = new ConsoleClient();
		_editor.connected.connect(on_editor_connected);
		_editor.message_received.connect(on_editor_message_received);

		_list_store = project_store._list_store;
		_preview = preview;
		_user_filter = user_filter;

		// Widgets
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);
		_filter_entry.key_press_event.connect(on_filter_entry_key_pressed);

		_tree_filter = new Gtk.TreeModelFilter(_list_store, null);
		_tree_filter.set_visible_func((model, iter) => {
			Value type;
			Value name;
			model.get_value(iter, ProjectStore.Column.TYPE, out type);
			model.get_value(iter, ProjectStore.Column.NAME, out name);

			string type_str = (string)type;
			string name_str = (string)name;

			return type_str != null
				&& name_str != null
				&& _user_filter(type_str, name_str)
				&& (_filter_entry.text.length == 0 || name_str.index_of(_filter_entry.text) > -1)
				;
		});

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
			Value id_a;
			Value id_b;
			model.get_value(iter_a, ProjectStore.Column.NAME, out id_a);
			model.get_value(iter_b, ProjectStore.Column.NAME, out id_b);
			return strcmp((string)id_a, (string)id_b);
		});

		_tree_view = new Gtk.TreeView();
		_tree_view.insert_column_with_attributes(-1
			, "Name"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.NAME
			, null
			);
/*
		// Debug
		_tree_view.insert_column_with_attributes(-1
			, "Type"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.TYPE
			, null
			);
*/
		_tree_view.model = _tree_sort;
		_tree_view.headers_visible = false;
		_tree_view.can_focus = false;
		_tree_view.row_activated.connect(on_row_activated);
		_tree_view.button_release_event.connect(on_button_released);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);
		_tree_selection.changed.connect(on_tree_selection_changed);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);
		_scrolled_window.set_size_request(300, 400);

		_editor_stack = new Gtk.Stack();

		this.pack_start(_filter_entry, false, true, 0);
		this.pack_start(_editor_stack, true, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);

		_no_preview_label = new Gtk.Label("No Preview");
		_no_preview_label.set_size_request(300, 300);
		_editor_stack.add(_no_preview_label);
		_disconnected_label = new Gtk.Label("Disconnected");
		_editor_stack.add(_disconnected_label);
		_oops_label = new Gtk.Label(null);
		_oops_label.set_markup("Something went wrong.\rTry to <a href=\"restart\">restart</a> this view.");
		_oops_label.activate_link.connect(() => {
			restart_editor.begin((obj, res) => {
				restart_editor.end(res);
			});
			return true;
		});
		_editor_stack.add(_oops_label);

		this.destroy.connect(on_destroy);
		this.unmap.connect(on_unmap);

		restart_editor.begin((obj, res) => {
			restart_editor.end(res);
		});
	}

	private void on_row_activated(Gtk.TreePath path, TreeViewColumn column)
	{
		Gtk.TreePath filter_path = _tree_sort.convert_path_to_child_path(path);
		Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(filter_path);
		Gtk.TreeIter iter;
		if (_list_store.get_iter(out iter, child_path))
		{
			Value name;
			Value type;
			_list_store.get_value(iter, ProjectStore.Column.NAME, out name);
			_list_store.get_value(iter, ProjectStore.Column.TYPE, out type);
			_name = (string)name;
			resource_selected((string)type, (string)name);
		}
	}

	private bool on_button_released(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_PRIMARY)
		{
			Gtk.TreePath path;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null))
			{
				if (_tree_view.get_selection().path_is_selected(path))
				{
					Gtk.TreePath filter_path = _tree_sort.convert_path_to_child_path(path);
					Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(filter_path);
					Gtk.TreeIter iter;
					if (_list_store.get_iter(out iter, child_path))
					{
						Value name;
						Value type;
						_list_store.get_value(iter, ProjectStore.Column.NAME, out name);
						_list_store.get_value(iter, ProjectStore.Column.TYPE, out type);
						_name = (string)name;
						resource_selected((string)type, (string)name);
					}
				}
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private void on_destroy()
	{
		stop_editor.begin((obj, res) => {
			stop_editor.end(res);
		});
	}

	private void on_unmap()
	{
		_filter_entry.text = "";
	}

	private async void start_editor(uint window_xid)
	{
		if (window_xid == 0)
			return;

		// Spawn unit_preview.
		string args[] =
		{
			ENGINE_EXE
			, "--data-dir"
			, _project.data_dir()
			, "--boot-dir"
			, UNIT_PREVIEW_BOOT_DIR
			, "--parent-window"
			, window_xid.to_string()
			, "--console-port"
			, UNIT_PREVIEW_TCP_PORT.to_string()
			, "--wait-console"
			, "--pumped"
			, null
		};
		GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(SubprocessFlags.NONE);
		sl.set_cwd(ENGINE_DIR);
		try
		{
			_editor_process = sl.spawnv(args);
		}
		catch (Error e)
		{
			loge(e.message);
		}

		// It is an error if the unit_preview disconnects after here.
		_editor.disconnected.disconnect(on_editor_disconnected);
		_editor.disconnected.connect(on_editor_disconnected_unexpected);

		// Try to connect to unit_preview.
		int tries = yield _editor.connect_async(UNIT_PREVIEW_ADDRESS
			, UNIT_PREVIEW_TCP_PORT
			, EDITOR_CONNECTION_TRIES
			, EDITOR_CONNECTION_INTERVAL
			);
		if (tries == EDITOR_CONNECTION_TRIES)
		{
			loge("Cannot connect to unit_preview.");
			return;
		}

		_tree_view.set_cursor(new Gtk.TreePath.first(), null, false);
	}

	public async void stop_editor()
	{
		if (!_preview)
			return;

		if (_editor != null)
		{
			// Reset "disconnected" signal.
			_editor.disconnected.disconnect(on_editor_disconnected);
			_editor.disconnected.disconnect(on_editor_disconnected_unexpected);

			// Explicit call to this function should not produce error messages.
			_editor.disconnected.connect(on_editor_disconnected);

			if (_editor.is_connected())
			{
				_stop_editor_callback = stop_editor.callback;
				_editor.send_script("Device.quit()");
				yield; // Wait for ConsoleClient to disconnect.
				_stop_editor_callback = null;
				_editor_stack.set_visible_child(_disconnected_label);
			}
		}

		int exit_status;
		wait_process(out exit_status, _editor_process);
		_editor_process = null;
	}

	public async void restart_editor()
	{
		if (!_preview)
			return;

		yield stop_editor();

		if (_editor_view != null)
		{
			_editor_stack.remove(_editor_view);
			_editor_view = null;
		}

		_editor_view = new EditorView(_editor, false);
		_editor_view.set_size_request(300, 300);
		_editor_view.realized.connect(on_editor_view_realized);
		_editor_view.show_all();

		_editor_stack.add(_editor_view);
		_editor_stack.set_visible_child(_editor_view);
	}

	private void on_editor_connected(string address, int port)
	{
		logi("Connected to preview@%s:%d".printf(address, port));

		// Start receiving data from the editor view.
		_editor.receive_async();
	}

	private void on_editor_disconnected()
	{
		logi("Disconnected from preview");

		if (_stop_editor_callback != null)
			_stop_editor_callback();
	}

	private void on_editor_disconnected_unexpected()
	{
		logw("Disconnected from preview unexpectedly");
		int exit_status;
		wait_process(out exit_status, _editor_process);
		_editor_process = null;

		_editor_stack.set_visible_child(_oops_label);
	}

	private void on_editor_message_received(ConsoleClient client, uint8[] json)
	{
		// Ignore the message content.
		client.receive_async();
	}

	private void on_editor_view_realized()
	{
		start_editor.begin(_editor_view.window_id);
	}

	private void on_filter_entry_text_changed()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_filter.refilter();
		_tree_selection.changed.connect(on_tree_selection_changed);
		_tree_view.set_cursor(new Gtk.TreePath.first(), null, false);
	}

	private bool on_filter_entry_key_pressed(Gdk.EventKey ev)
	{
		Gtk.TreeModel model;
		Gtk.TreeIter iter;
		bool selected = _tree_selection.get_selected(out model, out iter);

		if (ev.keyval == Gdk.Key.Down)
		{
			if (selected && model.iter_next(ref iter))
			{
				_tree_selection.select_iter(iter);
				_tree_view.scroll_to_cell(model.get_path(iter), null, true, 1.0f, 0.0f);
			}

			return Gdk.EVENT_STOP;
		}
		else if (ev.keyval == Gdk.Key.Up)
		{
			if (selected && model.iter_previous(ref iter))
			{
				_tree_selection.select_iter(iter);
				_tree_view.scroll_to_cell(model.get_path(iter), null, true, 1.0f, 0.0f);
			}

			return Gdk.EVENT_STOP;
		}
		else if (ev.keyval == Gdk.Key.Return)
		{
			if (selected)
			{
				Value name;
				Value type;
				model.get_value(iter, ProjectStore.Column.NAME, out name);
				model.get_value(iter, ProjectStore.Column.TYPE, out type);
				_name = (string)name;
				resource_selected((string)type, (string)name);
			}

			return Gdk.EVENT_STOP;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private void on_tree_selection_changed()
	{
		if (_preview)
		{
			Gtk.TreeModel model;
			Gtk.TreeIter iter;
			if (_tree_selection.get_selected(out model, out iter))
			{
				Value name;
				Value type;
				model.get_value(iter, ProjectStore.Column.NAME, out name);
				model.get_value(iter, ProjectStore.Column.TYPE, out type);
				_editor.send_script(UnitPreviewApi.set_preview_resource((string)type, (string)name));
				_editor.send(DeviceApi.frame());
			}
		}
	}

	public void set_type_filter(owned UserFilter filter)
	{
		_user_filter = filter;
		_tree_filter.refilter();
	}
}

}
