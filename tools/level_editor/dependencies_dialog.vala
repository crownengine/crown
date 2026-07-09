/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum DependenciesDialogColumn
{
	KIND,
	TYPE,
	NAME,
	PATH,
	LABEL,

	COUNT
}

public enum DependenciesDialogRowKind
{
	GROUP,
	FOLDER,
	RESOURCE,

	COUNT
}

public class DependenciesDialog : Gtk.Window
{
	public ProjectBrowser _project_browser;
	public ThumbnailCache _thumbnail_cache;
	public Gtk.Box _content;

	public uint append_items(Gtk.TreeStore store, string label, Gee.ArrayList<Value?> items)
	{
		if (items.size == 0)
			return 0;

		Gtk.TreeIter parent;
		store.insert_with_values(out parent
			, null
			, -1
			, DependenciesDialogColumn.KIND
			, DependenciesDialogRowKind.GROUP
			, DependenciesDialogColumn.TYPE
			, ""
			, DependenciesDialogColumn.NAME
			, label
			, DependenciesDialogColumn.PATH
			, ""
			, DependenciesDialogColumn.LABEL
			, label
			, -1
			);

		uint count = 0;
		foreach (Value? item_value in items) {
			string path = (string)item_value;

			string? resource_type = ResourceId.type(path);
			string? resource_name = ResourceId.name(path);
			DependenciesDialogRowKind kind = DependenciesDialogRowKind.FOLDER;
			string type = "";
			string name = path;

			if (resource_type != null && resource_name != null) {
				kind = DependenciesDialogRowKind.RESOURCE;
				type = resource_type;
				name = resource_name;
			}

			Gtk.TreeIter iter;
			store.insert_with_values(out iter
				, parent
				, -1
				, DependenciesDialogColumn.KIND
				, kind
				, DependenciesDialogColumn.TYPE
				, type
				, DependenciesDialogColumn.NAME
				, name
				, DependenciesDialogColumn.PATH
				, path
				, DependenciesDialogColumn.LABEL
				, path
				, -1
				);
			++count;
		}

		return count;
	}

	public void append_group(Gtk.Box box
		, string title
		, string tooltip
		, Gee.ArrayList<Value?> first_items
		, string first_kind
		, Gee.ArrayList<Value?> second_items
		, string second_kind
		)
	{
		if (first_items.size == 0 && second_items.size == 0)
			return;

		Gtk.TreeStore store = new Gtk.TreeStore(DependenciesDialogColumn.COUNT
			, typeof(DependenciesDialogRowKind) // DependenciesDialogColumn.KIND
			, typeof(string) // DependenciesDialogColumn.TYPE
			, typeof(string) // DependenciesDialogColumn.NAME
			, typeof(string) // DependenciesDialogColumn.PATH
			, typeof(string) // DependenciesDialogColumn.LABEL
			);
		store.set_sort_column_id(DependenciesDialogColumn.PATH, Gtk.SortType.ASCENDING);

		uint count = append_items(store, first_kind, first_items);
		count += append_items(store, second_kind, second_items);

		Gtk.Box group = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		group.set_margin_bottom(12);

		Gtk.Label header = new Gtk.Label(null);
		header.set_markup("<b>%s</b>".printf(GLib.Markup.escape_text("%s (%u)".printf(title, count))));
		header.set_xalign(0.0f);
		header.set_margin_bottom(8);
		header.set_tooltip_text(tooltip);
		group.pack_start(header, false, false, 0);

		Gtk.TreeView tree_view = new Gtk.TreeView.with_model(store);
		tree_view.headers_visible = true;
		tree_view.enable_search = true;
		tree_view.search_column = DependenciesDialogColumn.PATH;
		tree_view.set_tooltip_text(tooltip);
		tree_view.row_activated.connect((path, column) => {
				Gtk.TreeIter iter;
				if (!store.get_iter(out iter, path))
					return;

				Value val;
				store.get_value(iter, DependenciesDialogColumn.KIND, out val);
				DependenciesDialogRowKind dialog_kind = (DependenciesDialogRowKind)val;
				if (dialog_kind == DependenciesDialogRowKind.GROUP)
					return;

				ProjectStore.RowKind kind = dialog_kind == DependenciesDialogRowKind.FOLDER
					? ProjectStore.RowKind.FOLDER
					: ProjectStore.RowKind.RESOURCE
					;

				store.get_value(iter, DependenciesDialogColumn.TYPE, out val);
				string type = (string)val;
				store.get_value(iter, DependenciesDialogColumn.NAME, out val);
				string name = (string)val;

				_project_browser.reveal(kind, type, name);
			});

		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		cell_pixbuf.set_property("width", 16);
		cell_pixbuf.set_property("height", 16);
		cell_pixbuf.set_property("xpad", 2u);
		cell_pixbuf.set_property("ypad", 1u);
		Gtk.TreeViewColumn icon_column = new Gtk.TreeViewColumn();
		icon_column.sizing = Gtk.TreeViewColumnSizing.FIXED;
		icon_column.fixed_width = 28;
		icon_column.pack_start(cell_pixbuf, false);
		icon_column.set_cell_data_func(cell_pixbuf, (cell_layout, cell, model, iter) => {
				Gdk.Pixbuf? no_pixbuf = null;
				string? no_icon = null;
				cell.set_property("pixbuf", no_pixbuf);
				cell.set_property("icon-name", no_icon);

				Value val;
				model.get_value(iter, DependenciesDialogColumn.KIND, out val);
				DependenciesDialogRowKind dialog_kind = (DependenciesDialogRowKind)val;
				if (dialog_kind == DependenciesDialogRowKind.GROUP) {
					cell.set_property("icon-name", IconTheme.BROWSER_FOLDER);
					return;
				}

				ProjectStore.RowKind kind = dialog_kind == DependenciesDialogRowKind.FOLDER
					? ProjectStore.RowKind.FOLDER
					: ProjectStore.RowKind.RESOURCE
					;

				model.get_value(iter, DependenciesDialogColumn.TYPE, out val);
				string type = (string)val;
				model.get_value(iter, DependenciesDialogColumn.NAME, out val);
				string name = (string)val;

				set_thumbnail(cell, kind, type, name, 16, _thumbnail_cache);
			});
		tree_view.append_column(icon_column);

		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		Gtk.TreeViewColumn path_column = new Gtk.TreeViewColumn();
		path_column.title = _("Resource");
		path_column.expand = true;
		path_column.sort_column_id = DependenciesDialogColumn.PATH;
		path_column.pack_start(cell_text, true);
		path_column.set_cell_data_func(cell_text, (cell_layout, cell, model, iter) => {
				Value val;
				model.get_value(iter, DependenciesDialogColumn.LABEL, out val);
				string label = (string)val;

				string markup = GLib.Markup.escape_text(label);
				model.get_value(iter, DependenciesDialogColumn.KIND, out val);
				if ((DependenciesDialogRowKind)val == DependenciesDialogRowKind.GROUP)
					markup = "<b>%s</b>".printf(markup);

				cell.set_property("markup", markup);
			});
		tree_view.append_column(path_column);
		tree_view.expander_column = path_column;
		tree_view.expand_all();

		Gtk.ScrolledWindow scrolled = new Gtk.ScrolledWindow(null, null);
		scrolled.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
		scrolled.min_content_width = 360;
		scrolled.min_content_height = 120;
		scrolled.hexpand = true;
		scrolled.vexpand = true;
		scrolled.add(tree_view);
		group.pack_start(scrolled, true, true, 0);

		box.pack_start(group, true, true, 0);
	}

	public DependenciesDialog(Gtk.Window? parent, ProjectBrowser project_browser, ThumbnailCache thumbnail_cache)
	{
		_project_browser = project_browser;
		_thumbnail_cache = thumbnail_cache;

		this.title = _("Resource Dependencies");
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);
		this.set_default_size(920, 520);

		Gtk.HeaderBar header_bar = new Gtk.HeaderBar();
		header_bar.title = _("Resource Dependencies");
		header_bar.show_close_button = true;
		this.set_titlebar(header_bar);
		this.set_transient_for(parent);
		this.set_modal(true);

		Gtk.Button close_button = new Gtk.Button.with_label(_("Close"));
		close_button.clicked.connect(() => {
				this.hide();
			});
		header_bar.pack_start(close_button);

		_content = new Gtk.Box(Gtk.Orientation.VERTICAL, 12);
		_content.border_width = 12;
		this.add(_content);
	}

	public void set_content(string resource_path, Hashtable dependencies)
	{
		foreach (Gtk.Widget child in _content.get_children())
			child.destroy();

		Gee.ArrayList<Value?> dependency_items = (Gee.ArrayList<Value?>)dependencies["dependencies"];
		Gee.ArrayList<Value?> reference_items = (Gee.ArrayList<Value?>)dependencies["references"];
		Gee.ArrayList<Value?> dependent_items = (Gee.ArrayList<Value?>)dependencies["dependents"];
		Gee.ArrayList<Value?> referrer_items = (Gee.ArrayList<Value?>)dependencies["referrers"];

		Gtk.Label path_label = new Gtk.Label(resource_path);
		path_label.set_xalign(0.0f);
		path_label.set_line_wrap(true);
		path_label.set_line_wrap_mode(Pango.WrapMode.CHAR);
		path_label.set_selectable(true);
		_content.pack_start(path_label, false, false, 0);

		Gtk.Box columns = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 12);
		columns.border_width = 6;

		Gtk.Box left = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		left.hexpand = true;
		left.vexpand = true;

		Gtk.Box right = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		right.hexpand = true;
		right.vexpand = true;

		columns.pack_start(left, true, true, 0);
		columns.pack_start(right, true, true, 0);

		append_group(left
			, _("Uses")
			, _("Resources used by this resource.")
			, dependency_items
			, _("Depends On")
			, reference_items
			, _("References")
			);
		append_group(right
			, _("Used By")
			, _("Resources that use this resource.")
			, dependent_items
			, _("Used By")
			, referrer_items
			, _("Required By")
			);

		if (dependency_items.size == 0
			&& reference_items.size == 0
			&& dependent_items.size == 0
			&& referrer_items.size == 0
			) {
			Gtk.Label empty = new Gtk.Label(_("No direct dependencies or references."));
			empty.set_xalign(0.0f);
			left.pack_start(empty, false, false, 0);
		}

		if (left.get_children().length() != 0 || right.get_children().length() != 0) {
			if (left.get_children().length() == 0) {
				left.no_show_all = true;
				left.hide();
			}
			if (right.get_children().length() == 0) {
				right.no_show_all = true;
				right.hide();
			}
		}

		_content.pack_start(columns, true, true, 0);
		_content.show_all();
	}
}

} /* namespace Crown */
