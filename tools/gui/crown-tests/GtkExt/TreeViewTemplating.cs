using System;
using System.Collections.Generic;

namespace crown_tests.GtkExt
{
	public static class TreeViewTemplating
	{
		private static Dictionary<Gtk.TreeView, TreeViewTemplateInfo> mTemplateInfo;

		static TreeViewTemplating() {
			mTemplateInfo = new Dictionary<Gtk.TreeView, TreeViewTemplateInfo>();
		}

		private static TreeViewTemplateInfo GetInfo(Gtk.TreeView treeView)
		{
			TreeViewTemplateInfo info = null;
			if (!mTemplateInfo.TryGetValue (treeView, out info)) {
				info = new TreeViewTemplateInfo ();
				mTemplateInfo [treeView] = info;
			}
			return info;
		}

		public static void AddRowTemplate(Gtk.TreeView treeView, TreeViewRowTemplate template)
		{
			GetInfo (treeView).RowTemplates.Add (template);
		}

		public static void ApplyTemplating(Gtk.TreeView treeView)
		{
			foreach (var col in treeView.Columns) {
				col.SetCellDataFunc (col.CellRenderers[0], ValuePropertyDataFunc);
			}
		}

		private static void ValuePropertyDataFunc(Gtk.TreeViewColumn column, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			var info = GetInfo ((Gtk.TreeView)column.TreeView);


			var textCell = (cell as Gtk.CellRendererText);
			textCell.Text = string.Empty;
			var value = model.GetValue (iter, 0);
			if (value == null)
				return;

			foreach (var rowTemplate in info.RowTemplates) {
				if (value.GetType () == rowTemplate.TargetType) {
					Binding b = null;
					if (!rowTemplate.ColumnBindings.TryGetValue (column.Title, out b))
						return;

					var propInfo = value.GetType ().GetProperty (b.Path);
					if (propInfo == null)
						return;

					var propValue = propInfo.GetValue (value, null);
					if (b.Converter != null)
						propValue = b.Converter (propValue);
					textCell.Text = propValue == null ? String.Empty : propValue.ToString ();
					return;
				}
			}
		}

		private class TreeViewTemplateInfo
		{
			public List<TreeViewRowTemplate> RowTemplates = new List<TreeViewRowTemplate> ();
		}
	}
}

