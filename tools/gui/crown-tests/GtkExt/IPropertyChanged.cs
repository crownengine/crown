using System;

namespace crown_tests
{
	public class PropertyChangedEventArgs: EventArgs
	{
		public readonly String PropertyName;

		public PropertyChangedEventArgs(String propName)
		{
			PropertyName = propName;
		}
	}
	public delegate void PropertyChangedEventHandler(object sender, PropertyChangedEventArgs e);
	public interface IPropertyChanged
	{
		event PropertyChangedEventHandler PropertyChanged;
	}
}

