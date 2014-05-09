using System;
using Gtk;

namespace UnitEditor
{
	class MainClass
	{
		public static MainWindow g_win;

		public static void Main (string[] args)
		{
			Application.Init ();
			g_win = new MainWindow ();
			g_win.ShowAll ();
			Application.Run ();
		}
	}
}
