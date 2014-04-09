using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace crown_tests.tests
{
  public class TestExecutor
  {
    private TestContainer mContainer;
    private String mExeFullFileName;

    public TestExecutor(TestContainer container, String exeFullFileName)
    {
      mContainer = container;
      mExeFullFileName = exeFullFileName;
    }

    public void ExecuteAll()
    {
      foreach (var category in mContainer.Categories)
      {
        foreach (var test in category.Tests)
        {
          var p = new Process();
          p.StartInfo.FileName = mExeFullFileName;
          p.StartInfo.Arguments = string.Format("/test:\"{0}\"", test.Name);
          p.Start();
          p.WaitForExit();
          test.LastResult = p.ExitCode;
          System.Threading.Thread.Sleep(1500);
        }
      }
    }
  }
}
