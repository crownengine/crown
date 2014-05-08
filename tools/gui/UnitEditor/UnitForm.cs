using System;
using System.Collections.Generic;
using Gtk;

namespace UnitEditor
{
	[System.ComponentModel.ToolboxItem (true)]
	public partial class UnitForm : Gtk.EventBox
	{
		public Notebook instance;


		public UnitForm (string unit_file_name)
		{
			instance = new Notebook ();

			UnitEditor.RenderablesList renderables_list = new UnitEditor.RenderablesList (unit_file_name);
			instance.AppendPage (renderables_list, new Label("Renderables"));
		}
	}
}

