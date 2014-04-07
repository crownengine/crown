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
    
    // Create a column for the artist name
    Gtk.TreeViewColumn artistColumn = new Gtk.TreeViewColumn();
    artistColumn.Title = "Name";

    // Create a column for the song title
    Gtk.TreeViewColumn songColumn = new Gtk.TreeViewColumn();
    songColumn.Title = "Description";

    // Add the columns to the TreeView
    Gtk.TreeView treeview1 = new Gtk.TreeView();
    Add(treeview1);
    treeview1.AppendColumn(artistColumn);
    treeview1.AppendColumn(songColumn);

    treeview1.Model = LoadData();

    Gtk.CellRendererText testNameCell = new Gtk.CellRendererText();
    artistColumn.PackStart(testNameCell, true);

    Gtk.CellRendererText testDescriptionCell = new Gtk.CellRendererText();
    songColumn.PackStart(testDescriptionCell, true);

    // Tell the Cell Renderers which items in the model to display
    artistColumn.AddAttribute(testNameCell, "text", 0);
    songColumn.AddAttribute(testDescriptionCell, "text", 1);

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