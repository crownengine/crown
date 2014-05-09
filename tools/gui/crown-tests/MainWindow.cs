using System;
using Gtk;
using crown_tests.tests;
using Newtonsoft.Json;
using crown_tests.GtkExt;
using crown_tests.ViewModels;

namespace crown_tests
{
	public partial class MainWindow : Gtk.Window
	{
		private TestContainer mContainer;

		public MainWindow() : 
			base(Gtk.WindowType.Toplevel)
		{
			this.Build();

			//Set the ViewModels for each widget
			var crownTestsViewModel = new CrownTestsViewModel();
			BindingEngine.SetViewModel(this, crownTestsViewModel);
			//txtTestFolder and txtCrownTestsExe automatically inherit the view model of the MainWindow because it has not been not specified

			//Create and Apply templates for each widget
			Templating.ApplyTemplate(twTests,
				new TreeViewTemplate()
					.AddColumn("Name", new Gtk.CellRendererText())
					.AddColumn("State", new Gtk.CellRendererText())
				  .AddRowTemplate(TreeViewRowTemplate.Create(typeof(TestCategory))
																						 .SetBinding("Name", "Name"))
					.AddRowTemplate(TreeViewRowTemplate.Create(typeof(Test))
																						 .SetBinding("Name", "Name")
																						 .SetBinding("State", "LastResult")));

			Templating.ApplyTemplate(txtTestFolder,
				new EntryTemplate().SetTextBinding("TestFolder"));

			Templating.ApplyTemplate(txtCrownTestsExe,
				new EntryTemplate().SetTextBinding("CrownTestsExe"));

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

		private void LoadTestsData()
		{
			var testsJsonFullfileName = System.IO.Path.Combine(txtTestFolder.Text, "tests.json");
			if (!System.IO.File.Exists(testsJsonFullfileName)) {
				Console.WriteLine("Could not find test data: " + testsJsonFullfileName);
				return;
			}
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

