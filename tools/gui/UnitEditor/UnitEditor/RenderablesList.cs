using System;
using System.Collections.Generic;
using Gtk;

namespace UnitEditor
{

	[System.ComponentModel.ToolboxItem (true)]
	public partial class RenderablesList : Gtk.EventBox
	{
		private UnitFile m_unit_file;

		private ListStore m_renderables_store;

		private List<string> m_renderables_names;
		private List<Renderable> m_renderables;

		public RenderablesList (string file_name)
		{
			m_unit_file = new UnitFile (file_name);

			m_renderables_names = new List<string> (m_unit_file.renderables_names());
			m_renderables = new List<Renderable> (m_unit_file.renderables());

			Console.Write (m_renderables_names[0]);
			m_renderables_store = new ListStore (typeof (string), typeof (string), typeof (string), typeof(string), typeof(bool));

			TreeView tree = new TreeView ();
			this.Add (tree);

			TreeViewColumn nameColumn = new TreeViewColumn ();
			nameColumn.Title = "Name";
			nameColumn.Alignment = 0.5f;

			TreeViewColumn nodeColumn = new TreeViewColumn ();
			nodeColumn.Title = "Node";
			nodeColumn.Alignment = 0.5f;

			TreeViewColumn typeColumn = new TreeViewColumn ();
			typeColumn.Title = "Type";
			typeColumn.Alignment = 0.5f;

			TreeViewColumn resourceColumn = new TreeViewColumn ();
			resourceColumn.Title = "Resource";
			resourceColumn.Alignment = 0.5f;

			TreeViewColumn visibleColumn = new TreeViewColumn ();
			visibleColumn.Title = "Visible";
			visibleColumn.Alignment = 0.5f;

			// Assign the model to the TreeView
			tree.Model = m_renderables_store;

			CellRendererText nameCell = new CellRendererText ();
			nameCell.Editable = true;
			nameCell.Edited += delegate (object o, EditedArgs e) { 
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				m_renderables_store.GetIter (out iter, path);
				int i = path.Indices[0];

				string r = e.NewText;
				m_renderables_names[i] = r;
				m_renderables_store.SetValue (iter, 0, r);
			};
			nameColumn.PackStart (nameCell, true);

			CellRendererText nodeCell = new CellRendererText ();
			nodeCell.Editable = true;
			nodeCell.Edited += delegate (object o, EditedArgs e) { 
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				m_renderables_store.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = m_renderables[i];
				r.node = e.NewText;
				m_renderables_store.SetValue (iter, 1, r.node);
			};
			nodeColumn.PackStart (nodeCell, true);

			CellRendererText typeCell = new CellRendererText ();
			typeCell.Editable = true;
			typeCell.Edited += delegate (object o, EditedArgs e) { 
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				m_renderables_store.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = m_renderables[i];
				r.type = e.NewText;
				m_renderables_store.SetValue (iter, 2, r.type);
			};
			typeColumn.PackStart (typeCell, true);

			CellRendererText resourceCell = new CellRendererText ();
			resourceCell.Editable = true;
			resourceCell.Edited += delegate (object o, EditedArgs e) { 
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				m_renderables_store.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = m_renderables[i];
				r.resource = e.NewText;
				m_renderables_store.SetValue (iter, 3, r.resource);
			};
			resourceColumn.PackStart (resourceCell, true);

			CellRendererToggle visibleCell = new CellRendererToggle ();
			visibleCell.Activatable = true;
			visibleCell.Toggled += delegate (object o, ToggledArgs e) {
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				m_renderables_store.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = m_renderables[i];
				bool old = (bool) m_renderables_store.GetValue(iter, 4);
				r.visible = !old;
				m_renderables_store.SetValue(iter, 4, !old);
			};
			visibleColumn.PackStart (visibleCell, true);

			// Add the columns to the TreeView
			nameColumn.AddAttribute (nameCell, "text", 0);
			nodeColumn.AddAttribute (nodeCell, "text", 1);
			typeColumn.AddAttribute (typeCell, "text", 2);
			resourceColumn.AddAttribute (resourceCell, "text", 3);
			visibleColumn.AddAttribute (visibleCell, "active", 4);

			tree.AppendColumn (nameColumn);
			tree.AppendColumn (nodeColumn);
			tree.AppendColumn (typeColumn);
			tree.AppendColumn (resourceColumn);
			tree.AppendColumn (visibleColumn);

			create_model ();

			// Show the window and everything on it
			ShowAll ();
		}

		void create_model()
		{
			for (int i = 0; i < m_renderables.Count; i++)
			{
				m_renderables_store.AppendValues (
						m_renderables_names[i],
						m_renderables[i].node,
						m_renderables[i].type,
						m_renderables[i].resource,
						m_renderables[i].visible);
			}
		}

		static void delete_event (object obj, DeleteEventArgs args)
		{
			Application.Quit();
		}

		static void exitbutton_event (object obj, ButtonPressEventArgs args)
		{
			Application.Quit();
		}
	}
}

