using System;
using System.Collections.Generic;
using Gtk;

using UnitEditor;

public partial class MainWindow: Gtk.Window
{
	private VBox box;
	private UnitForm unb = null;

	public MainWindow () : base (Gtk.WindowType.Toplevel)
	{
		Build ();

		box = new VBox (false, 2);
		Add (box);

		UnitEditor.MainMenu mb = new UnitEditor.MainMenu ();
		this.AddAccelGroup (mb.uim.AccelGroup);
		box.PackStart(mb.instance, false, false, 0);


		ShowAll ();
	}

	public void open_unit(string file_name)
	{
		unb = new UnitEditor.UnitForm (file_name);
		box.PackStart (unb.instance, false, false, 0);
		box.ShowAll ();
	}

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}
}
