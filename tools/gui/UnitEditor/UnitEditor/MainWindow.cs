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

		List<UnitEditor.Renderable> renderables = new List<UnitEditor.Renderable> ();
		renderables.Add (new UnitEditor.Renderable ("a", "b", "c", true));

		UnitEditor.RenderablesList renderables_list = new UnitEditor.RenderablesList (renderables);
		box.PackStart(renderables_list, false, false, 0);

		Add (box);

		ShowAll ();
	}

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}
}
