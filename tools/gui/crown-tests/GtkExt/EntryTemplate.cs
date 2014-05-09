using System;

namespace crown_tests.GtkExt
{
	public class EntryTemplate: ITemplate
	{
		private BindingInfo mTextBinding;

		public EntryTemplate()
		{
		}

		public EntryTemplate SetTextBinding(String path)
		{
			mTextBinding = new BindingInfo() { Path = path };
			return this;
		}

		public void Apply(Gtk.Widget widget) 
		{
			Gtk.Entry entry = widget as Gtk.Entry;
			if (entry == null) {
				Console.WriteLine("EntryTemplate.Apply: Invalid widget type for this template");
				return;
			}

			if (mTextBinding != null) {
				var bindingTarget = new EntryBindingTarget(entry);
				Binding binding = BindingEngine.GetOrCreateBinding(widget, null, bindingTarget, mTextBinding);
				bindingTarget.Update(binding, binding.GetSourceValue());
			}
		}
	}
}

