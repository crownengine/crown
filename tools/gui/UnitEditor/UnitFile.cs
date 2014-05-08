using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using System.Diagnostics;

namespace UnitEditor
{
	public class UnitFile
	{
		private string m_filename;

		private List<string> m_renderables_names;
		private List<Renderable> m_renderables;

		private List<string> m_nodes_names;
		private List<Node> m_nodes;

		//------------------------------------------------------------------------------
		public UnitFile (string file_name)
		{
			Debug.Assert(File.Exists (file_name));
			m_filename = file_name;

			// Parse json file
			string json_string = string.Empty;
			JObject m_root;
			using (StreamReader streamReader = new StreamReader(file_name))
			{            
				json_string = streamReader.ReadToEnd();
				m_root = JObject.Parse (json_string);
			}

			// Deserialize renderables
			JToken renderables_token = m_root ["renderables"];
			Dictionary<string, Renderable> renderables = JsonConvert.DeserializeObject<Dictionary<string, Renderable>>(renderables_token.ToString());
			m_renderables_names = new List<string> (renderables.Keys.ToArray ());
			m_renderables = new List<Renderable> (renderables.Values.ToArray ());

			// Deserialize Nodes
			JToken nodes_token = m_root ["nodes"];
			Dictionary<string, Node> nodes = JsonConvert.DeserializeObject<Dictionary<string, Node>> (nodes_token.ToString ());
			m_nodes_names = new List<string> (nodes.Keys.ToArray ());
			m_nodes = new List<Node> (nodes.Values.ToArray ());
		}

		//------------------------------------------------------------------------------
		public string[] renderables_names()
		{
			return m_renderables_names.ToArray ();
		}

		//------------------------------------------------------------------------------
		public Renderable[] renderables()
		{
			return m_renderables.ToArray ();
		}

		//------------------------------------------------------------------------------
		public string[] nodes_name()
		{
			return m_nodes_names.ToArray ();
		}

		//------------------------------------------------------------------------------
		public Node[] nodes()
		{
			return m_nodes.ToArray ();
		}


		//------------------------------------------------------------------------------
		public void save()
		{
			using (StreamWriter writer = new StreamWriter (m_filename))
			{
				string json_string = "{";
				json_string += "\"renderables\": {";

				for (int i = 0; i < m_renderables.Count; i++)
				{
					string renderables_token = string.Format ("\t\"{0}\" : {1}", m_renderables_names[i], JsonConvert.SerializeObject(m_renderables[i]));
					if (i < m_renderables.Count - 1)
						json_string += ",";

					json_string += renderables_token;
				}
				json_string += "},\n";

				json_string += "\"nodes\": {";
				for (int i = 0; i < m_nodes.Count; i++)
				{
					string nodes_token = string.Format ("\t\"{0}\" : {1}", m_nodes_names[i], JsonConvert.SerializeObject(m_nodes[i]));
					json_string += nodes_token;
				}
				json_string += "}";
				json_string += "}";

				writer.Write (json_string);
			}
		}
	}
}

