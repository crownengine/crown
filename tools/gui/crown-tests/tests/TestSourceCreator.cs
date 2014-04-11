using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace crown_tests.tests
{
	public class TestSourceCreator
	{
		private TestContainer mContainer;
		private String mDestFolder;

		public TestSourceCreator(TestContainer container, String destfolder)
		{
			mContainer = container;
			mDestFolder = destfolder;
		}

		public void Create()
		{
			if (!Directory.Exists(mDestFolder))
				Directory.CreateDirectory(mDestFolder);

			CreateMainFile();

			foreach (var category in mContainer.Categories) {
				String folder = Path.Combine(mDestFolder, category.Name);
				if (!Directory.Exists(folder))
					Directory.CreateDirectory(folder);
				foreach (var test in category.Tests) {
					MaybeCreateTestFile(folder, test);
				}
			}
		}

		private void CreateMainFile()
		{
			var mainFullFileName = Path.Combine(mDestFolder, "main.cpp");

			String content = "";

			foreach (var category in mContainer.Categories) {
				content += "//Category '" + category.Name + "'" + Environment.NewLine;
				foreach (var test in category.Tests) {
					content += "#include \"" + test.Name + ".h\"" + Environment.NewLine;
				}
			}

			content += Environment.NewLine;
			content += "#include <string.h>" + Environment.NewLine;
			content += "int main(int argc, char** argv)" + Environment.NewLine;
			content += "{" + Environment.NewLine;
			content += "  if (argc < 2) return -1;" + Environment.NewLine;
			foreach (var category in mContainer.Categories) {
				foreach (var test in category.Tests) {
					content += "  if (strcmp(argv[1], \"/test:" + test.Name + "\") == 0)" + Environment.NewLine;
					content += "  return " + test.GetFunctionName() + "();" + Environment.NewLine;
				}
			}
			content += "  return -2;" + Environment.NewLine;
			content += "}" + Environment.NewLine;
			File.WriteAllText(mainFullFileName, content);
		}

		private void MaybeCreateTestFile(String fullFolderName, Test test)
		{
			var headerFullFileName = Path.Combine(fullFolderName, test.Name + ".h");
			var sourceFullFileName = Path.Combine(fullFolderName, test.Name + ".cpp");

			if (File.Exists(headerFullFileName) || File.Exists(sourceFullFileName))
				return;

			//Header
			String headerContent = "";
			headerContent += "#pragma once" + Environment.NewLine;
			headerContent += Environment.NewLine;
			headerContent += "int " + test.GetFunctionName() + "();" + Environment.NewLine;
			File.WriteAllText(headerFullFileName, headerContent);

			//Source
			String sourceContent = "";
			sourceContent += "#include \"" + Path.GetFileName(headerFullFileName) + "\"" + Environment.NewLine;
			sourceContent += "#include <time.h>" + Environment.NewLine;
			sourceContent += "#include <stdlib.h>" + Environment.NewLine;
			sourceContent += Environment.NewLine;
			sourceContent += "int " + test.GetFunctionName() + "()" + Environment.NewLine;
			sourceContent += "{" + Environment.NewLine;
			sourceContent += "  srand(time(NULL));" + Environment.NewLine;
			sourceContent += "  return rand() % 2;" + Environment.NewLine;
			sourceContent += "}" + Environment.NewLine;
			File.WriteAllText(sourceFullFileName, sourceContent);
		}
	}
}

