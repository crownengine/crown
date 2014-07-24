using System;
using Gtk;
using System;

public partial class MainWindow: Gtk.Window
{
	public const string SDK_DIR = Environment.GetEnvironmentVariable ("CROWN_INSTALLATION_DIR");

	public string project_name;
	public string source_path;
	public string destination_path;

	public MainWindow () : base (Gtk.WindowType.Toplevel)
	{
		Build ();
	}

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}

	protected void OnProjectButtonClicked (object sender, EventArgs e)
	{
		starter.ProjectDialog pd = new starter.ProjectDialog (this);
		pd.Run ();
		pd.Destroy ();

		project_entry.Text = project_name;
	}
}
