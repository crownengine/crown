/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class ComboBoxMap : Gtk.ComboBox, Property
{
	// Data
	public bool _stop_emit;
	public bool _inconsistent;
	public Gtk.ListStore _store;
	public Gtk.TreeModelFilter _filter;

	// Signals
	public signal void value_changed();

	public void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			_filter.refilter();

			if (_inconsistent) {
				_stop_emit = true;
				this.set_active_id(INCONSISTENT_ID);
				_stop_emit = false;
			}
		}
	}

	public bool is_inconsistent()
	{
		return _inconsistent;
	}

	public Value? generic_value()
	{
		return this.value;
	}

	public void set_generic_value(Value? val)
	{
		this.value = (string)val;
	}

	public string? value
	{
		get
		{
			return this.get_active_id();
		}
		set
		{
			_stop_emit = true;
			_filter.refilter();
			bool success = this.set_active_id((string)value);
			_stop_emit = false;
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

	public ComboBoxMap(int default_id = 0, string[]? labels = null, string[]? ids = null)
	{
		// Data
		_stop_emit = false;
		_inconsistent = false;

		_store = new Gtk.ListStore(2
			, typeof(string) // ID
			, typeof(string) // Label
			);
		_filter = new Gtk.TreeModelFilter(_store, null);
		_filter.set_visible_func(filter_visible_func);

		this.model = _filter;
		this.id_column = 0;
		this.entry_text_column = 1;

		Gtk.CellRendererText renderer = new Gtk.CellRendererText ();
		this.pack_start(renderer, true);
		this.add_attribute(renderer, "text", 1);

		insert_special_values();

		if (labels != null) {
			Gtk.TreeIter iter;
			for (int ii = 0; ii < ids.length; ++ii) {
				unowned string? id = ids != null ? ids[ii] : null;
				_store.insert_with_values(out iter, -1, 0, id, 1, labels[ii], -1);
			}
			if (ids != null && default_id < ids.length)
				this.value = ids[default_id];
		}

		// Widgets
		this.changed.connect(on_changed);
		this.scroll_event.connect(on_scroll);
	}

	public void append(string? id, string label)
	{
		Gtk.TreeIter iter;
		_store.insert_with_values(out iter, -1, 0, id, 1, label, -1);
	}

	public void clear()
	{
		_stop_emit = true;
		_store.clear();
		insert_special_values();
		_inconsistent = false;
		_stop_emit = false;
	}

	private void on_changed()
	{
		if (_inconsistent && this.get_active_id() == INCONSISTENT_ID)
			return;

		if (!_stop_emit) {
			if (_inconsistent) {
				_inconsistent = false;
				_filter.refilter();
			}
			value_changed();
		}
	}

	private bool on_scroll(Gdk.EventScroll ev)
	{
		GLib.Signal.stop_emission_by_name(this, "scroll-event");
		return Gdk.EVENT_PROPAGATE;
	}

	private void insert_special_values()
	{
		assert(_store.iter_n_children(null) == 0u);

		// Insert the "inconsistent" ID and label.
		Gtk.TreeIter iter;
		_store.insert_with_values(out iter, -1, 0, INCONSISTENT_ID, 1, INCONSISTENT_LABEL, -1);
	}
}

} /* namespace Crown */
