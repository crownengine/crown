using System;
using System.Collections.Generic;

namespace crown_tests.GtkExt
{
	public static class TreeViewTemplating
	{
		private static Dictionary<Gtk.TreeView, TreeViewTemplateInfo> mTemplateInfo;

		static TreeViewTemplating()
		{
			mTemplateInfo = new Dictionary<Gtk.TreeView, TreeViewTemplateInfo>();
		}

		private static TreeViewTemplateInfo GetInfo(Gtk.TreeView treeView)
		{
			TreeViewTemplateInfo info = null;
			if (!mTemplateInfo.TryGetValue(treeView, out info)) {
				info = new TreeViewTemplateInfo();
				mTemplateInfo[treeView] = info;
			}
			return info;
		}

		public static void AddRowTemplate(Gtk.TreeView treeView, TreeViewRowTemplate template)
		{
			GetInfo(treeView).RowTemplates.Add(template);
		}

		public static void ApplyTemplating(Gtk.TreeView treeView)
		{
			foreach (var col in treeView.Columns) {
				col.SetCellDataFunc(col.CellRenderers[0], ValuePropertyDataFunc);
			}
		}

		private static void ValuePropertyDataFunc(Gtk.TreeViewColumn column, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			var treeView = (Gtk.TreeView)column.TreeView;
			var info = GetInfo(treeView);

			var textCell = (cell as Gtk.CellRendererText);
			textCell.Text = string.Empty;
			var value = model.GetValue(iter, 0);
			if (value == null)
				return;

			foreach (var rowTemplate in info.RowTemplates) {
				if (value.GetType() == rowTemplate.TargetType) {
					//Here we have a value, which is the source for Binding, and a BindingInfo that is given by rowTemplate.ColumnBindings[column.Title] .
					//The instance of the BindingInfo is shared among all values (rows), since it was defined once in the rowTemplate.

					BindingInfo bindingInfo = null;
					if (!rowTemplate.ColumnBindings.TryGetValue(column.Title, out bindingInfo))
						return;

					//The actual binding, on the other hand, is specific to the current (row,column) pair.
					Binding binding = BindingEngine.GetOrCreateBinding(value, new TreeViewIterBindingTarget(treeView, iter, column), bindingInfo);
					var propValue = binding.GetSourceValue();
					textCell.Text = propValue == null ? String.Empty : propValue.ToString();
					return;
				}
			}
		}

		private class TreeViewTemplateInfo
		{
			public List<TreeViewRowTemplate> RowTemplates = new List<TreeViewRowTemplate>();
		}

		private class TreeViewIterBindingTarget: IBindingTarget
		{
			private Gtk.TreeIter mIter;
			private Gtk.TreeView mTreeView;
			private Gtk.TreeViewColumn mColumn;

			public TreeViewIterBindingTarget(Gtk.TreeView treeView, Gtk.TreeIter iter, Gtk.TreeViewColumn column)
			{
				mTreeView = treeView;
				mIter = iter;
				mColumn = column;
			}

			public override bool Equals(object obj)
			{
				var other = obj as TreeViewIterBindingTarget;
				if (other == null)
					return false;
				return other.mIter.Equals(mIter) && (mTreeView == other.mTreeView) && (mColumn == other.mColumn);
			}

			public override int GetHashCode()
			{
				return mIter.GetHashCode();
			}

			#region IBindingTarget implementation

			public void Update(Binding binding, object newValue)
			{
				mTreeView.Model.EmitRowChanged(mTreeView.Model.GetPath(mIter), mIter);
			}

			#endregion
		}
	}
}

