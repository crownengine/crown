using System;
using Gtk;
using Newtonsoft.Json;
using crown_tests.tests;

public partial class MainWindow : Gtk.Window
{
  Gtk.TreeStore mTreeStore;
  Gtk.TreeView mTreeView;
  Gtk.Entry mEntryTestFolder;
  Gtk.Entry mEntryCrownTestsExe;

  private TestContainer mContainer;

  public MainWindow(): base(Gtk.WindowType.Toplevel)
  {
    //Glade.XML gxml = new Glade.XML("main1.glade", "MyWindow", null);
    //gxml.Autoconnect(this);
    Title = "Test Browser";
    SetSizeRequest(500, 300);

    var table = new Gtk.Table(1, 2, false);

    var configTable = new Gtk.Table(1, 1, false);
    mEntryTestFolder = new Gtk.Entry();
    configTable.Attach(mEntryTestFolder, 1, 2, 0, 1);
    mEntryCrownTestsExe = new Gtk.Entry();
    configTable.Attach(mEntryCrownTestsExe, 1, 2, 1, 2);
    var label1 = new Gtk.Label("Tests folder");
    configTable.Attach(label1, 0, 1, 0, 1);
    var label2 = new Gtk.Label("crown-tests executable");
    configTable.Attach(label2, 0, 1, 1, 2);
    table.Attach(configTable, 0, 2, 0, 1);
    
    mTreeView = new Gtk.TreeView();
    mTreeView.AppendColumn("Name", new Gtk.CellRendererText(), "text", 0);
    mTreeView.AppendColumn("State", new Gtk.CellRendererText(), "text", 1);
    //treeview1.AppendColumn("Description", new Gtk.CellRendererText(), "text", 1);
    
    table.Attach(mTreeView, 0, 1, 1, 3);

    Gtk.Button btnCreate = new Gtk.Button();
    btnCreate.Label = "Create";
    btnCreate.Clicked += btnCreate_Click;
    table.Attach(btnCreate, 1, 2, 1, 2);

    Gtk.Button btnExecute = new Gtk.Button();
    btnExecute.Label = "Execute";
    btnExecute.Clicked += btnExecute_Click;
    table.Attach(btnExecute, 1, 2, 2, 3);

    Add(table);

    LoadConfigData();
    LoadTestsData();
  }

  private void btnCreate_Click(object o, EventArgs args)
  {
    var creator = new TestSourceCreator(mContainer, mEntryTestFolder.Text);
    creator.Create();
  }

  private void btnExecute_Click(object o, EventArgs args)
  {
    var executor = new TestExecutor(mContainer, mEntryCrownTestsExe.Text);
    executor.ExecuteAll();
    RefreshData();
  }

  private void LoadConfigData()
  {
    mEntryTestFolder.Text = @"..\..\..\..\..\tests\";
    mEntryCrownTestsExe.Text = @"..\..\..\..\..\build\tests\Debug\crown-tests.exe";
  }

  private void LoadTestsData()
  {
    var testsJsonFullfileName = System.IO.Path.Combine(mEntryTestFolder.Text, "tests.json");
    mContainer = JsonConvert.DeserializeObject<TestContainer>(System.IO.File.ReadAllText(testsJsonFullfileName));

    RefreshData();
  }

  private void RefreshData()
  {
    mTreeStore = new Gtk.TreeStore(typeof(string), typeof(string));
    //mTreeStore.Clear();
    foreach (var category in mContainer.Categories)
    {
      var iter = mTreeStore.AppendValues(category.Name);
      foreach (var test in category.Tests)
      {
        mTreeStore.AppendValues(iter, test.Name, test.LastResult == 0 ? "Passed" : "Failed");
      }
    }

    mTreeView.Model = mTreeStore;
  }

  protected override bool OnDeleteEvent(Gdk.Event evnt)
  {
    Application.Quit();
    return base.OnDeleteEvent(evnt);
  }
}