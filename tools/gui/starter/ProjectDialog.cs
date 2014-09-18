using System;
using Gtk;

namespace starter
{
	public partial class ProjectDialog : Gtk.Dialog
	{
		MainWindow win;

		public ProjectDialog (MainWindow win)
		{
			this.Build ();
			this.win = win;
		}

		protected void OnProjectDialogOkClicked (object sender, EventArgs e)
		{
			win.m_project_name = name_entry.Text;
			win.m_source_path = source_entry.Text;
		}

		protected void OnSourceButtonClicked (object sender, EventArgs e)
		{
			Gtk.FileChooserDialog fc = new Gtk.FileChooserDialog("Choose the file to open",
				this.win,
				FileChooserAction.SelectFolder,
				"Cancel", ResponseType.Cancel,
				"Open", ResponseType.Accept);
			if (fc.Run () == (int)ResponseType.Accept) 
			{
				source_entry.Text = fc.Filename;
			}

			fc.Destroy ();
		}
	}
}

