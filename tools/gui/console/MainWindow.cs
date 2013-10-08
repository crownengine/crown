using System;
using Gtk;
using Gdk;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using Newtonsoft.Json.Linq;

public partial class MainWindow: Gtk.Window
{
	private System.Net.Sockets.Socket m_sock = null;
	private TextTag tagInfo;
	private TextTag tagWarning;
	private TextTag tagError;
	private TextTag tagDebug;

	// Console history
	private const uint MAX_HISTORY_ITEMS = 256;
	private uint history_size = 0;
	private uint history_current = 0;
	private string[] history = new string[MAX_HISTORY_ITEMS];

	// Socket recv buffer
	private byte [] m_byBuff = new byte[4096]; // Recieved data buffer

	/// <summary>
	/// Initializes a new instance of the <see cref="MainWindow"/> class.
	/// </summary>
	public MainWindow (): base (Gtk.WindowType.Toplevel)
	{
		Build ();
		Connect ();

		// Create tags for color-formatted text
		tagInfo = new Gtk.TextTag ("info");
		tagInfo.BackgroundGdk = new Gdk.Color (255, 255, 255);

		tagWarning = new Gtk.TextTag ("warning");
		tagWarning.BackgroundGdk = new Gdk.Color (255, 255, 153);

		tagError = new Gtk.TextTag ("error");
		tagError.BackgroundGdk = new Gdk.Color (255, 153, 153);

		tagDebug = new Gtk.TextTag ("debug");
		tagDebug.BackgroundGdk = new Gdk.Color (224, 224, 224);

		TextBuffer textbuffer1 = textview1.Buffer;
		textbuffer1.TagTable.Add (tagInfo);
		textbuffer1.TagTable.Add (tagWarning);
		textbuffer1.TagTable.Add (tagError);
		textbuffer1.TagTable.Add (tagDebug);

		// Create completion dictionary
		ListStore lua_api = new ListStore (typeof (string));

		lua_api.AppendValues ("Device.frame_count");
		lua_api.AppendValues ("Device.last_delta_time");
		lua_api.AppendValues ("Device.start");
		lua_api.AppendValues ("Device.stop");
		lua_api.AppendValues ("Device.create_resource_package");
		lua_api.AppendValues ("Device.destroy_resource_package");
		lua_api.AppendValues ("Window.show");
		lua_api.AppendValues ("Window.hide");
		lua_api.AppendValues ("Window.get_size");
		lua_api.AppendValues ("Window.get_position");
		lua_api.AppendValues ("Window.resize");
		lua_api.AppendValues ("Window.move");
		lua_api.AppendValues ("Window.minimize");
		lua_api.AppendValues ("Window.restore");
		lua_api.AppendValues ("Window.is_resizable");
		lua_api.AppendValues ("Window.set_resizable");
		lua_api.AppendValues ("Window.show_cursor");
		lua_api.AppendValues ("Window.get_cursor_xy");
		lua_api.AppendValues ("Window.set_cursor_xy");
		lua_api.AppendValues ("Window.title");
		lua_api.AppendValues ("Window.set_title");
		lua_api.AppendValues ("Math.deg_to_rad");
		lua_api.AppendValues ("Math.rad_to_deg");
		lua_api.AppendValues ("Math.next_pow_2");
		lua_api.AppendValues ("Math.is_pow_2");
		lua_api.AppendValues ("Math.ceil");
		lua_api.AppendValues ("Math.floor");
		lua_api.AppendValues ("Math.sqrt");
		lua_api.AppendValues ("Math.inv_sqrt");
		lua_api.AppendValues ("Math.sin");
		lua_api.AppendValues ("Math.cos");
		lua_api.AppendValues ("Math.asin");
		lua_api.AppendValues ("Math.acos");
		lua_api.AppendValues ("Math.tan");
		lua_api.AppendValues ("Math.atan2");
		lua_api.AppendValues ("Math.abs");
		lua_api.AppendValues ("Math.fmod");
		lua_api.AppendValues ("Vec2.new");
		lua_api.AppendValues ("Vec2.val");
		lua_api.AppendValues ("Vec2.add");
		lua_api.AppendValues ("Vec2.sub");
		lua_api.AppendValues ("Vec2.mul");
		lua_api.AppendValues ("Vec2.div");
		lua_api.AppendValues ("Vec2.dot");
		lua_api.AppendValues ("Vec2.equals");
		lua_api.AppendValues ("Vec2.lower");
		lua_api.AppendValues ("Vec2.greater");
		lua_api.AppendValues ("Vec2.length");
		lua_api.AppendValues ("Vec2.squared_length");
		lua_api.AppendValues ("Vec2.set_length");
		lua_api.AppendValues ("Vec2.normalize");
		lua_api.AppendValues ("Vec2.negate");
		lua_api.AppendValues ("Vec2.get_distance_to");
		lua_api.AppendValues ("Vec2.get_angle_between");
		lua_api.AppendValues ("Vec2.zero");
		lua_api.AppendValues ("Vec3.new");
		lua_api.AppendValues ("Vec3.val");
		lua_api.AppendValues ("Vec3.add");
		lua_api.AppendValues ("Vec3.sub");
		lua_api.AppendValues ("Vec3.mul");
		lua_api.AppendValues ("Vec3.div");
		lua_api.AppendValues ("Vec3.dot");
		lua_api.AppendValues ("Vec3.cross");
		lua_api.AppendValues ("Vec3.equals");
		lua_api.AppendValues ("Vec3.lower");
		lua_api.AppendValues ("Vec3.greater");
		lua_api.AppendValues ("Vec3.length");
		lua_api.AppendValues ("Vec3.squared_length");
		lua_api.AppendValues ("Vec3.set_length");
		lua_api.AppendValues ("Vec3.normalize");
		lua_api.AppendValues ("Vec3.negate");
		lua_api.AppendValues ("Vec3.get_distance_to");
		lua_api.AppendValues ("Vec3.get_angle_between");
		lua_api.AppendValues ("Vec3.zero");
		lua_api.AppendValues ("Quat.new");
		lua_api.AppendValues ("Quat.negate");
		lua_api.AppendValues ("Quat.load_identity");
		lua_api.AppendValues ("Quat.length");
		lua_api.AppendValues ("Quat.conjugate");
		lua_api.AppendValues ("Quat.inverse");
		lua_api.AppendValues ("Quat.cross");
		lua_api.AppendValues ("Quat.mul");
		lua_api.AppendValues ("Quat.pow");
		lua_api.AppendValues ("StringSetting.value");
		lua_api.AppendValues ("StringSetting.synopsis");
		lua_api.AppendValues ("StringSetting.update");
		lua_api.AppendValues ("IntSetting.value");
		lua_api.AppendValues ("IntSetting.synopsis");
		lua_api.AppendValues ("IntSetting.min");
		lua_api.AppendValues ("IntSetting.max");
		lua_api.AppendValues ("IntSetting.update");
		lua_api.AppendValues ("FloatSetting.value");
		lua_api.AppendValues ("FloatSetting.synopsis");
		lua_api.AppendValues ("FloatSetting.min");
		lua_api.AppendValues ("FloatSetting.max");
		lua_api.AppendValues ("FloatSetting.update");
		lua_api.AppendValues ("Mouse.button_pressed");
		lua_api.AppendValues ("Mouse.button_released");
		lua_api.AppendValues ("Mouse.any_pressed");
		lua_api.AppendValues ("Mouse.any_released");
		lua_api.AppendValues ("Mouse.cursor_xy");
		lua_api.AppendValues ("Mouse.set_cursor_xy");
		lua_api.AppendValues ("Mouse.cursor_relative_xy");
		lua_api.AppendValues ("Mouse.set_cursor_relative_xy");
		lua_api.AppendValues ("Mouse.MB_LEFT");
		lua_api.AppendValues ("Mouse.KB_MIDDLE");
		lua_api.AppendValues ("Mouse.MB_RIGHT");
		lua_api.AppendValues ("Keyboard.modifier_pressed");
		lua_api.AppendValues ("Keyboard.button_pressed");
		lua_api.AppendValues ("Keyboard.button_released");
		lua_api.AppendValues ("Keyboard.any_pressed");
		lua_api.AppendValues ("Keyboard.any_released");
		lua_api.AppendValues ("Keyboard.TAB");
		lua_api.AppendValues ("Keyboard.ENTER");
		lua_api.AppendValues ("Keyboard.ESCAPE");
		lua_api.AppendValues ("Keyboard.SPACE");
		lua_api.AppendValues ("Keyboard.BACKSPACE");
		lua_api.AppendValues ("Keyboard.KP_0");
		lua_api.AppendValues ("Keyboard.KP_1");
		lua_api.AppendValues ("Keyboard.KP_2");
		lua_api.AppendValues ("Keyboard.KP_3");
		lua_api.AppendValues ("Keyboard.KP_4");
		lua_api.AppendValues ("Keyboard.KP_5");
		lua_api.AppendValues ("Keyboard.KP_6");
		lua_api.AppendValues ("Keyboard.KP_7");
		lua_api.AppendValues ("Keyboard.KP_8");
		lua_api.AppendValues ("Keyboard.KP_9");
		lua_api.AppendValues ("Keyboard.F1");
		lua_api.AppendValues ("Keyboard.F2");
		lua_api.AppendValues ("Keyboard.F3");
		lua_api.AppendValues ("Keyboard.F4");
		lua_api.AppendValues ("Keyboard.F5");
		lua_api.AppendValues ("Keyboard.F6");
		lua_api.AppendValues ("Keyboard.F7");
		lua_api.AppendValues ("Keyboard.F8");
		lua_api.AppendValues ("Keyboard.F9");
		lua_api.AppendValues ("Keyboard.F10");
		lua_api.AppendValues ("Keyboard.F11");
		lua_api.AppendValues ("Keyboard.F12");
		lua_api.AppendValues ("Keyboard.HOME");
		lua_api.AppendValues ("Keyboard.LEFT");
		lua_api.AppendValues ("Keyboard.UP");
		lua_api.AppendValues ("Keyboard.RIGHT");
		lua_api.AppendValues ("Keyboard.DOWN");
		lua_api.AppendValues ("Keyboard.PAGE_UP");
		lua_api.AppendValues ("Keyboard.PAGE_DOWN");
		lua_api.AppendValues ("Keyboard.LCONTROL");
		lua_api.AppendValues ("Keyboard.RCONTROL");
		lua_api.AppendValues ("Keyboard.LSHIFT");
		lua_api.AppendValues ("Keyboard.RSHIFT");
		lua_api.AppendValues ("Keyboard.CAPS_LOCK");
		lua_api.AppendValues ("Keyboard.LALT");
		lua_api.AppendValues ("Keyboard.RALT");
		lua_api.AppendValues ("Keyboard.LSUPER");
		lua_api.AppendValues ("Keyboard.RSUPER");
		lua_api.AppendValues ("Keyboard.NUM_0");
		lua_api.AppendValues ("Keyboard.NUM_1");
		lua_api.AppendValues ("Keyboard.NUM_2");
		lua_api.AppendValues ("Keyboard.NUM_3");
		lua_api.AppendValues ("Keyboard.NUM_4");
		lua_api.AppendValues ("Keyboard.NUM_5");
		lua_api.AppendValues ("Keyboard.NUM_6");
		lua_api.AppendValues ("Keyboard.NUM_7");
		lua_api.AppendValues ("Keyboard.NUM_8");
		lua_api.AppendValues ("Keyboard.NUM_9");
		lua_api.AppendValues ("Keyboard.A");
		lua_api.AppendValues ("Keyboard.B");
		lua_api.AppendValues ("Keyboard.C");
		lua_api.AppendValues ("Keyboard.D");
		lua_api.AppendValues ("Keyboard.E");
		lua_api.AppendValues ("Keyboard.F");
		lua_api.AppendValues ("Keyboard.G");
		lua_api.AppendValues ("Keyboard.H");
		lua_api.AppendValues ("Keyboard.I");
		lua_api.AppendValues ("Keyboard.J");
		lua_api.AppendValues ("Keyboard.K");
		lua_api.AppendValues ("Keyboard.L");
		lua_api.AppendValues ("Keyboard.M");
		lua_api.AppendValues ("Keyboard.N");
		lua_api.AppendValues ("Keyboard.O");
		lua_api.AppendValues ("Keyboard.P");
		lua_api.AppendValues ("Keyboard.Q");
		lua_api.AppendValues ("Keyboard.R");
		lua_api.AppendValues ("Keyboard.S");
		lua_api.AppendValues ("Keyboard.T");
		lua_api.AppendValues ("Keyboard.U");
		lua_api.AppendValues ("Keyboard.V");
		lua_api.AppendValues ("Keyboard.W");
		lua_api.AppendValues ("Keyboard.X");
		lua_api.AppendValues ("Keyboard.Y");
		lua_api.AppendValues ("Keyboard.Z");
		lua_api.AppendValues ("Keyboard.a");
		lua_api.AppendValues ("Keyboard.b");
		lua_api.AppendValues ("Keyboard.c");
		lua_api.AppendValues ("Keyboard.d");
		lua_api.AppendValues ("Keyboard.e");
		lua_api.AppendValues ("Keyboard.f");
		lua_api.AppendValues ("Keyboard.g");
		lua_api.AppendValues ("Keyboard.h");
		lua_api.AppendValues ("Keyboard.i");
		lua_api.AppendValues ("Keyboard.j");
		lua_api.AppendValues ("Keyboard.k");
		lua_api.AppendValues ("Keyboard.l");
		lua_api.AppendValues ("Keyboard.m");
		lua_api.AppendValues ("Keyboard.n");
		lua_api.AppendValues ("Keyboard.o");
		lua_api.AppendValues ("Keyboard.p");
		lua_api.AppendValues ("Keyboard.q");
		lua_api.AppendValues ("Keyboard.r");
		lua_api.AppendValues ("Keyboard.s");
		lua_api.AppendValues ("Keyboard.t");
		lua_api.AppendValues ("Keyboard.u");
		lua_api.AppendValues ("Keyboard.v");
		lua_api.AppendValues ("Keyboard.w");
		lua_api.AppendValues ("Keyboard.x");
		lua_api.AppendValues ("Keyboard.y");
		lua_api.AppendValues ("Keyboard.z");
		lua_api.AppendValues ("ResourcePackage.load");
		lua_api.AppendValues ("ResourcePackage.unload");
		lua_api.AppendValues ("ResourcePackage.flush");
		lua_api.AppendValues ("ResourcePackage.has_loaded");
		lua_api.AppendValues("Camera.local_position");
		lua_api.AppendValues("Camera.local_rotation");
		lua_api.AppendValues("Camera.local_pose");
		lua_api.AppendValues("Camera.world_position");
		lua_api.AppendValues("Camera.world_rotation");
		lua_api.AppendValues("Camera.world_pose");
		lua_api.AppendValues("Camera.set_local_position");
		lua_api.AppendValues("Camera.set_local_rotation");
		lua_api.AppendValues("Camera.set_local_pose");
		lua_api.AppendValues("Camera.set_projection_type");
		lua_api.AppendValues("Camera.projection_type");
		lua_api.AppendValues("Camera.fov");
		lua_api.AppendValues("Camera.set_fov");
		lua_api.AppendValues("Camera.aspect");
		lua_api.AppendValues("Camera.set_aspect");
		lua_api.AppendValues("Camera.near_clip_distance");
		lua_api.AppendValues("Camera.set_near_clip_distance");
		lua_api.AppendValues("Camera.far_clip_distance");
		lua_api.AppendValues("Camera.set_far_clip_distance");
		lua_api.AppendValues("Mesh.local_position");
		lua_api.AppendValues("Mesh.local_rotation");
		lua_api.AppendValues("Mesh.local_pose");
		lua_api.AppendValues("Mesh.world_position");
		lua_api.AppendValues("Mesh.world_rotation");
		lua_api.AppendValues("Mesh.world_pose");
		lua_api.AppendValues("Mesh.set_local_position");
		lua_api.AppendValues("Mesh.set_local_rotation");
		lua_api.AppendValues("Mesh.set_local_pose");
		lua_api.AppendValues("Unit.local_position");
		lua_api.AppendValues("Unit.local_rotation");
		lua_api.AppendValues("Unit.local_pose");
		lua_api.AppendValues("Unit.world_position");
		lua_api.AppendValues("Unit.world_rotation");
		lua_api.AppendValues("Unit.world_pose");
		lua_api.AppendValues("Unit.set_local_position");
		lua_api.AppendValues("Unit.set_local_rotation");
		lua_api.AppendValues("Unit.set_local_pose");
		lua_api.AppendValues("World.spawn_unit");
		lua_api.AppendValues("World.play_sound");
		lua_api.AppendValues("World.pause_sound");
		lua_api.AppendValues("World.link_sound");
		lua_api.AppendValues("World.set_listener");
		lua_api.AppendValues("World.set_sound_position");
		lua_api.AppendValues("World.set_sound_range");
		lua_api.AppendValues("World.set_sound_volume");

		entry1.Completion = new EntryCompletion ();
		entry1.Completion.Model = lua_api;
		entry1.Completion.TextColumn = 0;
	}

	/// <summary>
	/// Connect this instance.
	/// </summary>
	public void Connect()
	{
		// Close the socket if it is still open
		if( m_sock != null && m_sock.Connected )
		{
			m_sock.Shutdown( SocketShutdown.Both );
			System.Threading.Thread.Sleep( 10 );
			m_sock.Close();
		}

		// Create the socket object
		m_sock = new System.Net.Sockets.Socket( AddressFamily.InterNetwork, 
		                                       SocketType.Stream, ProtocolType.Tcp );    

		// Define the Server address and port
		IPEndPoint epServer = new IPEndPoint(  IPAddress.Parse("127.0.0.1"), 10001 );

		// Connect to the server blocking method
		// and setup callback for recieved data
		// m_sock.Connect( epServer );
		// SetupRecieveCallback( m_sock );

		// Connect to server non-Blocking method
		m_sock.Blocking = false;
		AsyncCallback onconnect = new AsyncCallback( OnConnect );
		m_sock.BeginConnect( epServer, onconnect, m_sock );
	}

	/// <summary>
	/// Raises the connect event.
	/// </summary>
	/// <param name="ar">Ar.</param>
	public void OnConnect( IAsyncResult ar )
	{
		// Socket was the passed in object
		System.Net.Sockets.Socket sock = (System.Net.Sockets.Socket)ar.AsyncState;

		// Check if we were sucessfull
		try
		{
			//    sock.EndConnect( ar );
			if( sock.Connected )
				SetupRecieveCallback( sock );
			else
				Console.Write("Unable to connect to remote machine");
		}
		catch( Exception ex )
		{
			Console.Write("Unable to connect to remote machine");
		}
	}

	/// <summary>
	/// Setups the recieve callback.
	/// </summary>
	/// <param name="sock">Sock.</param>
	public void SetupRecieveCallback(System.Net.Sockets.Socket sock)
	{
		try
		{
			AsyncCallback recieveData = new AsyncCallback( OnRecievedData );
			sock.BeginReceive( m_byBuff, 0, m_byBuff.Length, 
			                  SocketFlags.None, recieveData, sock );
		}
		catch( Exception ex )
		{
			Console.Write("Setup Recieve Callback failed!");
		}
	}

	/// <summary>
	/// Raises the recieved data event.
	/// </summary>
	/// <param name="ar">Ar.</param>
	public void OnRecievedData( IAsyncResult ar )
	{
		// Socket was the passed in object
		System.Net.Sockets.Socket sock = (System.Net.Sockets.Socket)ar.AsyncState;

		// Check if we got any data
		try
		{
			int nBytesRec = sock.EndReceive( ar );
			if( nBytesRec > 0 )
			{
				// Wrote the data to the List
				string sRecieved = Encoding.ASCII.GetString( m_byBuff, 0, nBytesRec );

				Console.Write(sRecieved);

				JObject obj = JObject.Parse(sRecieved);
				if (obj["type"].ToString() == "message")
				{
					string severity = obj["severity"].ToString();
					string message = obj["message"].ToString();

					if (severity == "info")
					{
						WriteLog(message, tagInfo);
					}
					else if (severity == "warning")
					{
						WriteLog(message, tagWarning);
					}
					else if (severity == "error")
					{
						WriteLog(message, tagError);
					}
					else if (severity == "debug")
					{
						WriteLog(message, tagDebug);
					}
				}
				else
				{
					WriteLog("Unknown response from server", tagInfo);
				}

				// If the connection is still usable restablish the callback
				SetupRecieveCallback( sock );
			}
			else
			{
				// If no data was recieved then the connection is probably dead
				Console.WriteLine( "Client {0}, disconnected", 
				                  sock.RemoteEndPoint );
				sock.Shutdown( SocketShutdown.Both );
				sock.Close();
			}
		}
		catch( Exception ex )
		{
			Console.Write("Unusual error druing Recieve!");
		}
	}

	protected void WriteLog(string text, TextTag tag)
	{
		Gtk.Application.Invoke (delegate
		{
			TextIter endIter = textview1.Buffer.EndIter;
			textview1.Buffer.Insert(ref endIter, text);
			endIter.BackwardChars(text.Length);
			textview1.Buffer.ApplyTag(tag, endIter, textview1.Buffer.EndIter);
			textview1.ScrollToMark(textview1.Buffer.CreateMark("bottom", textview1.Buffer.EndIter, false), 0, true, 0.0, 1.0);
		});
	}

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}

	protected void SendScript(String script)
	{
		String json = "{\"type\":\"script\",\"script\":\"";
		json += script;
		json += "\"}";
		Console.Write (json);
		m_sock.Send (Encoding.ASCII.GetBytes (json));
	}

	protected void SendCommand(String command)
	{
		char[] delimiterChars = { ' ', '\t' };
		string[] words = command.Split(delimiterChars);

		string cmd = words[0];
		string resource_type = words[1];
		string resource_name = words[2];
	
		string json = "{\"type\":\"command\",\"command\":\"";
		json += cmd;
		json += "\",";
		json += "\"resource_type\":";
		json += "\"";
		json += resource_type;
		json += "\",";
		json += "\"resource_name\":";
		json +="\"";
		json += resource_name;
		json += "\"}";
		Console.Write (json);
		m_sock.Send (Encoding.ASCII.GetBytes (json));
	}

	protected void OnConnectActivated (object sender, EventArgs e)
	{
		Connect ();
	}

	protected void OnEntryActivated (object sender, EventArgs e)
	{
		string text = entry1.Text;
		text = text.Trim ();

		// Do processing only if we have text
		if (text.Length > 0)
		{
			history [history_size % MAX_HISTORY_ITEMS] = text;
			history_size++;
			history_current = history_size;

			// Log entered text
			WriteLog ("> " + text + "\n", tagInfo);

			// Sanitize text entry
			text = text.Replace("\"", "\\\"");

			if (combobox1.Active == 0) {
				SendScript (text);
			} else {
				SendCommand (text);
			}
		}

		entry1.Text = "";
	}

	protected void OnEntryKeyPressed (object o, KeyPressEventArgs args)
	{
		switch (args.Event.Key)
		{
			case Gdk.Key.Down:
			{
				if (history_current < history_size)
				{
					history_current++;
				}
				break;
			}
			case Gdk.Key.Up:
			{
				if (history_current > 0)
				{
					history_current--;
				}
				break;
			}
			default:
			{
				return;
			}
		}

		if (history_size == history_current)
		{
			entry1.Text = "";
		}
		else
		{
			entry1.Text = history [history_current % MAX_HISTORY_ITEMS];
			entry1.Position = entry1.Text.Length;
		}

		args.RetVal = true;
	}
}
