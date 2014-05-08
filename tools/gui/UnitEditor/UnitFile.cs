using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Text;
using System.Collections.Generic;
using System.Linq;

namespace UnitEditor
{
	public class UnitFile
	{
		private JObject m_root;
		private Dictionary<string, Renderable> m_renderables;
		private Dictionary<string, Node> m_nodes;

		public UnitFile (string file_name)
		{
			string json_string = string.Empty;

			using (StreamReader streamReader = new StreamReader(file_name))
			{            
				json_string = streamReader.ReadToEnd();
				m_root = JObject.Parse (json_string);
			}

			deserialize ();
		}

		public string[] renderables_names()
		{
			return m_renderables.Keys.ToArray ();
		}

		public Renderable[] renderables()
		{
			return m_renderables.Values.ToArray ();
		}

		public void deserialize()
		{
			// Deserialize renderables
			JToken renderables_token = m_root ["renderables"];
			m_renderables = JsonConvert.DeserializeObject<Dictionary<string, Renderable>>(renderables_token.ToString());

			// Deserialize Nodes
			JToken nodes_token = m_root ["nodes"];
			m_nodes = JsonConvert.DeserializeObject<Dictionary<string, Node>> (nodes_token.ToString ());
		}

		public void serialize()
		{
			string json_string = "{";
			json_string += "\"renderables\": {";

			string last = m_renderables.Keys.Last ();
			foreach (var r in m_renderables)
			{
				var k = r.Key;
				var v = r.Value;
				string renderables_token = string.Format ("\t\"{0}\" : {1}", k, JsonConvert.SerializeObject(v));
				if (k != last)
					json_string += ",";
				json_string += renderables_token;
			}
			json_string += "},\n";

			json_string += "\"nodes\": {";
			foreach (var n in m_nodes)
			{
				var k = n.Key;
				var v = n.Value;
				string nodes_token = string.Format ("\t\"{0}\" : {1}", k, JsonConvert.SerializeObject(v));
				json_string += nodes_token;
			}
			json_string += "}";
			json_string += "}";

			Console.Write (json_string);
		}
	}
}

