using System;

namespace crown_tests.GtkExt
{
	public class ViewModelBase: IPropertyChanged
	{
		public ViewModelBase()
		{
		}

		protected Boolean SetAndNotify<T>(ref T field, T newValue, String propertyName) {
			if (!object.Equals(field, newValue)) {
				field = newValue;
				Notify(propertyName);
				return true;
			}
			return false;
		}

		#region IPropertyChanged implementation

		public event PropertyChangedEventHandler PropertyChanged;

		protected void Notify(String propertyName)
		{
			if (PropertyChanged != null)
				PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
		}

		#endregion
	}
}

