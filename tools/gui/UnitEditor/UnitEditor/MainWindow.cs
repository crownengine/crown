using System;
using System.Collections.Generic;
using Gtk;

public partial class MainWindow: Gtk.Window
{
	public MainWindow () : base (Gtk.WindowType.Toplevel)
	{
		Build ();

		VBox box = new VBox (false, 2);

		UnitEditor.MainMenu mb = new UnitEditor.MainMenu ();
		this.AddAccelGroup (mb.uim.AccelGroup);
		box.PackStart(mb.instance, false, false, 0);

		UnitEditor.UnitNotebook unb = new UnitEditor.UnitNotebook ("/home/mikymod/samples/doodles/blue_doodle.unit");
		box.PackStart (unb.instance, false, false, 0);
		Add (box);

		ShowAll ();
	}

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}
}
