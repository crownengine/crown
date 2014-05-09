using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace crown_tests.tests
{
	public enum ETestResult {
		Unknown = 0,
		Failed = 1,
		Passed = 2
	}

	[JsonObject(MemberSerialization.OptIn)]
	public class Test: ViewModelBase
	{
		[JsonProperty]
		public String Name { get; set; }

		[JsonProperty]
		public String Description { get; set; }

		ETestResult mLastResult;
		public ETestResult LastResult { 
			get { return mLastResult; }
			set {
				if (mLastResult != value) {
					mLastResult = value;
					Notify("LastResult");
				}
			}
		}

		public String GetFunctionName()
		{
			return "test_" + Name.ToLower().Replace(' ', '_');
		}
	}
}
