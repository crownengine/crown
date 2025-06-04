/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum DependenciesDialogColumn
{
	TYPE,
	NAME,
	PATH,
	LABEL,

	COUNT
}

public class DependenciesDialog : Gtk.Window
{
	public ProjectBrowser _project_browser;
	public ThumbnailCache _thumbnail_cache;
	public Gtk.Box _content;

	public void reveal_dependency(Gtk.TreeView tree_view, Gtk.TreePath path)
	{
		Gtk.TreeIter iter;
		Gtk.TreeModel store = tree_view.get_model();
		if (!store.get_iter(out iter, path) || store.iter_has_child(iter))
			return;

		Value val;
		store.get_value(iter, DependenciesDialogColumn.TYPE, out val);
		string type = (string)val;
		store.get_value(iter, DependenciesDialogColumn.NAME, out val);
		string name = (string)val;
		ProjectStore.RowKind kind = type == ""
			? ProjectStore.RowKind.FOLDER
			: ProjectStore.RowKind.RESOURCE
			;

		_project_browser.reveal(kind, type, name);
	}

#if CROWN_GTK3
	public void on_row_activated(Gtk.TreePath path, Gtk.TreeViewColumn column)
	{
		reveal_dependency((Gtk.TreeView)column.get_tree_view(), path);
	}
#endif

#if !CROWN_GTK3
	public void on_row_activated(Gtk.TreeView tree_view, Gtk.TreePath path, Gtk.TreeViewColumn? column)
	{
		reveal_dependency(tree_view, path);
	}
#endif

	public uint append_items(Gtk.TreeStore store, string label, GLib.GenericArray<Value?> items)
	{
		if (items.length == 0)
			return 0;

		Gtk.TreeIter parent;
		store.insert_with_values(out parent
			, null
			, -1
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
		for (int i = 0; i < items.length; ++i) {
			string path = (string)items[i];

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
		, GLib.GenericArray<Value?> first_items
		, string first_kind
		, GLib.GenericArray<Value?> second_items
		, string second_kind
		)
	{
		if (first_items.length == 0 && second_items.length == 0)
			return;

		Gtk.TreeStore store = new Gtk.TreeStore(DependenciesDialogColumn.COUNT
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
#if CROWN_GTK3
		group.pack_start(header, false, false, 0);
#else
		group.append(header);
#endif

		Gtk.TreeView tree_view = new Gtk.TreeView.with_model(store);
		tree_view.headers_visible = true;
		tree_view.enable_search = true;
		tree_view.search_column = DependenciesDialogColumn.PATH;
		tree_view.set_tooltip_text(tooltip);
		tree_view.row_activated.connect(on_row_activated);

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
				if (model.iter_has_child(iter)) {
					cell.set_property("icon-name", IconTheme.BROWSER_FOLDER);
					return;
				}

				model.get_value(iter, DependenciesDialogColumn.TYPE, out val);
				string type = (string)val;
				model.get_value(iter, DependenciesDialogColumn.NAME, out val);
				string name = (string)val;
				ProjectStore.RowKind kind = type == ""
					? ProjectStore.RowKind.FOLDER
					: ProjectStore.RowKind.RESOURCE
					;

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
				if (model.iter_has_child(iter))
					markup = "<b>%s</b>".printf(markup);

				cell.set_property("markup", markup);
			});
		tree_view.append_column(path_column);
		tree_view.expander_column = path_column;
		tree_view.expand_all();

#if CROWN_GTK3
		Gtk.ScrolledWindow scrolled = new Gtk.ScrolledWindow(null, null);
#else
		Gtk.ScrolledWindow scrolled = new Gtk.ScrolledWindow();
#endif
		scrolled.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
		scrolled.min_content_width = 360;
		scrolled.min_content_height = 120;
		scrolled.hexpand = true;
		scrolled.vexpand = true;
#if CROWN_GTK3
		scrolled.add(tree_view);
		group.pack_start(scrolled, true, true, 0);

		box.pack_start(group, true, true, 0);
#else
		scrolled.set_child(tree_view);
		group.append(scrolled);

		box.append(group);
#endif
	}

	public DependenciesDialog(Gtk.Window? parent, ProjectBrowser project_browser, ThumbnailCache thumbnail_cache)
	{
		_project_browser = project_browser;
		_thumbnail_cache = thumbnail_cache;

		this.title = _("Resource Dependencies");
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);
		this.set_default_size(920, 520);

		Gtk.HeaderBar header_bar = new Gtk.HeaderBar();
#if CROWN_GTK3
		header_bar.title = _("Resource Dependencies");
		header_bar.show_close_button = true;
#else
		header_bar.set_title_widget(new Gtk.Label(_("Resource Dependencies")));
		header_bar.show_title_buttons = true;
#endif
		this.set_titlebar(header_bar);
		this.set_transient_for(parent);
		this.set_modal(true);

		Gtk.Button close_button = new Gtk.Button.with_label(_("Close"));
		close_button.clicked.connect(() => {
				this.hide();
			});
		header_bar.pack_start(close_button);

		_content = new Gtk.Box(Gtk.Orientation.VERTICAL, 12);
		_content.set_margin_top(12);
		_content.set_margin_bottom(12);
		_content.set_margin_start(12);
		_content.set_margin_end(12);
#if CROWN_GTK3
		this.add(_content);
#else
		this.set_child(_content);
#endif
	}

	public void set_content(string resource_path, GLib.HashTable<string, Value?> dependencies)
	{
#if CROWN_GTK3
		foreach (Gtk.Widget child in _content.get_children())
			child.destroy();
#else
		Gtk.Widget? child = _content.get_first_child();
		while (child != null) {
			Gtk.Widget? next = child.get_next_sibling();
			_content.remove(child);
			child = next;
		}
#endif

		GLib.GenericArray<Value?> dependency_items = (GLib.GenericArray<Value?>)dependencies["dependencies"];
		GLib.GenericArray<Value?> reference_items = (GLib.GenericArray<Value?>)dependencies["references"];
		GLib.GenericArray<Value?> dependent_items = (GLib.GenericArray<Value?>)dependencies["dependents"];
		GLib.GenericArray<Value?> referrer_items = (GLib.GenericArray<Value?>)dependencies["referrers"];

		Gtk.Label path_label = new Gtk.Label(resource_path);
		path_label.set_xalign(0.0f);
		path_label.wrap = true;
		path_label.wrap_mode = Pango.WrapMode.CHAR;
		path_label.set_selectable(true);
#if CROWN_GTK3
		_content.pack_start(path_label, false, false, 0);
#else
		_content.append(path_label);
#endif

		Gtk.Box columns = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 12);
		columns.set_margin_top(6);
		columns.set_margin_bottom(6);
		columns.set_margin_start(6);
		columns.set_margin_end(6);

		Gtk.Box left = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		left.hexpand = true;
		left.vexpand = true;

		Gtk.Box right = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		right.hexpand = true;
		right.vexpand = true;

#if CROWN_GTK3
		columns.pack_start(left, true, true, 0);
		columns.pack_start(right, true, true, 0);
#else
		columns.append(left);
		columns.append(right);
#endif

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

		if (dependency_items.length == 0
			&& reference_items.length == 0
			&& dependent_items.length == 0
			&& referrer_items.length == 0
			) {
			Gtk.Label empty = new Gtk.Label(_("No direct dependencies or references."));
			empty.set_xalign(0.0f);
#if CROWN_GTK3
			left.pack_start(empty, false, false, 0);
#else
			left.append(empty);
#endif
		}

#if CROWN_GTK3
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
#else
		left.visible = left.get_first_child() != null;
		right.visible = right.get_first_child() != null;
		_content.append(columns);
#endif /* if CROWN_GTK3 */
	}
}

} /* namespace Crown */
