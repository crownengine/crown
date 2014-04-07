using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace crown_tests.tests
{
  [JsonObject(MemberSerialization.OptIn)]
  public class TestContainer
  {
    [JsonProperty]
    public List<TestCategory> Categories;

    public TestContainer()
    {
      Categories = new List<TestCategory>();
    }
  }
}
