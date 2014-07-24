using System;
using Gtk;

public partial class MainWindow: Gtk.Window
{
	// public string SDK_DIR = Environment.GetEnvironmentVariable ("CROWN_INSTALLATION_DIR");
	enum Platform
	{
		Linux32 = 0,
		Linux64,
		Windows32,
		Windows64,
		Android
	}

	enum BuildMode
	{
		Debug = 0,
		Development,
		Release
	}

	public string project_name = null;
	public string source_path = null;
	public string destination_path = null;


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

	protected void OnRunButtonClicked (object sender, EventArgs e)
	{
		Platform platform = (Platform) platform_combobox.Active;
		BuildMode build = (BuildMode) build_combobox.Active;
		// FIXME: Workaround
		Environment.SetEnvironmentVariable("CROWN_INSTALL_DIR", "/home/mikymod/CrownSDK", EnvironmentVariableTarget.Process);
		string executable = Environment.GetEnvironmentVariable("CROWN_INSTALL_DIR") + "/";
		Console.WriteLine (executable);
		switch (platform)
		{
			case Platform.Linux32:
			{
				switch (build)
				{
				case BuildMode.Debug: executable += "crown-linux-debug32"; break;
				case BuildMode.Development: executable += "crown-linux-development32"; break;
				case BuildMode.Release: executable += "crown-linux-release32"; break;
				}
				break;
			}
			case Platform.Linux64:
			{
				switch (build)
				{
				case BuildMode.Debug: executable += "crown-linux-debug64"; break;
				case BuildMode.Development: executable += "crown-linux-development64"; break;
				case BuildMode.Release: executable += "crown-linux-release64"; break;
				}
				break;
			}
			case Platform.Windows32:
			{
				switch (build)
				{
				case BuildMode.Debug: executable += "crown-windows-debug32"; break;
				case BuildMode.Development: executable += "crown-windows-development32"; break;
				case BuildMode.Release: executable += "crown-windows-release32"; break;
				}
				break;
			}
			case Platform.Windows64:
			{
				switch (build)
				{
				case BuildMode.Debug: executable += "crown-windows-debug64"; break;
				case BuildMode.Development: executable += "crown-windows-development64"; break;
				case BuildMode.Release: executable += "crown-windows-release64"; break;
				}
				break;
			}
		}

		executable += " --source-dir " + source_path;
		executable += " --bundle-dir " + destination_path;
		executable += " --compile --continue";
		Console.WriteLine (executable);
	}

}
