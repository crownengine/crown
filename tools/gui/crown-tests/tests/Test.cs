using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace crown_tests.tests
{
	[JsonObject(MemberSerialization.OptIn)]
	public class Test: ViewModelBase
	{
		[JsonProperty]
		public String Name { get; set; }

		[JsonProperty]
		public String Description { get; set; }

		int mLastResult;

		public int LastResult { 
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
