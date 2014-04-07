using System;
using Gtk;

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

    Gtk.TreeStore testsStore = new Gtk.TreeStore(typeof(string), typeof(string));

    Gtk.TreeIter iter = testsStore.AppendValues("FileSystem");
    testsStore.AppendValues(iter, "Test 1", "Test file existence");
    testsStore.AppendValues(iter, "Test 2", "Create empty file");

    iter = testsStore.AppendValues("Json");
    testsStore.AppendValues(iter, "Test 3", "Loading a simple file");
    testsStore.AppendValues(iter, "Test 4", "Saving a simple file");

    treeview1.Model = testsStore;

    Gtk.CellRendererText testNameCell = new Gtk.CellRendererText();
    artistColumn.PackStart(testNameCell, true);

    Gtk.CellRendererText testDescriptionCell = new Gtk.CellRendererText();
    songColumn.PackStart(testDescriptionCell, true);

    // Tell the Cell Renderers which items in the model to display
    artistColumn.AddAttribute(testNameCell, "text", 0);
    songColumn.AddAttribute(testDescriptionCell, "text", 1);
  }

  protected override bool OnDeleteEvent(Gdk.Event evnt)
  {
    Application.Quit();
    return base.OnDeleteEvent(evnt);
  }
}