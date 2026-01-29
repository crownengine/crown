/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputEnum : InputField
{
	public bool _inconsistent;
	public Gtk.ListStore _store;
	public Gtk.TreeModelFilter _filter;
	public Gtk.ComboBox _combo;
	public Gtk.EventControllerScroll _controller_scroll;

	public override void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			_filter.refilter();

			if (_inconsistent) {
				_combo.set_active_id(INCONSISTENT_ID);
			}
		}
	}

	public override bool is_inconsistent()
	{
		return _inconsistent;
	}

	public override GLib.Value union_value()
	{
		return this.value;
	}

	public override void set_union_value(GLib.Value v)
	{
		this.value = (string)v;
	}

	public string value
	{
		get
		{
			return _combo.get_active_id();
		}
		set
		{
			_filter.refilter();
			bool success = _combo.set_active_id(value);
			set_inconsistent(!success);
		}
	}

	public bool filter_visible_func(Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value id_val;
		model.get_value(iter, 0, out id_val);

		if (!_inconsistent && (string)id_val == INCONSISTENT_ID)
			return false;

		return true;
	}

	public InputEnum(string default_id = "DEFAULT", string[]? labels = null, string[]? ids = null)
	{
		_inconsistent = false;

		_store = new Gtk.ListStore(2
			, typeof(string) // ID
			, typeof(string) // Label
			);
		_filter = new Gtk.TreeModelFilter(_store, null);
		_filter.set_visible_func(filter_visible_func);

		_combo = new Gtk.ComboBox();
		_combo.model = _filter;
		_combo.id_column = 0;
		_combo.entry_text_column = 1;

		Gtk.CellRendererText renderer = new Gtk.CellRendererText();
		_combo.pack_start(renderer, true);
		_combo.add_attribute(renderer, "text", 1);

		insert_special_values();

		if (labels != null) {
			Gtk.TreeIter iter;
			for (int ii = 0; ii < ids.length; ++ii) {
				unowned string? id = ids != null ? ids[ii] : null;
				_store.insert_with_values(out iter, -1, 0, id, 1, labels[ii], -1);
			}
			// if (ids != null && default_id < ids.length)
			this.value = default_id;
		}

		_combo.changed.connect(on_changed);

#if CROWN_GTK3
		_combo.scroll_event.connect(() => {
				GLib.Signal.stop_emission_by_name(_combo, "scroll-event");
				return Gdk.EVENT_PROPAGATE;
			});
#else
		_controller_scroll = new Gtk.EventControllerScroll(_combo, Gtk.EventControllerScrollFlags.BOTH_AXES);
		_controller_scroll.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_scroll.scroll.connect(() => {
				// Do nothing, just consume the event to stop
				// the annoying scroll default behavior.
			});
#endif

		this.add(_combo);
	}

	public void append(string? id, string label)
	{
		_combo.changed.disconnect(on_changed);
		Gtk.TreeIter iter;
		_store.insert_with_values(out iter, -1, 0, id, 1, label, -1);
		_combo.changed.connect(on_changed);
	}

	public void clear()
	{
		_combo.changed.disconnect(on_changed);
		_store.clear();
		insert_special_values();
		_inconsistent = false;
		_combo.changed.connect(on_changed);
	}

	public string any_valid_id()
	{
		string some_id = INCONSISTENT_ID;

		if (_store.iter_n_children(null) > 1u) {
			_store.foreach((model, path, iter) => {
					Value id_val;
					model.get_value(iter, 0, out id_val);
					if ((string)id_val != INCONSISTENT_ID) {
						some_id = (string)id_val;
						return true;
					}

					return false;
				});
		}

		return some_id;
	}

	public void on_changed()
	{
		if (_combo.get_active_id() == null)
			return;

		if (_inconsistent && _combo.get_active_id() == INCONSISTENT_ID)
			return;

		if (_inconsistent) {
			_inconsistent = false;
			_filter.refilter();
		}

		value_changed(this);
	}

	public void insert_special_values()
	{
		assert(_store.iter_n_children(null) == 0u);

		// Insert the "inconsistent" ID and label.
		Gtk.TreeIter iter;
		_store.insert_with_values(out iter, -1, 0, INCONSISTENT_ID, 1, INCONSISTENT_LABEL, -1);
	}
}

} /* namespace Crown */
