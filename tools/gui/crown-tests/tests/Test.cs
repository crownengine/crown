using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace crown_tests.tests
{
  [JsonObject(MemberSerialization.OptIn)]
  public class Test
  {
    [JsonProperty]
		public String Name { get; set; }
    [JsonProperty]
		public String Description { get; set; }

		public int LastResult { get; set; }

    public String GetFunctionName()
    {
      return "test_" + Name.ToLower().Replace(' ', '_');
    }
  }
}
