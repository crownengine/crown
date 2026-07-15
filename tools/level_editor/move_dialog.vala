/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum MoveDialogColumn
{
	CHECKED,
	SENSITIVE,
	TYPE,
	NAME,
	PATH,
	TO,

	COUNT
}

public enum MoveDialogRowFlags
{
	NONE      = 0,
	CHECKED   = 1 << 0,
	SENSITIVE = 1 << 1,
}

public enum MoveDialogGroupFlags
{
	NONE       = 0,
	CHECKBOXES = 1 << 0,
	EXPAND     = 1 << 1,
	COLLAPSED  = 1 << 2,
}

public class MoveDialog : Gtk.Window
{
	public ProjectBrowser _project_browser;
	public ThumbnailCache _thumbnail_cache;
	public Gtk.TreeStore _move_store;
	public Gtk.EventControllerKey _controller_key;

	public signal void response(int response_id);

	public MoveDialog(Gtk.Window? parent
		, ProjectBrowser project_browser
		, ThumbnailCache thumbnail_cache
		, string title
		, string move_tooltip
		, GLib.HashTable<string, Value?> preview
		)
	{
		_project_browser = project_browser;
		_thumbnail_cache = thumbnail_cache;
		_move_store = create_store(false);

		this.title = title;
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
		header_bar.title = title;
		header_bar.show_close_button = true;
		this.set_titlebar(header_bar);

		Gtk.Button cancel_button = new Gtk.Button.with_label(_("Cancel"));
		cancel_button.clicked.connect(() => {
				this.response(Gtk.ResponseType.CANCEL);
			});
		header_bar.pack_start(cancel_button);

		Gtk.Button action_button = new Gtk.Button.with_label(_("Move"));
		action_button.get_style_context().add_class("suggested-action");
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

		GLib.GenericArray<Value?> move = (GLib.GenericArray<Value?>)preview["move"];
		GLib.GenericArray<Value?> also_move = (GLib.GenericArray<Value?>)preview["also_move"];
		if (move.length != 0) {
			Gtk.TreeIter parent_iter = append_parent(_move_store, _("Move"), MoveDialogRowFlags.CHECKED);
			for (int i = 0; i < move.length; ++i) {
				GLib.HashTable<string, Value?> item = (GLib.HashTable<string, Value?>)move[i];
				append_path(_move_store, parent_iter, (string)item["from"], (string)item["to"], MoveDialogRowFlags.CHECKED);
			}
		}
		if (also_move.length != 0) {
			Gtk.TreeIter parent_iter = append_parent(_move_store
				, _("Also safe to move")
				, MoveDialogRowFlags.CHECKED | MoveDialogRowFlags.SENSITIVE
				);
			for (int i = 0; i < also_move.length; ++i) {
				GLib.HashTable<string, Value?> item = (GLib.HashTable<string, Value?>)also_move[i];
				append_path(_move_store
					, parent_iter
					, (string)item["from"]
					, (string)item["to"]
					, MoveDialogRowFlags.CHECKED | MoveDialogRowFlags.SENSITIVE
					);
			}
		}

		bool has_move_paths = move.length != 0 || also_move.length != 0;
		if (has_move_paths) {
			append_tree_group(list
				, _("Move")
				, move_tooltip
				, _move_store
				, (uint)(move.length + also_move.length)
				, MoveDialogGroupFlags.CHECKBOXES | MoveDialogGroupFlags.EXPAND
				);
		}

		Gtk.TreeStore rewrite_store = create_store();
		var rewrite_paths = new GLib.GenericSet<string>(GLib.str_hash, GLib.str_equal);
		GLib.GenericArray<Value?> rewrite = (GLib.GenericArray<Value?>)preview["rewrite"];
		for (int i = 0; i < rewrite.length; ++i) {
			GLib.HashTable<string, Value?> item = (GLib.HashTable<string, Value?>)rewrite[i];
			string path = (string)item["path"];
			if (rewrite_paths.add(path))
				append_path(rewrite_store, null, path, "", MoveDialogRowFlags.SENSITIVE);
		}
		if (rewrite_paths.length != 0) {
			append_tree_group(list
				, _("Update References")
				, _("Source files whose resource references or paths may be rewritten.")
				, rewrite_store
				, rewrite_paths.length
				, MoveDialogGroupFlags.NONE
				);
		}

		Gtk.TreeStore shared_store = create_store();
		GLib.GenericArray<Value?> keep = (GLib.GenericArray<Value?>)preview["keep"];
		for (int i = 0; i < keep.length; ++i)
			append_path(shared_store, null, (string)keep[i], "", MoveDialogRowFlags.NONE);
		if (keep.length != 0) {
			append_tree_group(list
				, _("Shared")
				, _("Dependencies also used by resources outside the move set.")
				, shared_store
				, (uint)keep.length
				, MoveDialogGroupFlags.CHECKBOXES | MoveDialogGroupFlags.COLLAPSED
				);
		}

		content.pack_start(list, true, true, 0);
		action_button.sensitive = has_move_paths;
	}

	public Gtk.TreeStore create_store(bool sort = true)
	{
		Gtk.TreeStore store = new Gtk.TreeStore(MoveDialogColumn.COUNT
			, typeof(bool)
			, typeof(bool)
			, typeof(string)
			, typeof(string)
			, typeof(string)
			, typeof(string)
			);

		if (sort)
			store.set_sort_column_id(MoveDialogColumn.PATH, Gtk.SortType.ASCENDING);

		return store;
	}

	public Gtk.TreeIter append_parent(Gtk.TreeStore store, string label, MoveDialogRowFlags flags)
	{
		Gtk.TreeIter iter;
		store.insert_with_values(out iter
			, null
			, -1
			, MoveDialogColumn.CHECKED
			, (flags& MoveDialogRowFlags.CHECKED) != 0
			, MoveDialogColumn.SENSITIVE
			, (flags& MoveDialogRowFlags.SENSITIVE) != 0
			, MoveDialogColumn.TYPE
			, ""
			, MoveDialogColumn.NAME
			, ""
			, MoveDialogColumn.PATH
			, label
			, MoveDialogColumn.TO
			, ""
			, -1
			);

		return iter;
	}

	public void append_path(Gtk.TreeStore store
		, Gtk.TreeIter? parent
		, string path
		, string to
		, MoveDialogRowFlags flags
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
			, MoveDialogColumn.CHECKED
			, (flags& MoveDialogRowFlags.CHECKED) != 0
			, MoveDialogColumn.SENSITIVE
			, (flags& MoveDialogRowFlags.SENSITIVE) != 0
			, MoveDialogColumn.TYPE
			, type
			, MoveDialogColumn.NAME
			, name
			, MoveDialogColumn.PATH
			, path
			, MoveDialogColumn.TO
			, to
			, -1
			);
	}

	public void set_checked_recursive(Gtk.TreeStore store, Gtk.TreeIter iter, bool checked)
	{
		Value val;
		store.get_value(iter, MoveDialogColumn.SENSITIVE, out val);
		if (!(bool)val)
			return;

		store.set(iter, MoveDialogColumn.CHECKED, checked, -1);

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
				store.get_value(child, MoveDialogColumn.SENSITIVE, out val);
				if (!(bool)val)
					continue;

				store.get_value(child, MoveDialogColumn.CHECKED, out val);
				all_checked = all_checked && (bool)val;
			} while (store.iter_next(ref child));
		}

		store.set(parent, MoveDialogColumn.CHECKED, all_checked, -1);
	}

	public void reveal_row(Gtk.TreeStore store, Gtk.TreePath path)
	{
		Gtk.TreeIter iter;
		if (!store.get_iter(out iter, path))
			return;

		if (store.iter_has_child(iter))
			return;

		Value val;
		store.get_value(iter, MoveDialogColumn.TYPE, out val);
		string type = (string)val;
		store.get_value(iter, MoveDialogColumn.NAME, out val);
		string name = (string)val;
		ProjectStore.RowKind kind = type == ""
			? ProjectStore.RowKind.FOLDER
			: ProjectStore.RowKind.RESOURCE
			;

		_project_browser.reveal(kind, type, name);
	}

	public Gtk.TreeView create_tree_view(Gtk.TreeStore store, MoveDialogGroupFlags flags)
	{
		Gtk.TreeView tree_view = new Gtk.TreeView.with_model(store);
		bool show_targets = store == _move_store;
		tree_view.headers_visible = true;
		tree_view.enable_search = true;
		tree_view.search_column = MoveDialogColumn.PATH;
		tree_view.row_activated.connect((path, column) => {
				reveal_row(store, path);
			});

		if ((flags& MoveDialogGroupFlags.CHECKBOXES) != 0) {
			Gtk.CellRendererToggle cell_toggle = new Gtk.CellRendererToggle();
			cell_toggle.toggled.connect((path_string) => {
					Gtk.TreeIter iter;
					if (!store.get_iter_from_string(out iter, path_string))
						return;

					Value val;
					store.get_value(iter, MoveDialogColumn.SENSITIVE, out val);
					if (!(bool)val)
						return;

					store.get_value(iter, MoveDialogColumn.CHECKED, out val);
					bool checked = !(bool)val;
					set_checked_recursive(store, iter, checked);
					update_parent_checked(store, iter);
				});

			Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
			column.sizing = Gtk.TreeViewColumnSizing.FIXED;
			column.fixed_width = 28;
			column.pack_start(cell_toggle, false);
			column.add_attribute(cell_toggle, "active", MoveDialogColumn.CHECKED);
			column.add_attribute(cell_toggle, "sensitive", MoveDialogColumn.SENSITIVE);
			column.add_attribute(cell_toggle, "activatable", MoveDialogColumn.SENSITIVE);
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
		path_column.title = show_targets ? _("Old Name") : _("Resource");
		path_column.expand = true;
		path_column.sort_column_id = MoveDialogColumn.PATH;
		path_column.pack_start(cell_text, true);
		path_column.set_cell_data_func(cell_text, label_func);
		tree_view.append_column(path_column);
		tree_view.expander_column = path_column;

		if (show_targets) {
			Gtk.CellRendererText cell_target = new Gtk.CellRendererText();
			Gtk.TreeViewColumn target_column = new Gtk.TreeViewColumn();
			target_column.title = _("New Name");
			target_column.expand = true;
			target_column.sort_column_id = MoveDialogColumn.TO;
			target_column.pack_start(cell_target, true);
			target_column.add_attribute(cell_target, "text", MoveDialogColumn.TO);
			tree_view.append_column(target_column);
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

		model.get_value(iter, MoveDialogColumn.TYPE, out val);
		string type = (string)val;
		model.get_value(iter, MoveDialogColumn.NAME, out val);
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
		model.get_value(iter, MoveDialogColumn.PATH, out val);
		string label = (string)val;

		string markup = GLib.Markup.escape_text(label);
		if (model.iter_has_child(iter))
			markup = "<b>%s</b>".printf(markup);

		cell.set_property("markup", markup);
	}

	public void append_tree_group(Gtk.Box box
		, string title
		, string tooltip
		, Gtk.TreeStore store
		, uint count
		, MoveDialogGroupFlags flags
		)
	{
		bool expand = (flags& MoveDialogGroupFlags.EXPAND) != 0;
		bool collapsed = (flags& MoveDialogGroupFlags.COLLAPSED) != 0;
		Gtk.Label header = new Gtk.Label(null);
		header.set_markup("<b>%s</b>".printf(GLib.Markup.escape_text("%s (%u)".printf(title, count))));
		header.set_xalign(0.0f);
		header.set_tooltip_text(tooltip);

		Gtk.TreeView tree_view = create_tree_view(store, flags);
		tree_view.set_tooltip_text(tooltip);

		Gtk.ScrolledWindow scrolled = new Gtk.ScrolledWindow(null, null);
		scrolled.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
		scrolled.min_content_width = 360;
		scrolled.min_content_height = 120;
		scrolled.hexpand = true;
		scrolled.vexpand = expand && !collapsed;
		scrolled.add(tree_view);

		if (collapsed) {
			Expander expander = new Expander();
			expander.set_margin_bottom(12);
			expander.set_tooltip_text(tooltip);
			expander.custom_header = header;

			scrolled.set_margin_top(8);
			expander.add(scrolled);
			box.pack_start(expander, false, true, 0);
			return;
		}

		Gtk.Box group = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		group.set_margin_bottom(12);
		header.set_margin_bottom(8);
		group.pack_start(header, false, false, 0);
		group.pack_start(scrolled, expand, true, 0);
		box.pack_start(group, expand, true, 0);
	}

	public void selected(out string[] checked_from, out string[] checked_to)
	{
		var selected_from = new GLib.GenericArray<string>();
		var selected_to = new GLib.GenericArray<string>();

		_move_store.foreach((model, path, iter) => {
				Value val;
				model.get_value(iter, MoveDialogColumn.CHECKED, out val);
				if ((bool)val && !model.iter_has_child(iter)) {
					model.get_value(iter, MoveDialogColumn.PATH, out val);
					selected_from.add((string)val);
					model.get_value(iter, MoveDialogColumn.TO, out val);
					selected_to.add((string)val);
				}

				return false;
			});

		checked_from = selected_from.steal();
		checked_to = selected_to.steal();
	}
}

} /* namespace Crown */
