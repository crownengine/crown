/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

[CCode (cname = "crown_infinite_drag_sampler_start")]
extern void* infinite_drag_sampler_start(Gdk.Display display, Gdk.Window window, Gdk.Device device, int anchor_x, int anchor_y, int cancel_button);
[CCode (cname = "crown_infinite_drag_sampler_drain")]
extern void infinite_drag_sampler_drain(void* sampler, out double delta_x, out double delta_y, out int samples);
[CCode (cname = "crown_infinite_drag_sampler_released")]
extern bool infinite_drag_sampler_released(void* sampler);
[CCode (cname = "crown_infinite_drag_sampler_cancel_requested")]
extern bool infinite_drag_sampler_cancel_requested(void* sampler);
[CCode (cname = "crown_infinite_drag_sampler_stop")]
extern void infinite_drag_sampler_stop(void* sampler, out double delta_x, out double delta_y, out int samples);

namespace Crown
{
public class InfiniteDragController : GLib.Object
{
	public enum Axis { X, Y, XY }

	public double activation_margin = 5.0;
	public int activation_poll_ms = 1;
	public int update_interval_ms = 8;
	public Axis axis_mode = Axis.X;
	/// X11 button that cancels the drag instead of committing it; 0 disables it.
	public int cancel_button = Gdk.BUTTON_SECONDARY;

	public signal void drag_started();
	public signal void drag_delta(double dx, double dy, double total_dx, double total_dy);
	public signal void drag_committed();
	public signal void drag_cancelled();
	public signal void drag_finished(bool was_dragging);
	/// Sampler ended the session on its own; adapters must reset their gesture when this fires.
	public signal void release_detected_externally();

	public bool dragging { get { return _dragging; } }

	private unowned Gtk.Widget _widget;
	private void* _sampler;
	private uint _activation_timeout_id;
	private uint _update_timeout_id;
	private bool _pressed;
	private bool _dragging;
	private bool _dirty;
	private double _total_dx;
	private double _total_dy;
	private double _last_dx;
	private double _last_dy;

	public InfiniteDragController(Gtk.Widget widget)
	{
		_widget = widget;
		_widget.destroy.connect(() => cancel());
	}

	/// Call from the adapter's own gesture "pressed" handler.
	public void start(int root_x, int root_y)
	{
		_pressed = true;
		_dragging = false;
		_dirty = false;
		_total_dx = 0.0;
		_total_dy = 0.0;

		if (_sampler == null) {
			Gdk.Device pointer = _widget.get_display().get_default_seat().
				get_pointer();
			_sampler = infinite_drag_sampler_start(_widget.get_display(), _widget.
				get_window(), pointer, root_x, root_y, cancel_button);
		}
		if (_sampler == null) {
			loge("InfiniteDragController: sampler failed to start");
			_pressed = false;
			return;
		}

		_activation_timeout_id = GLib.Timeout.add(activation_poll_ms,
			on_activation_poll);
		_update_timeout_id = GLib.Timeout.add(update_interval_ms,
			on_update_poll);
	}

	/// Call from the adapter's own gesture "released" handler.
	public void release()
	{
		finish(true, false);
	}

	/// Call from the adapter's own gesture "cancel" handler (or on destroy).
	public void cancel()
	{
		finish(false, false);
	}

	/// forced skips the was_dragging check, for an explicit cancel request.
	private void finish(bool commit, bool forced)
	{
		if (!_pressed)
			return;

		_pressed = false;
		stop_timers();
		stop_sampler(commit);

		bool was_dragging = _dragging;
		_dragging = false;
		if (was_dragging || forced) {
			if (commit)
				drag_committed();
			else
				drag_cancelled();
		}
		drag_finished(was_dragging || forced);
	}

	private void stop_timers()
	{
		if (_activation_timeout_id != 0)
			GLib.Source.remove(_activation_timeout_id);
		if (_update_timeout_id != 0)
			GLib.Source.remove(_update_timeout_id);
		_activation_timeout_id = 0;
		_update_timeout_id = 0;
	}

	private bool on_activation_poll()
	{
		if (!_pressed || _dragging) {
			_activation_timeout_id = 0;
			return GLib.Source.REMOVE;
		}
		if (check_ended()) {
			_activation_timeout_id = 0;
			return GLib.Source.REMOVE;
		}
		if (_dragging) {
			_activation_timeout_id = 0;
			return GLib.Source.REMOVE;
		}
		return GLib.Source.CONTINUE;
	}

	private bool on_update_poll()
	{
		if (!_pressed) {
			_update_timeout_id = 0;
			return GLib.Source.REMOVE;
		}
		if (check_ended())
			return GLib.Source.REMOVE;

		if (_dragging && _dirty) {
			_dirty = false;
			drag_delta(_last_dx, _last_dy, _total_dx, _total_dy);
		}
		return GLib.Source.CONTINUE;
	}

	/// Returns true if the sampler ended the session (release or cancel).
	private bool check_ended()
	{
		double dx, dy;
		int samples;
		infinite_drag_sampler_drain(_sampler, out dx, out dy, out samples);
		process_samples(dx, dy);

		bool cancel_requested = infinite_drag_sampler_cancel_requested(_sampler);
		bool released = infinite_drag_sampler_released(_sampler);
		if (!cancel_requested && !released)
			return false;

		release_detected_externally();
		finish(!cancel_requested, cancel_requested);
		return true;
	}

	private void process_samples(double dx, double dy)
	{
		_last_dx = dx;
		_last_dy = dy;
		if (axis_mode != Axis.Y)
			_total_dx += dx;
		if (axis_mode != Axis.X)
			_total_dy += dy;

		if (!_dragging) {
			double magnitude = (axis_mode == Axis.Y) ? _total_dy.abs() : _total_dx.
				abs();
			if (magnitude > activation_margin) {
				_dragging = true;
				_dirty = true;
				drag_started();
			}
		} else if (dx != 0 || dy != 0) {
			_dirty = true;
		}
	}

	private void stop_sampler(bool process_final_samples)
	{
		if (_sampler == null)
			return;

		double dx, dy;
		int samples;
		infinite_drag_sampler_stop(_sampler, out dx, out dy, out samples);
		_sampler = null;
		if (process_final_samples)
			process_samples(dx, dy);
	}
}

} /* namespace Crown */
