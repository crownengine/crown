/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum DeleteDialogColumn
{
	CHECKED,
	SENSITIVE,
	TYPE,
	NAME,
	PATH,
	MISSING,

	COUNT
}

public enum DeleteDialogRowFlags
{
	NONE      = 0,
	CHECKED   = 1 << 0,
	SENSITIVE = 1 << 1,
}

public enum DeleteDialogGroupFlags
{
	CHECKBOXES     = 1 << 0,
	EXPAND         = 1 << 1,
	MISSING_COLUMN = 1 << 2,
}

public class DeleteDialog : Gtk.Window
{
	public ProjectBrowser _project_browser;
	public ThumbnailCache _thumbnail_cache;
	public Gtk.TreeStore _delete_store;
	public Gtk.TreeStore _missing_store;
	public Gtk.TreeModelFilter? _missing_filter;
	public Gee.HashSet<string> _checked_paths;
	public Gtk.Widget? _missing_group;
	public Gtk.Label? _missing_header;
	public Gtk.EventControllerKey _controller_key;

	public signal void response(int response_id);

	public DeleteDialog(Gtk.Window? parent
		, ProjectBrowser project_browser
		, ThumbnailCache thumbnail_cache
		, Hashtable preview
		)
	{
		_project_browser = project_browser;
		_thumbnail_cache = thumbnail_cache;
		_delete_store = create_store(false);
		_missing_store = create_store();
		_missing_filter = null;
		_checked_paths = new Gee.HashSet<string>();
		_missing_group = null;
		_missing_header = null;

		this.title = _("Delete Resources");
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);
		this.set_default_size(920, 520);
		this.set_transient_for(parent);
		this.set_modal(true);
		this.delete_event.connect(() => {
				response(Gtk.ResponseType.DELETE_EVENT);
				return Gdk.EVENT_STOP;
			});

		_controller_key = new Gtk.EventControllerKey(this);
		_controller_key.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_key.key_pressed.connect((keyval) => {
				if (keyval == Gdk.Key.Escape) {
					this.close();
					return Gdk.EVENT_STOP;
				}

				return Gdk.EVENT_PROPAGATE;
			});

		Gtk.HeaderBar header_bar = new Gtk.HeaderBar();
		header_bar.title = _("Delete Resources");
		header_bar.show_close_button = true;
		this.set_titlebar(header_bar);

		Gtk.Button cancel_button = new Gtk.Button.with_label(_("Cancel"));
		cancel_button.clicked.connect(() => {
				this.response(Gtk.ResponseType.CANCEL);
			});
		header_bar.pack_start(cancel_button);

		Gtk.Button action_button = new Gtk.Button.with_label(_("Delete"));
		action_button.get_style_context().add_class("destructive-action");
		action_button.clicked.connect(() => {
				this.response(Gtk.ResponseType.ACCEPT);
			});
		header_bar.pack_end(action_button);

		Gtk.Box content = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		content.border_width = 12;
		this.add(content);

		Gtk.Box list = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		list.border_width = 6;
		list.hexpand = true;
		list.vexpand = true;

		Gee.ArrayList<Value?> delete_paths = (Gee.ArrayList<Value?>)preview["delete"];
		Gee.ArrayList<Value?> associated = (Gee.ArrayList<Value?>)preview["associated"];
		Gee.ArrayList<Value?> unused = (Gee.ArrayList<Value?>)preview["unused"];
		Gee.ArrayList<Value?> keep = (Gee.ArrayList<Value?>)preview["keep"];
		uint delete_count = append_path_group(_delete_store
			, _("Delete")
			, delete_paths
			, DeleteDialogRowFlags.CHECKED
			);
		delete_count += append_path_group(_delete_store
			, _("Associated files")
			, associated
			, DeleteDialogRowFlags.CHECKED | DeleteDialogRowFlags.SENSITIVE
			);
		delete_count += append_path_group(_delete_store
			, _("Becomes unused")
			, unused
			, DeleteDialogRowFlags.SENSITIVE
			);
		delete_count += append_path_group(_delete_store
			, _("Shared")
			, keep
			, DeleteDialogRowFlags.SENSITIVE
			);

		if (delete_count != 0) {
			append_tree_group(list
				, _("Delete")
				, _("Selected resources and associated files to delete.\nResources that become unused and shared dependencies are unchecked by default.")
				, _delete_store
				, delete_count
				, DeleteDialogGroupFlags.CHECKBOXES | DeleteDialogGroupFlags.EXPAND
				);
		}

		Gee.ArrayList<Value?> missing_references = (Gee.ArrayList<Value?>)preview["missing_references"];
		if (missing_references.size != 0) {
			foreach (Value? item_value in missing_references) {
				Hashtable item = (Hashtable)item_value;
				append_path(_missing_store
					, null
					, (string)item["path"]
					, (string)item["missing"]
					, DeleteDialogRowFlags.NONE
					);
			}

			Gtk.TreeModelFilter missing_filter = new Gtk.TreeModelFilter(_missing_store, null);
			missing_filter.set_visible_func(missing_filter_visible_func);
			_missing_filter = missing_filter;
			Gtk.TreeModelSort missing_sort = new Gtk.TreeModelSort.with_model(missing_filter);
			missing_sort.set_sort_column_id(DeleteDialogColumn.PATH, Gtk.SortType.ASCENDING);
			_missing_header = new Gtk.Label(null);
			_missing_group = append_tree_group(list
				, _("Will Reference Missing Resources")
				, _("Resources outside the delete set that will still point to deleted paths.")
				, missing_sort
				, 0
				, DeleteDialogGroupFlags.MISSING_COLUMN
				, _missing_header
				);
			_missing_group.no_show_all = true;
		}

		update_missing_references();
		content.pack_start(list, true, true, 0);
		action_button.sensitive = delete_count != 0;
	}

	public Gtk.TreeStore create_store(bool sort = true)
	{
		Gtk.TreeStore store = new Gtk.TreeStore(DeleteDialogColumn.COUNT
			, typeof(bool)
			, typeof(bool)
			, typeof(string)
			, typeof(string)
			, typeof(string)
			, typeof(string)
			);
		if (sort)
			store.set_sort_column_id(DeleteDialogColumn.PATH, Gtk.SortType.ASCENDING);
		return store;
	}

	public Gtk.TreeIter append_parent(Gtk.TreeStore store, string label, DeleteDialogRowFlags flags)
	{
		Gtk.TreeIter iter;
		store.insert_with_values(out iter
			, null
			, -1
			, DeleteDialogColumn.CHECKED
			, (flags& DeleteDialogRowFlags.CHECKED) != 0
			, DeleteDialogColumn.SENSITIVE
			, (flags& DeleteDialogRowFlags.SENSITIVE) != 0
			, DeleteDialogColumn.TYPE
			, ""
			, DeleteDialogColumn.NAME
			, ""
			, DeleteDialogColumn.PATH
			, label
			, DeleteDialogColumn.MISSING
			, ""
			, -1
			);

		return iter;
	}

	public void append_path(Gtk.TreeStore store
		, Gtk.TreeIter? parent
		, string path
		, string missing
		, DeleteDialogRowFlags flags
		)
	{
		string? resource_type = ResourceId.type(path);
		string? resource_name = ResourceId.name(path);
		string type = "";
		string name = path;

		if (resource_type != null && resource_name != null) {
			type = resource_type;
			name = resource_name;
		}

		Gtk.TreeIter iter;
		store.insert_with_values(out iter
			, parent
			, -1
			, DeleteDialogColumn.CHECKED
			, (flags& DeleteDialogRowFlags.CHECKED) != 0
			, DeleteDialogColumn.SENSITIVE
			, (flags& DeleteDialogRowFlags.SENSITIVE) != 0
			, DeleteDialogColumn.TYPE
			, type
			, DeleteDialogColumn.NAME
			, name
			, DeleteDialogColumn.PATH
			, path
			, DeleteDialogColumn.MISSING
			, missing
			, -1
			);
	}

	public uint append_path_group(Gtk.TreeStore store
		, string label
		, Gee.ArrayList<Value?> paths
		, DeleteDialogRowFlags flags
		)
	{
		if (paths.size == 0)
			return 0;

		Gtk.TreeIter parent = append_parent(store, label, flags);
		foreach (Value? item_value in paths)
			append_path(store, parent, (string)item_value, "", flags);
		return (uint)paths.size;
	}

	public void set_checked_recursive(Gtk.TreeStore store, Gtk.TreeIter iter, bool checked)
	{
		Value val;
		store.get_value(iter, DeleteDialogColumn.SENSITIVE, out val);
		if (!(bool)val)
			return;

		store.set(iter, DeleteDialogColumn.CHECKED, checked, -1);

		Gtk.TreeIter child;
		if (store.iter_children(out child, iter)) {
			do {
				set_checked_recursive(store, child, checked);
			} while (store.iter_next(ref child));
		}
	}

	public void update_parent_checked(Gtk.TreeStore store, Gtk.TreeIter iter)
	{
		Gtk.TreeIter parent;
		if (!store.iter_parent(out parent, iter))
			return;

		bool all_checked = true;
		Gtk.TreeIter child;
		if (store.iter_children(out child, parent)) {
			do {
				Value val;
				store.get_value(child, DeleteDialogColumn.SENSITIVE, out val);
				if (!(bool)val)
					continue;

				store.get_value(child, DeleteDialogColumn.CHECKED, out val);
				all_checked = all_checked && (bool)val;
			} while (store.iter_next(ref child));
		}

		store.set(parent, DeleteDialogColumn.CHECKED, all_checked, -1);
	}

	public void reveal_row(Gtk.TreeModel model, Gtk.TreePath path)
	{
		Gtk.TreeIter iter;
		if (!model.get_iter(out iter, path) || model.iter_has_child(iter))
			return;

		Value val;
		model.get_value(iter, DeleteDialogColumn.TYPE, out val);
		string type = (string)val;
		model.get_value(iter, DeleteDialogColumn.NAME, out val);
		string name = (string)val;
		ProjectStore.RowKind kind = type == ""
			? ProjectStore.RowKind.FOLDER
			: ProjectStore.RowKind.RESOURCE
			;
		_project_browser.reveal(kind, type, name);
	}

	public Gtk.TreeView create_tree_view(Gtk.TreeModel model, DeleteDialogGroupFlags flags)
	{
		Gtk.TreeView tree_view = new Gtk.TreeView.with_model(model);
		bool show_missing = (flags& DeleteDialogGroupFlags.MISSING_COLUMN) != 0;
		tree_view.headers_visible = true;
		tree_view.enable_search = true;
		tree_view.search_column = DeleteDialogColumn.PATH;
		tree_view.row_activated.connect((path, column) => {
				reveal_row(model, path);
			});

		if ((flags& DeleteDialogGroupFlags.CHECKBOXES) != 0) {
			Gtk.TreeStore store = (Gtk.TreeStore)model;
			Gtk.CellRendererToggle cell_toggle = new Gtk.CellRendererToggle();
			cell_toggle.toggled.connect((path_string) => {
					Gtk.TreeIter iter;
					if (!store.get_iter_from_string(out iter, path_string))
						return;

					Value val;
					store.get_value(iter, DeleteDialogColumn.SENSITIVE, out val);
					if (!(bool)val)
						return;

					store.get_value(iter, DeleteDialogColumn.CHECKED, out val);
					bool checked = !(bool)val;
					set_checked_recursive(store, iter, checked);
					update_parent_checked(store, iter);
					if (store == _delete_store)
						update_missing_references();
				});

			Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
			column.sizing = Gtk.TreeViewColumnSizing.FIXED;
			column.fixed_width = 28;
			column.pack_start(cell_toggle, false);
			column.add_attribute(cell_toggle, "active", DeleteDialogColumn.CHECKED);
			column.add_attribute(cell_toggle, "sensitive", DeleteDialogColumn.SENSITIVE);
			column.add_attribute(cell_toggle, "activatable", DeleteDialogColumn.SENSITIVE);
			tree_view.append_column(column);
		}

		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		cell_pixbuf.set_property("width", 16);
		cell_pixbuf.set_property("height", 16);
		cell_pixbuf.set_property("xpad", 2u);
		cell_pixbuf.set_property("ypad", 1u);
		Gtk.TreeViewColumn icon_column = new Gtk.TreeViewColumn();
		icon_column.sizing = Gtk.TreeViewColumnSizing.FIXED;
		icon_column.fixed_width = 28;
		icon_column.pack_start(cell_pixbuf, false);
		icon_column.set_cell_data_func(cell_pixbuf, icon_func);
		tree_view.append_column(icon_column);

		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		Gtk.TreeViewColumn path_column = new Gtk.TreeViewColumn();
		path_column.title = _("Resource");
		path_column.expand = true;
		path_column.sort_column_id = DeleteDialogColumn.PATH;
		path_column.pack_start(cell_text, true);
		path_column.set_cell_data_func(cell_text, label_func);
		tree_view.append_column(path_column);
		tree_view.expander_column = path_column;

		if (show_missing) {
			Gtk.CellRendererText cell_missing = new Gtk.CellRendererText();
			Gtk.TreeViewColumn missing_column = new Gtk.TreeViewColumn();
			missing_column.title = _("Missing Resource");
			missing_column.expand = true;
			missing_column.sort_column_id = DeleteDialogColumn.MISSING;
			missing_column.pack_start(cell_missing, true);
			missing_column.add_attribute(cell_missing, "text", DeleteDialogColumn.MISSING);
			tree_view.append_column(missing_column);
		}

		tree_view.expand_all();
		return tree_view;
	}

	public void icon_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Gdk.Pixbuf? no_pixbuf = null;
		string? no_icon = null;
		cell.set_property("pixbuf", no_pixbuf);
		cell.set_property("icon-name", no_icon);

		Value val;
		if (model.iter_has_child(iter)) {
			cell.set_property("icon-name", IconTheme.BROWSER_FOLDER);
			return;
		}

		model.get_value(iter, DeleteDialogColumn.TYPE, out val);
		string type = (string)val;
		model.get_value(iter, DeleteDialogColumn.NAME, out val);
		string name = (string)val;
		ProjectStore.RowKind kind = type == ""
			? ProjectStore.RowKind.FOLDER
			: ProjectStore.RowKind.RESOURCE
			;
		set_thumbnail(cell, kind, type, name, 16, _thumbnail_cache);
	}

	public void label_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, DeleteDialogColumn.PATH, out val);
		string label = (string)val;

		string markup = GLib.Markup.escape_text(label);
		if (model.iter_has_child(iter))
			markup = "<b>%s</b>".printf(markup);

		cell.set_property("markup", markup);
	}

	public Gtk.Widget append_tree_group(Gtk.Box box
		, string title
		, string tooltip
		, Gtk.TreeModel model
		, uint count
		, DeleteDialogGroupFlags flags
		, Gtk.Label? custom_header = null
		)
	{
		bool expand = (flags& DeleteDialogGroupFlags.EXPAND) != 0;
		Gtk.Label header;
		if (custom_header != null)
			header = custom_header;
		else
			header = new Gtk.Label(null);
		header.set_markup("<b>%s</b>".printf(GLib.Markup.escape_text("%s (%u)".printf(title, count))));
		header.set_xalign(0.0f);
		header.set_tooltip_text(tooltip);

		Gtk.TreeView tree_view = create_tree_view(model, flags);
		tree_view.set_tooltip_text(tooltip);

		Gtk.ScrolledWindow scrolled = new Gtk.ScrolledWindow(null, null);
		scrolled.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
		scrolled.min_content_width = 360;
		scrolled.min_content_height = 120;
		scrolled.hexpand = true;
		scrolled.vexpand = expand;
		scrolled.add(tree_view);

		Gtk.Box group = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		group.set_margin_bottom(12);
		header.set_margin_bottom(8);
		group.pack_start(header, false, false, 0);
		group.pack_start(scrolled, expand, true, 0);
		box.pack_start(group, expand, true, 0);
		return group;
	}

	public bool missing_filter_visible_func(Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, DeleteDialogColumn.PATH, out val);
		string path = (string)val;
		model.get_value(iter, DeleteDialogColumn.MISSING, out val);
		string missing = (string)val;

		return _checked_paths.contains(missing) && !_checked_paths.contains(path);
	}

	public void update_missing_references()
	{
		Gtk.Widget? group = _missing_group;
		Gtk.Label? header = _missing_header;
		Gtk.TreeModelFilter? filter = _missing_filter;
		if (group == null || header == null || filter == null)
			return;

		_checked_paths.clear();
		_delete_store.foreach((model, path, iter) => {
				Value val;
				model.get_value(iter, DeleteDialogColumn.CHECKED, out val);
				if ((bool)val && !model.iter_has_child(iter)) {
					model.get_value(iter, DeleteDialogColumn.PATH, out val);
					_checked_paths.add((string)val);
				}

				return false;
			});

		filter.refilter();

		var missing_paths = new Gee.HashSet<string>();
		filter.foreach((model, path, iter) => {
				Value val;
				model.get_value(iter, DeleteDialogColumn.PATH, out val);
				missing_paths.add((string)val);
				return false;
			});
		uint count = (uint)missing_paths.size;

		header.set_markup("<b>%s</b>".printf(GLib.Markup.escape_text("%s (%u)".printf(_("Will Reference Missing Resources"), count))));
		if (count != 0) {
			group.no_show_all = false;
			group.show_all();
		} else {
			group.hide();
			group.no_show_all = true;
		}
	}

	public string[] selected()
	{
		var paths = new Gee.ArrayList<string>();
		_delete_store.foreach((model, path, iter) => {
				Value val;
				model.get_value(iter, DeleteDialogColumn.CHECKED, out val);
				if ((bool)val && !model.iter_has_child(iter)) {
					model.get_value(iter, DeleteDialogColumn.PATH, out val);
					paths.add((string)val);
				}

				return false;
			});

		return paths.to_array();
	}
}

} /* namespace Crown */
