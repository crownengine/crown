using System;

namespace crown_tests.GtkExt
{
	public class ViewModelBase: IPropertyChanged
	{
		public ViewModelBase()
		{
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

