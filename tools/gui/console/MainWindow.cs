using System;
using Gtk;
using Gdk;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using Newtonsoft.Json.Linq;
using Crown.Console;
using Crown.Core;

public partial class MainWindow: Gtk.Window
{
	private EntryHistory History = new EntryHistory(256);
	private ConsoleClient Client = null;

	private ActionGroup Actions = null;
	private UIManager UI = new UIManager();

	private ScrolledWindow scrolledwindow1;
	private TextView textview1;
	private Entry entry1;

	private string Address = "127.0.0.1";
	private int Port = 10001;

	public MainWindow (): base (Gtk.WindowType.Toplevel)
	{
		Build ();

		ActionEntry[] entries = new ActionEntry[] {
			new ActionEntry("Unpause", null, "Unpause", "<ctrl>U", null, OnUnpause),
			new ActionEntry("EngineMenu", null, "_Engine", null, null, null),

			new ActionEntry("RebuildAndReload", null, "Rebuild And Reload", null, null, OnRebuildAndReload),
			new ActionEntry("ScriptMenu", null, "_Script", null, null, null),

			new ActionEntry("Reconnect", null, "Reconnect", "<ctrl>R", null, null),
			new ActionEntry("ConnectMenu", null, "_Connect", null, null, null)
		};

		Actions = new ActionGroup("group");
		Actions.Add(entries);
		UI.InsertActionGroup(Actions, 0);
		UI.AddUiFromResource("Menu.xml");
		AddAccelGroup(UI.AccelGroup);

		MenuBar menuBar = (MenuBar)UI.GetWidget("/MenuBar");
		vbox1.PackStart(menuBar, false, false, 0);

		// Create tags for color-formatted text
		TextTag tagInfo = new Gtk.TextTag ("info");
		tagInfo.BackgroundGdk = new Gdk.Color (255, 255, 255);
		TextTag tagWarning = new Gtk.TextTag ("warning");
		tagWarning.BackgroundGdk = new Gdk.Color (255, 255, 153);
		TextTag tagError = new Gtk.TextTag ("error");
		tagError.BackgroundGdk = new Gdk.Color (255, 153, 153);
		TextTag tagDebug = new Gtk.TextTag ("debug");
		tagDebug.BackgroundGdk = new Gdk.Color (224, 224, 224);

		textview1 = new TextView();
		textview1.Editable = false;
		textview1.CanFocus = false;
		TextBuffer textbuffer1 = textview1.Buffer;
		textbuffer1.TagTable.Add (tagInfo);
		textbuffer1.TagTable.Add (tagWarning);
		textbuffer1.TagTable.Add (tagError);
		textbuffer1.TagTable.Add (tagDebug);

		scrolledwindow1 = new ScrolledWindow();
		scrolledwindow1.Add(textview1);
		vbox1.PackStart(scrolledwindow1, true, true, 0);

		entry1 = new Entry();
		entry1.KeyPressEvent += new KeyPressEventHandler(OnEntryKeyPressed);
		entry1.Activated += new EventHandler(OnEntryActivated);
		vbox1.PackStart(entry1, false, true, 0);

		EnableMainMenu(false);
		Actions.GetAction("ScriptMenu").Sensitive = false;
		Client = new ConsoleClient();
		Client.ConnectedEvent += OnConnected;
		Client.DisconnectedEvent += OnDisconnected;
		Client.MessageReceivedEvent += OnMessageReceived;
		Connect(Address, Port);
		ShowAll();
	}

	protected void EnableMainMenu(bool enable)
	{
		Actions.GetAction("EngineMenu").Sensitive = enable;
		// Actions.GetAction("ScriptMenu").Sensitive = enable;
	}

	protected void OnConnected(object o, ConnectedArgs args)
	{
		EnableMainMenu(true);
		LogInfo("Connected to " + args.Address + ":" + args.Port.ToString() + "\n");
	}

	protected void OnDisconnected(object o, DisconnectedArgs args)
	{
		EnableMainMenu(false);
		LogInfo("Disconnected\n");
	}

	protected void Connect(string addr, int port)
	{
		LogInfo("Trying " + addr + ":" + port.ToString() + "\n");
		Client.Connect(addr, port);
	}

	protected void OnMessageReceived(object o, MessageReceivedArgs args)
	{
		JObject obj = JObject.Parse(args.Json);
		if (obj["type"].ToString() == "message")
		{
			string severity = obj["severity"].ToString();
			string message = obj["message"].ToString();

			if (severity == "info") LogInfo(message);
			else if (severity == "warning") LogWarning(message);
			else if (severity == "error") LogError(message);
			else if (severity == "debug") LogDebug(message);
		}
	}

	protected void OnEntryActivated (object sender, EventArgs e)
	{
		string text = entry1.Text;
		text = text.Trim ();

		// Do processing only if we have text
		if (text.Length > 0)
		{
			History.Push(text);
			Client.SendScript(text);
			LogInfo("> " + text + "\n");
		}

		entry1.Text = "";
	}

	protected void OnEntryKeyPressed (object o, KeyPressEventArgs args)
	{
		string text = "";

		switch (args.Event.Key)
		{
			case Gdk.Key.Down: text = History.Next(); break;
			case Gdk.Key.Up: text = History.Previous(); break;
			default: return; // Ignore other keys
		}

		entry1.Text = text;
		entry1.Position = entry1.Text.Length;
		args.RetVal = true;
	}

	protected void OnUnpause(object o, EventArgs args)
	{
		Client.Send("{\"type\":\"command\",\"command\":\"unpause\"}");
	}

	protected void OnRebuildAndReload(object o, EventArgs args)
	{
		Client.Send("{\"type\":\"command\",\"command\":\"reload\"}");
	}

	// Logging
	private void WriteLog(string text, string tag)
	{
		Gtk.Application.Invoke (delegate {
			TextIter endIter = textview1.Buffer.EndIter;
			textview1.Buffer.Insert(ref endIter, text);
			endIter.BackwardChars(text.Length);
			textview1.Buffer.ApplyTag(textview1.Buffer.TagTable.Lookup(tag), endIter, textview1.Buffer.EndIter);
			textview1.ScrollToMark(textview1.Buffer.CreateMark("bottom", textview1.Buffer.EndIter, false), 0, true, 0.0, 1.0);
		});
	}

	private void LogInfo(string text)
	{
		WriteLog(text, "info");
	}

	private void LogWarning(string text)
	{
		WriteLog(text, "warning");
	}

	private void LogError(string text)
	{
		WriteLog(text, "error");
	}

	private void LogDebug(string text)
	{
		WriteLog(text, "debug");
	}

	private void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}
}
