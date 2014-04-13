using System;
using System.Collections.Generic;

namespace crown_tests.GtkExt
{
	public class TreeViewRowTemplate
	{
		public Type TargetType;
		public Dictionary<String, BindingInfo> ColumnBindings;

		public TreeViewRowTemplate(Type targetType)
		{
			this.TargetType = targetType;
			this.ColumnBindings = new Dictionary<string, BindingInfo>();
		}

		public static TreeViewRowTemplate Create(Type targetType)
		{
			return new TreeViewRowTemplate(targetType);
		}

		public TreeViewRowTemplate SetBinding(String colName, String path)
		{
			ColumnBindings.Add(colName, new BindingInfo() { Path = path });
			return this;
		}

		public TreeViewRowTemplate SetBinding(String colName, String path, Func<object, object> Converter)
		{
			ColumnBindings.Add(colName, new BindingInfo() { Path = path, Converter = Converter });
			return this;
		}
	}
}
