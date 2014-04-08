using System;
using Gtk;
using Newtonsoft.Json;
using crown_tests.tests;

public partial class MainWindow : Gtk.Window
{
  public MainWindow(): base(Gtk.WindowType.Toplevel)
  {
    //Glade.XML gxml = new Glade.XML("main1.glade", "MyWindow", null);
    //gxml.Autoconnect(this);
    Title = "Test Browser";
    SetSizeRequest(500, 300);
    
    Gtk.TreeView treeview1 = new Gtk.TreeView();
    treeview1.AppendColumn("Name", new Gtk.CellRendererText(), "text", 0);
    treeview1.AppendColumn("Description", new Gtk.CellRendererText(), "text", 1);
    treeview1.Model = LoadData();
    Add(treeview1);

    LoadData();
  }

  private Gtk.TreeStore LoadData()
  {
    var container = JsonConvert.DeserializeObject<TestContainer>(System.IO.File.ReadAllText("tests.json"));

    Gtk.TreeStore testsStore = new Gtk.TreeStore(typeof(string), typeof(string));
    foreach (var category in container.Categories)
    {
      var iter = testsStore.AppendValues(category.Name);
      foreach (var test in category.Tests)
      {
        testsStore.AppendValues(iter, test.Name, test.Description);
      }
    }
    return testsStore;
  }

  protected override bool OnDeleteEvent(Gdk.Event evnt)
  {
    Application.Quit();
    return base.OnDeleteEvent(evnt);
  }
}