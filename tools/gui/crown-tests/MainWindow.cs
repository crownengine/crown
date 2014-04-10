using System;
using Gtk;
using crown_tests.tests;
using Newtonsoft.Json;

namespace crown_tests
{
	public partial class MainWindow : Gtk.Window
	{
		private TestContainer mContainer;

		public MainWindow () : 
			base (Gtk.WindowType.Toplevel)
		{
			this.Build ();

			twTests.AppendColumn("Name", new Gtk.CellRendererText(), "text", 0);
			twTests.AppendColumn("State", new Gtk.CellRendererText(), "text", 1);

			LoadConfigData ();
			LoadTestsData ();
		}

		protected void OnDeleteEvent (object sender, DeleteEventArgs a)
		{
			Application.Quit ();
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
			RefreshData();
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
			var treeStore = twTests.Model as Gtk.TreeStore; // new Gtk.TreeStore(typeof(string), typeof(string));
			if (treeStore == null)
				treeStore = new Gtk.TreeStore(typeof(string), typeof(string));
			treeStore.Clear();
			foreach (var category in mContainer.Categories)
			{
				var iter = treeStore.AppendValues(category.Name);
				foreach (var test in category.Tests)
				{
					treeStore.AppendValues(iter, test.Name, test.LastResult == 0 ? "Passed" : "Failed");
				}
			}

			twTests.Model = treeStore;
		}
		#endregion
	}
}

