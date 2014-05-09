using System;
using System.Collections.Generic;

namespace crown_tests.GtkExt
{
	public interface ITemplate
	{
		void Apply(Gtk.Widget widget);
	}

	public static class Templating
	{
		private static Dictionary<Gtk.Widget, ITemplate> mTemplates;

		static Templating()
		{
			mTemplates = new Dictionary<Gtk.Widget, ITemplate>();
		}

		public static ITemplate GetTemplate(Gtk.Widget widget)
		{
			ITemplate template = null;
			mTemplates.TryGetValue(widget, out template);
			return template;
		}

		public static void ApplyTemplate(Gtk.Widget widget, ITemplate template)
		{
			if (GetTemplate(widget) != null) {
				Console.WriteLine("Templating.ApplyTemplate: template already applied for this widget");
				return;
			}

			mTemplates.Add(widget, template);
			template.Apply(widget);
		}
	}
}

