using System;
using System.Collections.Generic;

namespace crown_tests.GtkExt
{
	public static class BindingEngine
	{
		private static Dictionary<IBindingTarget, Binding> mBindings;
		private static Dictionary<Gtk.Widget, ViewModelBase> mViewModels;

		static BindingEngine() {
			mBindings = new Dictionary<IBindingTarget, Binding>();
			mViewModels = new Dictionary<Gtk.Widget, ViewModelBase>();
		}

		public static void SetViewModel(Gtk.Widget widget, ViewModelBase viewModel)
		{
			if (mViewModels.ContainsKey(widget)) {
				Console.WriteLine("BindingEngine.SetViewModel: ViewModel already set for this widget");
				return;
			}
			mViewModels.Add(widget, viewModel);
		}

		public static ViewModelBase GetViewModel(Gtk.Widget widget)
		{
			ViewModelBase viewModel = null;
			mViewModels.TryGetValue(widget, out viewModel);
			return viewModel;
		}

		public static Binding GetOrCreateBinding(Gtk.Widget widget, Object source, IBindingTarget target, BindingInfo info) 
		{
			Binding result = null;
			if (!mBindings.TryGetValue(target, out result)) {
				result = new Binding();
				result.Info = info;
				result.Connect(widget, source, target);
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

		public void Connect(Gtk.Widget widget, Object source, IBindingTarget target) {
			if (source == null) {
				mSource = BindingEngine.GetViewModel(widget);
				if (mSource == null) {
					Console.WriteLine("Binding.Connect: ViewModel not set for this widget");
					return;
				}
			} else {
				mSource = source;
			}

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

	public class BindingTargetValueChangedEventArgs: EventArgs
	{
		public Object NewValue;
	}
	public delegate void BindingTargetValueChangedEventHandler(object sender, BindingTargetValueChangedEventArgs e);

	public interface IBindingTarget
	{

		void Update(Binding binding, Object newValue);
		event BindingTargetValueChangedEventHandler ValueChanged;
	}
}

