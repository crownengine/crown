using System;
using crown_tests.GtkExt;

namespace crown_tests.ViewModels
{
	public class CrownTestsViewModel: ViewModelBase
	{
		public CrownTestsViewModel()
		{
			TestFolder = @"..\..\..\..\..\tests\";
			CrownTestsExe = @"..\..\..\..\..\build\tests\Debug\crown-tests.exe";
		}

		private String mTestFolder;
		public String TestFolder {
			get { return mTestFolder; }
			set { SetAndNotify(ref mTestFolder, value, "TestFolder"); }
		}

		private String mCrownTestsExe;
		public String CrownTestsExe {
			get { return mCrownTestsExe; }
			set { SetAndNotify(ref mCrownTestsExe, value, "CrownTestsExe"); }
		}
	}
}

