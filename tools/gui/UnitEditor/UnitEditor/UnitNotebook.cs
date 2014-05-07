using System;
using System.Collections.Generic;
using Gtk;

namespace UnitEditor
{
	[System.ComponentModel.ToolboxItem (true)]
	public partial class UnitNotebook : Gtk.EventBox
	{
		public Notebook instance;


		public UnitNotebook (string file_name)
		{
			instance = new Notebook ();

			UnitEditor.RenderablesList renderables_list = new UnitEditor.RenderablesList (file_name);
			instance.AppendPage (renderables_list, new Label("Renderables"));
		}
	}
}

