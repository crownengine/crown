using System;

namespace crown_tests.GtkExt
{
	sealed class TreeViewIterBindingTarget: IBindingTarget
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

		public event BindingTargetValueChangedEventHandler ValueChanged;

		public void Update(Binding binding, object newValue)
		{
			mTreeView.Model.EmitRowChanged(mTreeView.Model.GetPath(mIter), mIter);
		}

		#endregion
	}
}

