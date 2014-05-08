using System;
using System.Collections.Generic;
using Gtk;

using UnitEditor;

public partial class MainWindow: Gtk.Window
{
	private VBox box;
	private Notebook nb;
	private UnitForm uf = null;

	public MainWindow () : base (Gtk.WindowType.Toplevel)
	{
		Build ();

		box = new VBox (false, 2);
		Add (box);

		UnitEditor.MainMenu mb = new UnitEditor.MainMenu ();
		this.AddAccelGroup (mb.uim.AccelGroup);
		box.PackStart(mb.instance, false, false, 0);

		nb = new Notebook ();
		box.PackStart (nb, false, false, 0);

		ShowAll ();
	}

	public void open_unit(string file_name)
	{
		uf = new UnitEditor.UnitForm (file_name);
		nb.AppendPage (uf.instance, new Label (file_name));

		ShowAll ();
	}

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}
}
