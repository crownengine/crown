using System;

namespace crown_tests.GtkExt
{
	sealed class EntryBindingTarget: IBindingTarget
	{
		private Gtk.Entry mEntry;

		public EntryBindingTarget(Gtk.Entry Entry)
		{
			mEntry = Entry;
		}

		public override bool Equals(object obj)
		{
			var other = obj as EntryBindingTarget;
			if (other == null)
				return false;
			return (mEntry == other.mEntry);
		}

		public override int GetHashCode()
		{
			return mEntry.GetHashCode();
		}

		#region IBindingTarget implementation

		public event BindingTargetValueChangedEventHandler ValueChanged;

		public void Update(Binding binding, object newValue)
		{
			mEntry.Text = newValue == null ? String.Empty : newValue.ToString();
		}

		#endregion
	}
}

