/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class DialogLevelChanged : Gtk.MessageDialog
{
	public DialogLevelChanged(Gtk.Window? parent)
	{
		Object(text: "Save changes to Level before closing?"
			, message_type: Gtk.MessageType.WARNING
			, modal: true
			);

		add_buttons("Close _without Saving"
			, ResponseType.NO
			, "_Cancel"
			, ResponseType.CANCEL
			, "_Save"
			, ResponseType.YES
			);
		set_default_response(ResponseType.YES);

		if (parent != null)
			this.set_transient_for(parent);
	}
}

}
