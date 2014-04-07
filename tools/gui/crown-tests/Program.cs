using System;
using Gtk;
using Glade;

namespace crown_tests
{
  public class Program
  {
    public static void Main(string[] args)
    {
      Application.Init();
      MainWindow win = new MainWindow();
      win.ShowAll();
      Application.Run();
    }
  }
}