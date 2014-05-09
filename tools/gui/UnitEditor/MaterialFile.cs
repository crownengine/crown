using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace UnitEditor
{
	public class MaterialFile
	{
		private JObject m_root;
		private List<string> m_materials;

		public MaterialFile (string mat_file_name)
		{
			string json_string = string.Empty;

			using (StreamReader streamReader = new StreamReader(mat_file_name))
			{            
				json_string = streamReader.ReadToEnd();
				m_root = JObject.Parse (json_string);
			}

			deserialize ();
		}

		public string[] materials()
		{
			return m_materials.ToArray ();
		}

		public void deserialize()
		{
			JToken renderables_token = m_root ["texture_layers"];
			m_materials = JsonConvert.DeserializeObject<List<string>>(renderables_token.ToString());
		}

		public void serialize()
		{
			string json_string = "{\n";
			json_string += "\"texture_layers\": [\n";
			string last = m_materials.Last();
			foreach (string m in m_materials)
			{
				json_string += "\"" + m + "\"\n";
				if (m != last) json_string += ",";
			}
			json_string += "]\n";
			json_string += "}";

			Console.Write (json_string);
		}
	}
}

