using System;
using System.Collections.Generic;
using Gtk;

namespace UnitEditor
{

	[System.ComponentModel.ToolboxItem (true)]
	public partial class RenderablesList : Gtk.EventBox
	{
		private ListStore renderablesStore;
		private List<Renderable> renderables;

		public RenderablesList (List<Renderable> renderables)
		{
			this.renderables = new List<Renderable> (renderables);
			this.renderablesStore = new ListStore (typeof (string), typeof (string), typeof(string), typeof(bool));

			// Create our TreeView
			TreeView tree = new TreeView ();

			// Add our tree to the window
			this.Add (tree);

			// Create a column for the artist name
			TreeViewColumn nodeColumn = new TreeViewColumn ();
			nodeColumn.Title = "Node";
			nodeColumn.Alignment = 0.5f;

			// Create a column for the song title
			TreeViewColumn typeColumn = new TreeViewColumn ();
			typeColumn.Title = "Type";
			typeColumn.Alignment = 0.5f;

			// Create a column for the artist name
			TreeViewColumn resourceColumn = new TreeViewColumn ();
			resourceColumn.Title = "Resource";
			resourceColumn.Alignment = 0.5f;

			// Create a column for the song title
			TreeViewColumn visibleColumn = new TreeViewColumn ();
			visibleColumn.Title = "Visible";
			visibleColumn.Alignment = 0.5f;

			// Assign the model to the TreeView
			tree.Model = renderablesStore;

			// Create the text cell that will display the node and add cell to the column
			CellRendererText nodeCell = new CellRendererText ();
			nodeCell.Editable = true;
			nodeCell.Edited += delegate (object o, EditedArgs e) { 
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				renderablesStore.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = renderables[i];
				r.node = e.NewText;
				renderablesStore.SetValue (iter, 0, r.node);
				Console.WriteLine("node:{0}, type:{1}, resource:{2}, visible:{3}", r.node, r.type, r.resource, r.visible);
			};
			nodeColumn.PackStart (nodeCell, true);

			CellRendererText typeCell = new CellRendererText ();
			typeCell.Editable = true;
			typeCell.Edited += delegate (object o, EditedArgs e) { 
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				renderablesStore.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = renderables[i];
				r.type = e.NewText;
				renderablesStore.SetValue (iter, 1, r.type);
				Console.WriteLine("node:{0}, type:{1}, resource:{2}, visible:{3}", r.node, r.type, r.resource, r.visible);
			};
			typeColumn.PackStart (typeCell, true);

			CellRendererText resourceCell = new CellRendererText ();
			resourceCell.Editable = true;
			resourceCell.Edited += delegate (object o, EditedArgs e) { 
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				renderablesStore.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = renderables[i];
				r.resource = e.NewText;
				renderablesStore.SetValue (iter, 2, r.resource);
				Console.WriteLine("node:{0}, type:{1}, resource:{2}, visible:{3}", r.node, r.type, r.resource, r.visible);
			};
			resourceColumn.PackStart (resourceCell, true);

			CellRendererToggle visibleCell = new CellRendererToggle ();
			visibleCell.Activatable = true;
			visibleCell.Toggled += delegate (object o, ToggledArgs e) {
				TreePath path = new TreePath (e.Path);
				TreeIter iter;
				renderablesStore.GetIter (out iter, path);
				int i = path.Indices[0];

				Renderable r = renderables[i];
				bool old = (bool) renderablesStore.GetValue(iter, 3);
				r.visible = !old;
				renderablesStore.SetValue(iter, 3, !old);
				Console.WriteLine("node:{0}, type:{1}, resource:{2}, visible:{3}", r.node, r.type, r.resource, r.visible);					
			};
			visibleColumn.PackStart (visibleCell, true);

			// Add the columns to the TreeView
			nodeColumn.AddAttribute (nodeCell, "text", 0);
			typeColumn.AddAttribute (typeCell, "text", 1);
			resourceColumn.AddAttribute (resourceCell, "text", 2);
			visibleColumn.AddAttribute (visibleCell, "active", 3);
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
			foreach (Renderable renderable in this.renderables.ToArray())
			{
				add_renderable(renderable);
			}
		}

		public void add_renderable (Renderable r)
		{
			this.renderables.Add (r);
			this.renderablesStore.AppendValues (r.node, r.type, r.resource, r.visible);
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

