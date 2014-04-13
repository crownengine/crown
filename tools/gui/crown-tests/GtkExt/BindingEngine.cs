using System;
using System.Collections.Generic;

namespace crown_tests
{
	public static class BindingEngine
	{
		private static Dictionary<IBindingTarget, Binding> mBindings;

		static BindingEngine() {
			mBindings = new Dictionary<IBindingTarget, Binding>();
		}

		public static Binding GetOrCreateBinding(Object source, IBindingTarget target, BindingInfo info) {
			Binding result = null;
			if (!mBindings.TryGetValue(target, out result)) {
				result = new Binding();
				result.Info = info;
				result.Connect(source, target);
				mBindings[target] = result;
			}
			return result;
		}
	}

	//BindingInfo holds binding informations shared among all Bindings of the same type. For example, a TreeViewRowTemplate that is applied to 
	//multiple rows has a BindingInfo for each column, that is shared among Binding instance for single rows.
	public class BindingInfo
	{
		public String Path;
		public Func<object, object> Converter;
	}

	//A Binding si an association between two properties. Usually the source is a ViewModel property and the Target a View property. 
	//The Source and the Target are determined based on the context of application: When inserted in a Template, upon first application the two
	//are determined and set, attaching necessary event handlers to manage the update when one of them changes.
	public class Binding
	{
		public BindingInfo Info;

		private Object mSource;
		private IBindingTarget mTarget;

		public void Connect(Object source, IBindingTarget target) {
			mSource = source;
			mTarget = target;

			var propertyChanged = mSource as IPropertyChanged;
			if (propertyChanged != null) {
				propertyChanged.PropertyChanged += (object sender, PropertyChangedEventArgs e) => mTarget.Update(this, GetSourceValue());
			}
		}

		public Object GetSourceValue() {
			var propInfo = mSource.GetType().GetProperty(Info.Path);
			if (propInfo == null)
				return null;

			var propValue = propInfo.GetValue(mSource, null);
			if (Info.Converter != null)
				propValue = Info.Converter(propValue);
			return propValue;
		}
	}

	public interface IBindingTarget
	{
		void Update(Binding binding, Object newValue);
	}
}

