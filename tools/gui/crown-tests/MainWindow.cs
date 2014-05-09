using System;
using Gtk;
using crown_tests.tests;
using Newtonsoft.Json;
using crown_tests.GtkExt;

namespace crown_tests
{
	public partial class MainWindow : Gtk.Window
	{
		private TestContainer mContainer;

		public MainWindow() : 
			base(Gtk.WindowType.Toplevel)
		{
			this.Build();

			twTests.AppendColumn("Name", new Gtk.CellRendererText());
			twTests.AppendColumn("State", new Gtk.CellRendererText());
			Templating.ApplyTemplate(twTests,
				new TreeViewTemplate()
				  .AddRowTemplate(TreeViewRowTemplate.Create(typeof(TestCategory))
																						 .SetBinding("Name", "Name"))
					.AddRowTemplate(TreeViewRowTemplate.Create(typeof(Test))
																						 .SetBinding("Name", "Name")
																						 .SetBinding("State", "LastResult")));

			LoadConfigData();
			LoadTestsData();
		}

		protected void OnDeleteEvent(object sender, DeleteEventArgs a)
		{
			Application.Quit();
			a.RetVal = true;
		}

		#region "My Code"

		private void btnCreate_Click(object o, EventArgs args)
		{
			var creator = new TestSourceCreator(mContainer, txtTestFolder.Text);
			creator.Create();
		}

		private void btnExecute_Click(object o, EventArgs args)
		{
			var executor = new TestExecutor(mContainer, txtCrownTestsExe.Text);
			executor.ExecuteAll();
		}

		private void LoadConfigData()
		{
			txtTestFolder.Text = @"..\..\..\..\..\tests\";
			txtCrownTestsExe.Text = @"..\..\..\..\..\build\tests\Debug\crown-tests.exe";
		}

		private void LoadTestsData()
		{
			var testsJsonFullfileName = System.IO.Path.Combine(txtTestFolder.Text, "tests.json");
			mContainer = JsonConvert.DeserializeObject<TestContainer>(System.IO.File.ReadAllText(testsJsonFullfileName));

			RefreshData();
		}

		private void RefreshData()
		{
			var treeStore = twTests.Model as Gtk.TreeStore;
			if (treeStore == null)
				treeStore = new Gtk.TreeStore(typeof(object));
			treeStore.Clear();
			foreach (var category in mContainer.Categories) {
				var iter = treeStore.AppendValues(category);
				foreach (var test in category.Tests) {
					treeStore.AppendValues(iter, test);
				}
			}

			twTests.Model = treeStore;
		}

		#endregion
	}
}

