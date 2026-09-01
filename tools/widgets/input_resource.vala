/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputResource : InputField
{
#if CROWN_GTK3
	public const Gtk.TargetEntry[] DND_TARGETS =
	{
		{ "RESOURCE_PATH", Gtk.TargetFlags.SAME_APP, TargetInfo.RESOURCE_PATH },
	};
#endif
	public const string UNSET_RESOURCE = _("(None)");

	public string _type;
	public bool _name_unset;
	public bool _nullable;
	public Gtk.Entry _name;
	public Gtk.Button _selector;
	public Gtk.Button _revealer;
	public SelectResourceDialog _dialog;
	public Gtk.EventControllerKey _controller_key;
	public Gtk.GestureSingle _gesture_click;
#if !CROWN_GTK3
	public Gtk.EventControllerFocus _controller_focus;
	public Gtk.DropTarget _drop_target;
#endif
	public Gtk.Box _box;

	public override void set_inconsistent(bool inconsistent)
	{
	}

	public override bool is_inconsistent()
	{
		return false;
	}

	public override GLib.Value union_value()
	{
		return this.value;
	}

	public override void set_union_value(GLib.Value v)
	{
		this.value = (string?)v;
	}

	public string? value
	{
		get
		{
			return _name_unset ? null : _name.text;
		}
		set
		{
			_name_unset = value == null;
			_name.text = value != null ? value : UNSET_RESOURCE;
			_revealer.sensitive = value != null;
		}
	}

	public InputResource(string type, Database db)
	{
		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
#if CROWN_GTK3
		_box.get_style_context().add_class(Gtk.STYLE_CLASS_LINKED);
#else
		_box.add_css_class("linked");
#endif

		// Data
		_type = type;
		_name_unset = true;
		_nullable = false;

		// Widgets
		_name = new Gtk.Entry();
		_name.set_width_chars(0);
		_name.set_editable(false);
		_name.hexpand = true;
		_name.changed.connect(on_name_value_changed);
		_name.activate.connect(on_name_activate);
#if CROWN_GTK3
		_name.focus_in_event.connect(on_name_focus_in);
		_name.focus_out_event.connect(on_name_focus_out);
		_box.pack_start(_name, true, true);
#else
		_controller_focus = new Gtk.EventControllerFocus();
		_controller_focus.enter.connect(on_name_focus_enter);
		_controller_focus.leave.connect(on_name_focus_leave);
		_name.add_controller(_controller_focus);
		_box.append(_name);
#endif

#if CROWN_GTK3
		_gesture_click = new Gtk.GestureMultiPress(_name);
		((Gtk.GestureMultiPress)_gesture_click).pressed.connect(on_name_button_pressed);
		((Gtk.GestureMultiPress)_gesture_click).released.connect(on_name_button_released);
#else
		_gesture_click = new Gtk.GestureClick();
		((Gtk.GestureClick)_gesture_click).pressed.connect(on_name_button_pressed);
		((Gtk.GestureClick)_gesture_click).released.connect(on_name_button_released);
		_name.add_controller(_gesture_click);
#endif

		_revealer = new Gtk.Button.from_icon_name("go-jump-symbolic");
		_revealer.clicked.connect(on_revealer_clicked);
		_revealer.set_tooltip_text(_("Reveal in the project browser."));
#if CROWN_GTK3
		_revealer.set_can_focus(false);
		_box.pack_end(_revealer, false);
#else
		_revealer.focusable = false;
#endif

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.set_tooltip_text(_("Select a resource."));
		_selector.clicked.connect(on_selector_clicked);
#if CROWN_GTK3
		_selector.set_can_focus(false);
		_box.pack_end(_selector, false);

		_controller_key = new Gtk.EventControllerKey(_name);
#else
		_selector.focusable = false;
		_box.append(_selector);
		_box.append(_revealer);

		_controller_key = new Gtk.EventControllerKey();
#endif
		_controller_key.key_pressed.connect(on_key_pressed);
#if !CROWN_GTK3
		_name.add_controller(_controller_key);
#endif

#if CROWN_GTK3
		Gtk.drag_dest_set(_name
			, Gtk.DestDefaults.MOTION
			| Gtk.DestDefaults.HIGHLIGHT
			, DND_TARGETS
			, Gdk.DragAction.COPY
			);
		_name.drag_data_received.connect(on_drag_data_received);
		_name.drag_motion.connect(on_drag_motion);
		_name.drag_drop.connect(on_drag_drop);
#else
		_drop_target = new Gtk.DropTarget(typeof(string), Gdk.DragAction.COPY);
		_drop_target.drop.connect(on_drag_drop);
		_name.add_controller(_drop_target);
#endif

		this.value = null;

		db._project.file_added.connect(on_file_added_or_changed);
		db._project.file_changed.connect(on_file_added_or_changed);
		db._project.file_removed.connect(on_file_removed);

#if CROWN_GTK3
		this.add(_box);
#else
		this.set_child(_box);
#endif
	}

	public void on_selector_clicked()
	{
		if (_dialog == null) {
			_dialog = ((LevelEditorApplication)GLib.Application.get_default()).new_select_resource_dialog(_type);
			_dialog.resource_selected.connect(on_select_resource_dialog_resource_selected);
		}

#if CROWN_GTK3
		_dialog.show_all();
#else
		_dialog.show();
#endif
		_dialog.present();
	}

	public void on_select_resource_dialog_resource_selected(string type, string name)
	{
		this.value = name;
		_dialog.hide();
	}

	public void on_revealer_clicked()
	{
		var tuple = new GLib.Variant.tuple({new GLib.Variant.int32((int)ProjectStore.RowKind.RESOURCE), _type, this.value});
		GLib.Application.get_default().activate_action("reveal-resource", tuple);
	}

	public void on_name_value_changed()
	{
		value_changed(this);
	}

	public void on_name_button_pressed(int n_press, double x, double y)
	{
		_name.grab_focus();
	}

	public void on_name_button_released(int n_press, double x, double y)
	{
		if (_gesture_click.get_current_button() == Gdk.BUTTON_PRIMARY)
			GLib.Idle.add(on_name_select);
	}

	public bool on_name_select()
	{
		_name.set_position(-1);
		_name.select_region(0, -1);
		return GLib.Source.REMOVE;
	}

	public void on_name_activate()
	{
		_name.select_region(0, 0);
		_name.set_position(-1);
	}

	public void name_focus_in()
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_name);

		_name.set_position(-1);
		_name.select_region(0, -1);
	}

	public void name_focus_out()
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_name);

		_name.select_region(0, 0);
	}

#if CROWN_GTK3
	public bool on_name_focus_in(Gdk.EventFocus ev)
	{
		name_focus_in();
		return Gdk.EVENT_PROPAGATE;
	}
#endif

#if CROWN_GTK3
	public bool on_name_focus_out(Gdk.EventFocus ev)
	{
		name_focus_out();
		return Gdk.EVENT_PROPAGATE;
	}
#endif

#if !CROWN_GTK3
	public void on_name_focus_enter()
	{
		name_focus_in();
	}
#endif /* if !CROWN_GTK3 */

#if !CROWN_GTK3
	public void on_name_focus_leave()
	{
		name_focus_out();
	}
#endif /* if !CROWN_GTK3 */

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval != Gdk.Key.Delete)
			return Gdk.EVENT_PROPAGATE;

		if (_nullable)
			this.value = null;

		return Gdk.EVENT_PROPAGATE;
	}

	public void on_file_added_or_changed(string type, string name, uint64 size, uint64 mtime)
	{
		if (type == _type && name == _name.text)
			value_changed(this);
	}

	public void on_file_removed(string type, string name)
	{
		if (type == _type && name == _name.text)
			value_changed(this);
	}

#if CROWN_GTK3
	public bool on_drag_motion(Gdk.DragContext context, int x, int y, uint time_)
	{
		Gdk.Atom target = Gtk.drag_dest_find_target(_name, context, null);
		Gdk.drag_status(context, target != Gdk.Atom.NONE ? Gdk.DragAction.COPY : 0, time_);
		return true;
	}
#endif

#if CROWN_GTK3
	public bool on_drag_drop(Gdk.DragContext context, int x, int y, uint time_)
	{
		Gdk.Atom target = Gtk.drag_dest_find_target(_name, context, null);
		if (target == Gdk.Atom.NONE)
			return false;

		Gtk.drag_get_data(_name, context, target, time_);
		return true;
	}
#endif

#if !CROWN_GTK3
	public bool on_drag_drop(GLib.Value value, double x, double y)
	{
		string resource_path = (string)value;
		if (ResourceId.type(resource_path) != _type)
			return false;

		string? resource_name = ResourceId.name(resource_path);
		if (resource_name == null)
			return false;

		this.value = resource_name;
		return true;
	}
#endif

#if CROWN_GTK3
	public void on_drag_data_received(Gdk.DragContext context, int x, int y, Gtk.SelectionData data, uint info, uint time_)
	{
		unowned uint8[] raw_data = data.get_data_with_length();
		bool success = false;

		if (raw_data.length != -1) {
			string resource_path = (string)raw_data;
			if (ResourceId.type(resource_path) == _type) {
				string? resource_name = ResourceId.name(resource_path);
				if (resource_name != null) {
					this.value = resource_name;
					success = true;
				}
			}
		}

		Gtk.drag_finish(context, success, false, time_);
	}
#endif /* if CROWN_GTK3 */
}

} /* namespace Crown */
