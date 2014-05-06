using System;
using Gtk;

namespace UnitEditor
{
	[System.ComponentModel.ToolboxItem (true)]
	public partial class MainMenu : Gtk.EventBox
	{
		public MenuBar instance;

		public MainMenu ()
		{
			instance = new MenuBar ();

			Menu file_menu = new Menu ();

			MenuItem file_item = new MenuItem("File");
			file_item.Submenu = file_menu;

			MenuItem exit_item = new MenuItem("Exit");
			exit_item.Activated += new EventHandler (file_chooser_cb);
			file_menu.Append (exit_item);

			instance.Append (file_item);

			ShowAll ();
		}

		static void exit_cb (object o, EventArgs args)
		{
			Application.Quit ();
		}

		static void file_chooser_cb(object sender, System.EventArgs e)
		{
			Gtk.FileChooserDialog fc = new Gtk.FileChooserDialog("Open file", null, FileChooserAction.Open);
			fc.AddButton(Stock.Cancel, ResponseType.Cancel);
			fc.AddButton(Stock.Open, ResponseType.Ok);
			fc.Filter = new FileFilter();
			fc.Filter.AddPattern("*.unit");
			fc.Filter.AddPattern("*.material");
			fc.Filter.AddPattern("*.physics");

			if (fc.Run() == (int)ResponseType.Ok)
			{
				Console.WriteLine(fc.Filename);
			}

			//Don't forget to call Destroy() or the FileChooserDialog window won't get closed.
			fc.Destroy();
		}
	}
}

