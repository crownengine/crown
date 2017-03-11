/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using Cairo;
using Gee;
using GLib;
using Gtk;

namespace Crown
{
	public class Gui
	{
		public static void Spline(Cairo.Context cr, Vector2 from, Vector2 to, Vector4 col, double line_width = 2)
		{
			double from_x = from.x;
			double from_y = from.y;
			double to_x = to.x;
			double to_y = to.y;
			double offset = ((from_x > to_x) ? ((from_x-to_x)/2) : ((to_x-from_x)/2)) + Math.fabs(from_y-to_y)/6;
			double flip = from_x > to_x ? -1.0 : 1.0;
			double fromm = from_x + offset * flip;
			double tomm = to_x + offset * -flip;

			cr.move_to(from_x, from_y);
			cr.curve_to(fromm, from_y, tomm, to_y, to_x, to_y);

			cr.set_line_width(line_width);
			cr.set_source_rgba(col.x, col.y, col.z, col.w);
			cr.stroke();
		}

		public static void SplineSelf(Cairo.Context cr, Vector2 from, Vector2 from1, Vector2 to, Vector2 to1, Vector4 col, double line_width = 2)
		{
			// cr.MoveTo(from.x, from.y);
			// cr.CurveTo(from.x + 40, from.y, from1.x + 40, from1.y - 10, from1.x, from1.y);

			// cr.MoveTo(from1.x, from1.y);
			// cr.CurveTo(from1.x - 40, from1.y + 10, to1.x + 40, to1.y + 10, to1.x, to1.y);

			// cr.MoveTo(to1.x, to1.y);
			// cr.CurveTo(to1.x - 40, to1.y - 10, to.x - 40, to.y, to.x, to.y);

			// cr.LineWidth = 2;
			// cr.SetSourceColor(col);
			// cr.Stroke();
		}

		// Align (-1, 0, 1) -> (left, center, right)
		public static void Text(Cairo.Context cr, string text, double size, Vector2 min, Vector2 max, Vector4 col, int align = 0)
		{
			// cr.SelectFontFace("Sans", FontSlant.Normal, FontWeight.Bold);
			// cr.SetFontSize (size);
			// TextExtents extents = cr.TextExtents (text);

			// double text_x = align == -1 ? min.x : align == 0 ? min.x + (max.x - extents.Width) / 2 : max.x - extents.Width;
			// double text_y = min.y + (max.y - extents.Height) / 2 - extents.yBearing - extents.yAdvance;

			// cr.MoveTo (text_x + 1, text_y + 1);
			// cr.SetSourceRGB(col.R * 1/5, col.G * 1/5, col.B * 1/5);
			// cr.ShowText (text);

			// cr.MoveTo (text_x, text_y);
			// cr.SetSourceRGB(col.R, col.G, col.B);
			// cr.ShowText (text);
			// cr.Fill();
		}

		public static void Node(Cairo.Context cr, double x, double y, double w, double h, bool outline, string label, Vector4 color)
		{
			double radius = 2;

			if ((radius > h / 2) || (radius > h / 2))
				radius = Math.fmin((double) h / 2, (double) w / 2);

			cr.save();
			cr.translate(x, y);

			if (outline)
			{
				cr.arc(radius - 1, radius - 1, radius, Math.PI, -Math.PI / 2);
				cr.arc(w + 1 - radius, radius - 1, radius, -Math.PI / 2, 0);
				cr.arc(w + 1 - radius, h + 1 - radius, radius, 0, Math.PI / 2);
				cr.arc(radius - 1, h + 1 - radius, radius, Math.PI / 2, Math.PI);
				cr.close_path();

				cr.set_line_width(2);
				cr.set_source_rgb(0.9, 0.1, 0);
				cr.stroke();
			}

			// Draw background
			cr.arc(radius, radius, radius, Math.PI, -Math.PI / 2);
			cr.arc(w - radius, radius, radius, -Math.PI / 2, 0);
			cr.arc(w - radius, h - radius, radius, 0, Math.PI / 2);
			cr.arc(radius, h - radius, radius, Math.PI / 2, Math.PI);
			cr.close_path();
			cr.clip_preserve();

			Vector4 col = color;

			// Gradient
			Cairo.Pattern grad = new Cairo.Pattern.linear(0, 0, 0, h);
			col.w = 0.75;
			grad.add_color_stop_rgba(0.0, col.x, col.y, col.z, col.w);
			double height = NodeQuad.LabelHeight / h;
			double height2 = (double)2 / h;
			grad.add_color_stop_rgba(height, col.x, col.y, col.z, col.w);
			grad.add_color_stop_rgba(height, 0.2, 0.2, 0.2, 1.0);
			grad.add_color_stop_rgba(height + height2, 0.35, 0.35, 0.35, 0.9);
			grad.add_color_stop_rgba(1.0, 0.35, 0.35, 0.35, 0.9);
			cr.set_source(grad);
			// Gradient
			cr.fill_preserve();

			Cairo.Pattern stk = new Cairo.Pattern.linear(0, 0, 0, h);
			stk.add_color_stop_rgba(0.0, 0.9, 0.9, 0.9, 0.2);
			stk.add_color_stop_rgba(1.0, 0.6, 0.6, 0.6, 0.1);
			cr.set_source(stk);
			cr.set_line_width(3);
			cr.stroke();

			// // Draw title
			// Gui.Text(cr, label, 13, Vector2(0, 0), Vector2(w, NodeQuad.LabelHeight), Vector4(0.9, 0.9, 0.9));

			// cr.Translate(0, NodeQuad.LabelHeight);

			// // // Draw rows
			// // for (int i = 0; i < _rows.Count; i++)
			// // {
			// // 	NodeRow row = _rows[i];
			// // 	Vector2 orig = TypeUtils.IsInput(row.Value) ? Vector2(18, 0) : Vector2(18, 0);
			// // 	Vector2 exten = TypeUtils.IsInput(row.Value) ? Vector2(150, 20) : Vector2(132, 20);
			// // 	int align = TypeUtils.IsInput(row.Value) ? -1 : 1;
			// // 	double arcorig = TypeUtils.IsInput(row.Value) ? 8 : 150 - 8;

			// // 	string value = (TypeUtils.IsEvent(row.Value) || !TypeUtils.IsInput(row.Value)) ? "" :
			// // 		row.Value.ToString() == "" ? "" : " = " + row.Value.ToString();

			// // 	Gui.Text(cr, row.Label + value, 12, orig, exten, TypeUtils.HexToColor(0xe6e6e6), align);
			// // 	if (row.Linkable)
			// // 	{
			// // 		col = TypeUtils.TypeColor[row.Value.GetType()];
			// // 		cr.SetSourceRGB(col.R, col.G, col.B);
			// // 		cr.Arc(arcorig, 20 * 0.5, 4, 0, Math.PI * 2);
			// // 	}
			// // 	cr.Fill();
			// // 	cr.Translate(0, NodeQuad.ItemHeight);
			// // }

			cr.restore();
		}
	}

	public struct MouseState
	{
		public double StartX { get; set; }
		public double StartY { get; set; }
		public double X { get; set; }
		public double Y { get; set; }

		public void UpdateStartPosition(double x, double y)
		{
			StartX = x;
			StartY = y;
		}

		public void UpdatePosition(double x, double y)
		{
			X = x;
			Y = y;
		}
	}

	public struct NodeInstance
	{
		public int i;

		public NodeInstance(int i)
		{
			this.i = i;
		}

		public bool IsValid()
		{
			return i != int.MAX;
		}
	}

	public struct NodeQuad
	{
		public Guid Node;
		public double x;
		public double y;
		public double Width;
		public int NumRows;

		public const int LabelHeight = 20;
		public const int ItemHeight = 20;

		public double Height()
		{
			return LabelHeight + NumRows * ItemHeight;
		}

		public void SetPosition(double x, double y)
		{
			this.x = x;
			this.y = y;
		}

		// Returns wheter x and y are inside the node area.
		public bool IsInside(double x, double y)
		{
			return x > this.x
				&& y > this.y
				&& x < this.x + Width
				&& y < this.y + Height();
		}

		public bool IsInsideResizeHandle(double x, double y)
		{
			return x > this.x + Width
				&& y > this.y
				&& x < this.x + Width + 4
				&& y < this.y + Height();
		}

		public bool IsInsideLabel(double x, double y)
		{
			return y > this.y + LabelHeight
				&& y < this.y + LabelHeight
				&& x > this.x
				&& x < this.x + Width;
		}

		public int IsInsideRow(double x, double y)
		{
			// if (this.y > y + LabelHeight && this.y < y + LabelHeight + Height())
			// {
			// 	int row = (int)(((y - (y + LabelHeight)) / (Height() - LabelHeight)) * NumRows);
			// 	return Math.fmin(row, NumRows - 1);
			// }
			return -1;
		}

		public int IsInsideConnector(double x, double y)
		{
			int row = IsInsideRow(x, y);
			if (row == -1)
				return -1;

			Vector2 pos = RowConnector(row);
			return (x-pos.x)*(x-pos.x) + (y-pos.y)*(y-pos.y) < 12*12 ? row : -1;
		}

		public Vector2 RowConnector(int i)
		{
			double yy = y + (LabelHeight + (double)i * ItemHeight);
			yy += ItemHeight * 0.5;
			bool left = false; //node.IsLeftConnector(i);
			return Vector2(x + (left ? 10 : Width - 10), yy);
		}

		public void UpdateSize(int num_rows)
		{
			Width = 150;
			NumRows = num_rows;
		}
	}

	public struct LinkInstance
	{
		public int i;

		public LinkInstance(int i)
		{
			this.i = i;
		}

		public bool IsValid()
		{
			return i != int.MAX;
		}
	}

	public struct LinkRenderer
	{
		public Guid Link;
		public Guid FromNode;
		public int FromRow;
		public Guid ToNode;
		public int ToRow;
	}

	public class GraphView : Gtk.EventBox
	{
		// Data
		private Guid _selected_node;
		private Guid _enter_node;
		private int _selected_row;

		private HashSet<Guid?> _selection;

		private HashMap<Guid?, int> _iter_to_node;
		private LinkedList<int> _zorder;
		private NodeQuad[] _nodes;

		private HashMap<Guid?, int> _iter_to_link;
		private LinkRenderer[] _links;

		private MouseState _mouse;
		private double _node_offt_x;
		private double _node_offt_y;
		private double _scroll_x_old;
		private double _scroll_y_old;
		private bool _linking;
		private bool _dragging;
		private bool _scrolling;

		private double _grid_scale_min;
		private double _grid_scale_max;
		private double _grid_scale_step;
		private double _grid_size;
		private double _grid_scale;
		private double _grid_scroll_x;
		private double _grid_scroll_y;

		private GraphStore _model;

		private Gtk.Menu _menu;
		private Gtk.Menu _node_menu;
		private UIManager _ui_manager;

		private Gdk.Cursor _cursor_fleur;
		private Gdk.Cursor _cursor_move;
		private Gdk.Cursor _cursor_resize;

		const Gtk.ActionEntry[] action_entries =
		{
			{ "select-all", null, "Select All", "<ctrl>A", null, OnSelectAll },
			{ "invert-selection", null, "Invert _selection", "<ctrl><shift>I", null, OnInvertSelection },

			{ "delete", null, "Delete", "<ctrl>D", null, OnDelete },

			{ "menu-data", null, "Data", null, null, null },
			{ "add-bool-data", null, "Bool", null, null, OnAddNode },
			{ "add-float-data", null, "Float", null, null, OnAddNode },
			{ "add-vector3-data", null, "Vector3", null, null, OnAddNode },
			{ "add-string-data", null, "String", null, null, OnAddNode },

			{ "menu-event", null, "Event", null, null, null },
			{ "add-level-loaded-event", null, "Level Loaded", null, null, OnAddNode },
			{ "add-unit-spawned-event", null, "Unit Spawned", null, null, OnAddNode },
			{ "add-physics-collision-event", null, "Physics Collision", null, null, OnAddNode },
			{ "add-external-event", null, "External Event", null, null, OnAddNode },

			{ "add-spawn-unit", null, "Spawn Unit", null, null, OnAddNode },
			{ "add-play-sound", null, "Play Sound", null, null, OnAddNode },
			{ "add-play-sprite-animation", null, "Play Sprite Animation", null, null, OnAddNode },
			{ "add-debug-print", null, "Debug Print", null, null, OnAddNode },

			{ "menu-logic", null, "Logic", null, null, null },
			{ "add-delay-node", null, "Delay", null, null, OnAddNode },
			{ "add-once-node", null, "Once", null, null, OnAddNode },
			{ "add-sequence-node", null, "Sequence", null, null, OnAddNode },
			{ "add-toggle-node", null, "Toggle", null, null, OnAddNode }
		};

		// HashMap<string, Type> _add_node_action;

		// Signals
		public signal void node_clicked(NodeInstance node);
		public signal void node_pressed(NodeInstance node);
		public signal void node_released(NodeInstance node);
		public signal void node_enter_notify(NodeInstance node);
		public signal void node_leave_notify(NodeInstance node);
		public signal void node_row_pressed(NodeInstance node, int row);
		public signal void node_row_released(NodeInstance node, int row);

		public GraphView(GraphStore store)
		{
			_model = store;

			_selected_node = GUID_ZERO;
			_enter_node = GUID_ZERO;
			_selected_row = -1;

			_selection = new HashSet<Guid?>();

			_iter_to_node = new HashMap<Guid?, int>();
			_zorder = new Gee.LinkedList<int>();
			_nodes = {};

			_iter_to_link = new HashMap<Guid?, int>();
			_links = {};

			_node_offt_x = 0;
			_node_offt_y = 0;
			_scroll_x_old = 0;
			_scroll_y_old = 0;
			_linking = false;
			_dragging = false;
			_scrolling = false;

			_grid_scale_min = 0.5;
			_grid_scale_max = 1.5;
			_grid_scale_step = 0.05;
			_grid_size = 16.0;
			_grid_scale = 1.0;
			_grid_scroll_x = 0.0;
			_grid_scroll_y = 0.0;

			// _add_node_action = new HashMap<string, Type>
			// {
			// 	{ "add-bool-data",               typeof(BoolDataNode)          },
			// 	{ "add-float-data",              typeof(FloatDataNode)         },
			// 	{ "add-vector3-data",            typeof(Vector3DataNode)       },
			// 	{ "add-string-data",             typeof(StringDataNode)        },
			// 	{ "add-level-loaded-event",      typeof(LevelLoadedEvent)      },
			// 	{ "add-unit-spawned-event",      typeof(UnitSpawnedEvent)      },
			// 	{ "add-physics-collision-event", typeof(PhysicsCollisionEvent) },
			// 	{ "add-external-event",          typeof(ExternalEvent)         },
			// 	{ "add-spawn-unit",              typeof(SpawnUnitNode)         },
			// 	{ "add-play-sound",              typeof(PlaySoundNode)         },
			// 	{ "add-play-sprite-animation",   typeof(PlaySpriteAnimation)   },
			// 	{ "add-debug-print",             typeof(DebugPrint)            },
			// 	{ "add-delay-node",              typeof(DelayNode)             },
			// 	{ "add-once-node",               typeof(OnceNode)              },
			// 	{ "add-sequence-node",           typeof(SequenceNode)          },
			// 	{ "add-toggle-node",             typeof(ToggleNode)            }
			// };

			Gtk.ActionGroup actions = new Gtk.ActionGroup("group");
			actions.add_actions(action_entries, this);

			_ui_manager = new UIManager();
			_ui_manager.insert_action_group(actions, 0);
			_ui_manager.add_ui_from_file("ui/graph_view_menu.xml");

			_menu = _ui_manager.get_widget("/popup") as Gtk.Menu;
			_node_menu = _ui_manager.get_widget("/popup-node") as Gtk.Menu;

			// _cursor_fleur = new Gdk.Cursor(Gdk.CursorType.Fleur);
			// _cursor_move = new Gdk.Cursor(Gdk.CursorType.Fleur); // FIXME
			// _cursor_resize = new Gdk.Cursor(Gdk.CursorType.SbHDoubleArrow);

			// PopupMenu += new PopupMenuHandler(ShowPopup);

			this.can_focus = true;
			this.events |= Gdk.EventMask.POINTER_MOTION_MASK
				| Gdk.EventMask.KEY_PRESS_MASK
				| Gdk.EventMask.KEY_RELEASE_MASK
				| Gdk.EventMask.FOCUS_CHANGE_MASK
				| Gdk.EventMask.SCROLL_MASK
				;

			this.button_release_event.connect(on_button_release);
			this.button_press_event.connect(on_button_press);
			this.key_press_event.connect(on_key_press);
			this.motion_notify_event.connect(on_motion_notify);
			this.scroll_event.connect(on_scroll);
			this.draw.connect(on_draw);

			CreateNodeQuad(50, 50, 6, Guid.new_guid());
			CreateNodeQuad(400, 50, 6, Guid.new_guid());
			CreateNodeQuad(800, 50, 6, Guid.new_guid());

			set_size_request(1280, 720);
		}

		private void OnDelete(Gtk.Action action)
		{
			DeleteSelection();
		}

		private void OnSelectAll(Gtk.Action action)
		{
			SelectAll();
		}

		private void OnInvertSelection(Gtk.Action action)
		{
			InvertSelection();
		}

		private void OnAddNode(Gtk.Action action)
		{
			// AddNode(_add_node_action[action.Name]);
		}

		private void ShowPopup()
		{
			// if (_menu != null)
			// 	_menu.popup();
		}

		private void ShowNodePopup()
		{
			// if (_node_menu != null)
			// 	_node_menu.popup();
		}

		private NodeInstance MakeNodeInstance(int i)
		{
			return NodeInstance(i);
		}

		private NodeInstance CreateNodeQuad(double x, double y, int num_rows, Guid iter)
		{
			WindowToWorld(ref x, ref y);

			NodeQuad node = { iter, x, y, 0 };
			node.UpdateSize(6);

			NodeInstance i = MakeNodeInstance(_nodes.length);

			_nodes += node;
			// _zorder.AddLast(i.i);
			// _iter_to_node[iter] = i.i;

			return i;
		}

		private void DestroyNodeQuad(NodeInstance i)
		{
			// int last = _nodes.Count - 1;

			// NodeQuad quad = _nodes[i.i];
			// NodeQuad last_quad = _nodes[last];

			// _nodes[i.i] = last_quad;
			// _nodes.RemoveAt(last);

			// _zorder.Remove(last);

			// _iter_to_node[last_quad.Node] = i.i;
			// _iter_to_node.Remove(quad.Node);
		}

		private LinkInstance MakeLinkInstance(int i)
		{
			return new LinkInstance(i);
		}

		private LinkInstance CreateLinkRenderer(Guid iter, Guid from_node, int from_row, Guid to_node, int to_row)
		{
			// LinkRenderer link = { Link = iter, FromNode = from_node, FromRow = from_row, ToNode = to_node, ToRow = to_row };

			// LinkInstance i = MakeLinkInstance(_links.Count);

			// _links.Add(link);
			// _iter_to_link[iter] = i.i;

			// return i;
			return { 0 };
		}

		private void DestroyLinkInstance(LinkInstance i)
		{
			// int last = _links.Count - 1;

			// LinkRenderer link = _links[i.i];
			// LinkRenderer last_link = _links[last];

			// _links[i.i] = last_link;
			// _links.RemoveAt(last);

			// _iter_to_link[last_link.Link] = i.i;
			// _iter_to_link.Remove(link.Link);
		}

		private double SnapToGrid(double x)
		{
			return x - x % _grid_size;
		}

		private void WorldToWindow(ref double x, ref double y)
		{
			x = x * _grid_scale + _grid_scroll_x;
			y = y * _grid_scale + _grid_scroll_y;
		}

		private void WindowToWorld(ref double x, ref double y)
		{
			x = (x - _grid_scroll_x) * 1.0 / _grid_scale;
			y = (y - _grid_scroll_y) * 1.0 / _grid_scale;
		}

		public NodeInstance QueryGraph(double x, double y)
		{
			WindowToWorld(ref x, ref y);

			for (int i = 0; i < _nodes.length; ++i)
			{
				if (_nodes[i].IsInside(x, y))
					return MakeNodeInstance(i);
			}

			// LinkedListNode<int> item = _zorder.Last;

			// while (item != null)
			// {
			// 	if (_nodes[item.Value].IsInside(x, y))
			// 		return MakeNodeInstance(item.Value);

			//     item = item.Previous;
			// }

			return MakeNodeInstance(int.MAX);
		}

		public int IsInsideNodeRow(NodeInstance node, double x, double y)
		{
			WindowToWorld(ref x, ref y);
			return _nodes[node.i].IsInsideRow(x, y);
		}

		public int IsInsideConnector(NodeInstance node, double x, double y)
		{
			WindowToWorld(ref x, ref y);
			return _nodes[node.i].IsInsideConnector(x, y);
		}

		public void SetNodePosition(NodeInstance i, double x, double y)
		{
			_nodes[i.i].SetPosition(SnapToGrid(x), SnapToGrid(y));
		}

		private bool on_button_press(Gdk.EventButton ev)
		{
			queue_draw();

			_mouse.UpdateStartPosition((double)ev.x, (double)ev.y);
			NodeInstance inst = QueryGraph((double)ev.x, (double)ev.y);

			if (ev.button == 1) // Left button
			{
				if (!inst.IsValid())
				{
					_selection.clear();
					return true;
				}

				NodeQuad node = _nodes[inst.i];

				double nx = node.x;
				double ny = node.y;
				WorldToWindow(ref nx, ref ny);
				_node_offt_x = (double)ev.x - nx;
				_node_offt_y = (double)ev.y - ny;

				if ((ev.state & Gdk.ModifierType.CONTROL_MASK) != 0)
				{
					if (_selection.contains(node.Node))
						_selection.remove(node.Node);
					else
						_selection.add(node.Node);
				}
				else
				{
					_selection.clear();
					_selection.add(node.Node);
				}

				_selected_node = node.Node;
				_selected_row = IsInsideNodeRow(inst, (double)ev.x, (double)ev.y);

				_linking = _selected_row != -1 && IsInsideConnector(inst, (double)ev.x, (double)ev.y) != -1;
				_dragging = _selected_row == -1 && (IsInsideConnector(inst, (double)ev.x, (double)ev.y) == -1);

				node_pressed(inst);
				if (_selected_row != -1)
					node_row_pressed(inst, _selected_row);
				return true;
			}
			if (ev.button == 2) // Middle button
			{
				_scrolling = true;
				_scroll_x_old = _grid_scroll_x;
				_scroll_y_old = _grid_scroll_y;
				// this.GdkWindow.cursor = _cursor_fleur;
				return true;
			}
			if (ev.button == 3) // Right button
			{
				if (inst.IsValid())
				{
					NodeQuad node = _nodes[inst.i];

					_selected_node = node.Node;
					if (!_selection.contains(node.Node))
					{
						_selection.clear();
						_selection.add(node.Node);
					}
					ShowNodePopup();
					return true;
				}

				ShowPopup();
				return true;
			}

			return true;
		}

		private bool on_button_release(Gdk.EventButton ev)
		{
			queue_draw();
			NodeInstance inst = QueryGraph((double)ev.x, (double)ev.y);

			// Left button
			if (ev.button == 1) {
				if (inst.IsValid())
				{
					node_released(inst);

					NodeQuad node = _nodes[inst.i];

					if (_linking)
					{
						int num_row = IsInsideNodeRow (inst, (double)ev.x, (double)ev.y);
						if (num_row != -1)
						{
							if (IsInsideConnector(inst, (double)ev.x, (double)ev.y) != -1)
							{
								// if (Model.CanLink(_selected_node, _selected_row, node.Node, num_row))
									// Model.CreateLink(_selected_node, _selected_row, node.Node, num_row);

								_linking = false;
							}
						}
					}
				}

				_selected_node = GUID_ZERO;
				_selected_row = -1;
				_dragging = false;
				_linking = false;
			}
			else if (ev.button == 2)
			{
				_scrolling = false;
				// this.GdkWindow.cursor = null; //new Gdk.Cursor (Gdk.CursorType.Fleur);
			}

			return true;
		}

		private bool on_motion_notify(Gdk.EventMotion ev)
		{
			queue_draw();

			if (_dragging || _linking)
				queue_draw();

			if (_scrolling)
			{
				queue_draw();
				_grid_scroll_x = _scroll_x_old + ((double)ev.x - _mouse.StartX);
				_grid_scroll_y = _scroll_y_old + ((double)ev.y - _mouse.StartY);
			}

			if (_selected_node != GUID_ZERO && _dragging)
			{
				NodeInstance node = MakeNodeInstance(_iter_to_node[_selected_node]);
				double xx = (double)ev.x - _node_offt_x;
				double yy = (double)ev.y - _node_offt_y;
				WindowToWorld(ref xx, ref yy);
				SetNodePosition(node, xx, yy);

				NodeQuad nq = _nodes[node.i];
				queue_draw_area((int)nq.x, (int)nq.y, (int)nq.Width, (int)nq.Height());
				return true;
			}

			stdout.printf("Querying\n");
			NodeInstance inst = QueryGraph((double)ev.x, (double)ev.y);
			if (inst.IsValid())
			{
				NodeQuad node = _nodes[inst.i];

				_enter_node = node.Node;

				node_enter_notify(inst);
				queue_draw();
				return true;
			}

			if (!inst.IsValid() && _enter_node != GUID_ZERO)
			{
				// Is node alive
				// if (Model.HasNode(_enter_node))
				{
					// node_leave_notify(_enter_node);
				}
				_enter_node = GUID_ZERO;

				queue_draw ();
				return true;
			}

			return true;
		}

		private bool on_scroll(Gdk.EventScroll ev)
		{
			stdout.printf("Srolling\n");
			if ((ev.state & Gdk.ModifierType.CONTROL_MASK) != 0)
			{
				stdout.printf("SAD\n");
				_grid_scale += _grid_scale_step * (ev.direction == Gdk.ScrollDirection.UP ? 1.0 : -1.0);
				_grid_scale = Math.fmin(_grid_scale_max, _grid_scale);
				_grid_scale = Math.fmax(_grid_scale_min, _grid_scale);
				queue_draw();
			}

			return true;
		}

		private bool on_draw(Context cr)
		{
			// Draw background
			cr.set_source_rgb(0.207, 0.219, 0.223);
			cr.paint();

			// Draw grid
			Gtk.Allocation allocation;
			get_allocation(out allocation);

			int divs = allocation.width / (int) _grid_size;
			divs = (int)Math.fmax(divs, allocation.height / (int) _grid_size);
			cr.set_source_rgb(0.107, 0.119, 0.123);
			for (int dd = 0; dd < divs; dd++)
			{
				cr.set_line_width((dd % 8 == 0) ? 0.5 : 0.2);
				cr.move_to((allocation.width * dd) / divs, 0);
				cr.line_to((allocation.width * dd) / divs, allocation.height);
				cr.move_to(0, (allocation.width * dd) / divs);
				cr.line_to(allocation.width, (allocation.width * dd) / divs);
			}
			cr.stroke();

			cr.translate(_grid_scroll_x, _grid_scroll_y);
			cr.scale(_grid_scale, _grid_scale);

		// 	// Draw links
		// 	foreach (LinkRenderer lr in _links)
		// 	{
		// 		NodeQuad from_node = _nodes[_iter_to_node[lr.FromNode]];
		// 		NodeQuad to_node = _nodes[_iter_to_node[lr.ToNode]];

		// 		Vector4 col = Vector4(1, 1, 1, 1); // TypeUtils.TypeColor[from_node._rows[item.Value.FromRow].Value.GetType()];
		// 		col = Vector4(col.R*0.75, col.G*0.75, col.B*0.75);

		// 		if (lr.FromNode != lr.ToNode)
		// 		{
		// 			Vector2 from = from_node.RowConnector(lr.FromRow);
		// 			Vector2 to = to_node.RowConnector(lr.ToRow);
		// 			Gui.Spline(cr, from, to, col);
		// 		}
		// 		else
		// 		{
		// 			Vector2 from = from_node.RowConnector(lr.FromRow);
		// 			Vector2 from1 = Vector2(from.x, from_node.y + from_node.Height() + 30);
		// 			Vector2 to = to_node.RowConnector(lr.ToRow);
		// 			Vector2 to1 = Vector2(to.x, from_node.y + from_node.Height() + 30);
		// 			Gui.SplineSelf(cr, from, from1, to, to1, col);
		// 		}
		// 	}

			// Draw nodes
			foreach (int item in _zorder)
			{
				NodeQuad quad = _nodes[item];
				bool outline = false;

				// Gui.Node(cr, quad.x, quad.y, quad.Width, quad.Height(), outline, node.Label, node._rows, col);
			}

			foreach (NodeQuad nq in _nodes)
			{
				Gui.Node(cr, nq.x, nq.y, nq.Width, nq.Height(), false, "", Vector4(1, 0, 0, 1));
			}

			// Draw current link
			if (_linking)
			{
				int xx, yy;
				xx = (int)_mouse.X;
				yy = (int)_mouse.Y;

				double xxx = (double)xx;
				double yyy = (double)yy;
				WindowToWorld(ref xxx, ref yyy);

				NodeInstance inst = QueryGraph(xxx, yyy);
				if (inst.IsValid())
				{
					int row = -1;
					if ((row = IsInsideNodeRow(inst, xxx, yyy)) != -1)
					{
						if (IsInsideConnector(inst, xxx, yyy) != -1)
						{
							// if (Model.CanLink(_nodes[inst.i].Node, row, _selected_node, _selected_row))
							// {
							// 	NodeQuad to = _nodes[inst.i];
							// 	Vector2 rxy = to.RowConnector(row);
							// 	xxx = (int)rxy.x;
							// 	yyy = (int)rxy.y;
							// }
						}
					}
				}

				NodeQuad from = _nodes[_iter_to_node[_selected_node]];
				Vector2 txy = from.RowConnector(_selected_row);
				int mbx = (int)txy.x;
				int mby = (int)txy.y;

				//Vector4 col = TypeUtils.TypeColor[from._rows[_selected_row].Value.GetType()];
				Gui.Spline(cr, Vector2((double) mbx, (double) mby), Vector2((double) xxx, (double) yyy), Vector4(1, 1, 11, 1));
			}

			return true;
		}

		private bool on_key_press(Gdk.EventKey evnt)
		{
			if (evnt.keyval == Gdk.Key.@1)
			{
				_grid_scale = 1.0;
				queue_draw();
			}

			return true;
		}

		public void DeleteSelection()
		{
			// foreach (Guid item in _selection)
			// 	Model.DeleteNode(item);

			// _selected_node = GUID_ZERO;
			// _selected_row = -1;
			// _selection.clear();
		}

		public Guid AddNode(Type node)
		{
			queue_draw();
			// return Model.CreateNode(node);
			return Guid.new_guid();
		}

		public void SelectAll()
		{
			_selection.clear();
			foreach (NodeQuad nq in _nodes)
			{
				_selection.add(nq.Node);
			}
			queue_draw();
		}

		public void InvertSelection()
		{
			foreach (NodeQuad nq in _nodes)
			{
				if (_selection.contains(nq.Node))
					_selection.remove(nq.Node);
				else
					_selection.add(nq.Node);
			}
			queue_draw();
		}

		// public void OnNodePressed(object o, NodePressedArgs args)
		// {
		// 	Console.WriteLine("Node pressed = {0}", args.Guid);
		// 	_zorder.Remove(args.Guid.i);
		// 	_zorder.AddLast(args.Guid.i);

		// 	Console.WriteLine("new z order = {0}", _zorder.Count);
		// }

		// public void OnNodeRowPressed(object o, NodeRowPressedArgs args)
		// {
		// 	Console.WriteLine("Node row pressed = {0}:{1}", args.Guid, args.Row);

		// 	NodeQuad quad = _nodes[args.Guid.i];

		// 	if (_linking || _dragging || !(Model.GetNode(quad.Node).GetRow(args.Row).Value is InVariable))
		// 		return;

		// 	NodeRow row = Model.GetNode(quad.Node).GetRow(args.Row);

		// 	MessageDialog dg = new MessageDialog((Window)this.Toplevel
		// 		, DialogFlags.Modal
		// 		, MessageType.Other
		// 		, ButtonsType.OkCancel
		// 		, row.Label
		// 		);

		// 	Entry entry = new Entry();
		// 	entry.Text = row.Value.ToString();

		// 	dg.VBox.PackEnd(entry);
		// 	dg.ShowAll();

		// 	if (dg.Run() == (int)ResponseType.Ok)
		// 	{
		// 		((InVariable)row.Value).FromString(entry.Text.Trim());
		// 	}

		// 	dg.Destroy();
		// 	queue_draw();
		// }

		// public void OnNodeInserted(object o, NodeInsertedArgs args)
		// {
		// 	CreateNodeQuad(SnapToGrid(_mouse.StartX), SnapToGrid(_mouse.StartY), Model.GetNode(args.Guid)._rows.Count, args.Guid);
		// }

		// public void OnNodeDeleted(object o, NodeDeletedArgs args)
		// {
		// 	NodeInstance inst = MakeNodeInstance(_iter_to_node[args.Guid]);
		// 	DestroyNodeQuad(inst);
		// 	queue_draw();
		// }

		// public void OnLinkInserted(object o, LinkInsertedArgs args)
		// {
		// 	Link link = _model.GetLink(args.Guid);
		// 	CreateLinkRenderer(args.Guid, link.FromNode, link.FromRow, link.ToNode, link.ToRow);
		// }

		// public void OnLinkDeleted(object o, LinkDeletedArgs args)
		// {
		// 	LinkInstance inst = MakeLinkInstance(_iter_to_link[args.Guid]);
		// 	DestroyLinkInstance(inst);
		// }
	}
}
