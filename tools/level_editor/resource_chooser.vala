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
	public ConsoleClient _console_client;
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

	public Slide _editor_slide;
	public EditorView _editor_view;

	// Signals
	public signal void resource_selected(string type, string name);

	public ResourceChooser(Project? project, ProjectStore project_store, bool preview)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_project = project;

		_console_client = new ConsoleClient();
		_console_client.connected.connect(on_client_connected);
		_console_client.disconnected.connect(on_client_disconnected);
		_console_client.message_received.connect(on_client_message_received);

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

		_editor_slide = new Slide();

		this.pack_start(_filter_entry, false, true, 0);
		this.pack_start(_editor_slide, true, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);

		Gtk.Label label = new Gtk.Label("No Preview");
		label.set_size_request(300, 300);
		_editor_slide.show_widget(label);

		this.destroy.connect(on_destroy);
		this.unmap.connect(on_unmap);

		restart_editor();
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
		stop_editor();
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

		// Try to connect to unit_preview.
		int tries = yield _console_client.connect_async("127.0.0.1"
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

	public void stop_editor()
	{
		if (!_preview)
			return;

		if (_console_client != null && _console_client.is_connected())
		{
			_console_client.send_script("Device.quit()");
			_console_client.close();
		}

		if (_editor_process != null)
		{
			try
			{
				_editor_process.wait();
			}
			catch (Error e)
			{
				loge(e.message);
			}
		}
	}

	public void restart_editor()
	{
		if (!_preview)
			return;

		stop_editor();

		if (_editor_view != null)
		{
			_editor_view = null;
		}

		_editor_view = new EditorView(_console_client, false);
		_editor_view.set_size_request(300, 300);
		_editor_view.realized.connect(on_editor_view_realized);

		_editor_slide.show_widget(_editor_view);
	}

	private void on_client_connected(string address, int port)
	{
		_console_client.receive_async();
	}

	private void on_client_disconnected()
	{
		// Do nothing.
	}

	private void on_client_message_received(ConsoleClient client, uint8[] json)
	{
		// Ignore the message content.
		_console_client.receive_async();
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
				_console_client.send_script(UnitPreviewApi.set_preview_resource((string)type, (string)name));
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
