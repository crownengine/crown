using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Collections;

namespace Crown
{
namespace Core
{
	public delegate void MessageReceived(object o, MessageReceivedArgs args);
	public class MessageReceivedArgs : EventArgs
	{
		public readonly string Json;
		public MessageReceivedArgs(string json)
		{
			Json = json;
		}
	}

	public delegate void Connected(object o, ConnectedArgs args);
	public class ConnectedArgs : EventArgs
	{
		public readonly string Address;
		public readonly int Port;
		public ConnectedArgs(string address, int port)
		{
			Address = address;
			Port = port;
		}
	}

	public delegate void Disconnected(object o, DisconnectedArgs args);
	public class DisconnectedArgs : EventArgs
	{
		public DisconnectedArgs()
		{
		}
	}

	// Manages communication with engine executable.
	public class ConsoleClient
	{
		private Socket Client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

		// Events
		public event Connected ConnectedEvent;
		public event Disconnected DisconnectedEvent;
		public event MessageReceived MessageReceivedEvent;

		public void Connect(string address, int port)
		{
			try {
				Client.BeginConnect(new IPEndPoint(IPAddress.Parse(address), port), new AsyncCallback(OnConnected), Client);
			}
			catch (Exception e) {
				Console.WriteLine(e.ToString());
			}
		}

		public void Close()
		{
			try {
				if (Client.Connected) {
					Client.Shutdown(SocketShutdown.Both);
					Client.Close();
					EmitDisconnected();
				}
			}
			catch (Exception e) {
				Console.WriteLine(e.ToString());
			}
		}

		private void OnConnected(IAsyncResult ar)
		{
			try {
				Socket client = (Socket)ar.AsyncState;
				client.EndConnect(ar);
				IPEndPoint ep = client.RemoteEndPoint as IPEndPoint;
				EmitConnected(ep.Address.ToString(), ep.Port);
				Receive(client);
			}
			catch (Exception e) {
				Console.WriteLine(e.ToString());
			}
		}

		// Sends the JSON-encoded data to the target
		public void Send(string json)
		{
			try {
				Client.Send(BitConverter.GetBytes(json.Length));
				Client.Send(Encoding.ASCII.GetBytes(json));
			} catch (Exception e) {
				Console.WriteLine(e.ToString());
			}
		}

		// Sends the lua script to the target
		public void SendScript(string lua)
		{
			lua = lua.Replace("\"", "\\\"");
			string json = "{\"type\":\"script\",\"script\":\"" + lua + "\"}";
			Send(json);
		}

		private class StateObject
		{
			public Socket Client;
			public byte[] msgHeader = new byte[4];
			public byte[] buf = new byte[4096];
		}

		private void Receive(Socket client)
		{
			try {
				StateObject state = new StateObject();
				state.Client = client;
				Client.BeginReceive(state.msgHeader, 0, 4, SocketFlags.None, new AsyncCallback(OnReceive), state);
			}
			catch (Exception e) {
				Console.WriteLine(e.ToString());
			}
		}

		private void OnReceive(IAsyncResult ar)
		{
			try {
				StateObject state = (StateObject)ar.AsyncState;
				Socket client = state.Client;

				int bytesRead = client.EndReceive(ar);
				if(bytesRead > 0)
				{
					Int32 size = BitConverter.ToInt32(state.msgHeader, 0);
					int rr = client.Receive(state.buf, size, SocketFlags.None);
					string json = Encoding.ASCII.GetString(state.buf, 0, rr);
					EmitMessageReceived(json);
					Receive(client);
				}
				else
				{
					Close();
				}
			}
			catch(Exception e) {
				Console.WriteLine(e.ToString());
			}
		}

		public void EmitDisconnected()
		{
			if (DisconnectedEvent != null)
				DisconnectedEvent(this, new DisconnectedArgs());
		}

		public void EmitMessageReceived(string json)
		{
			if (MessageReceivedEvent != null)
				MessageReceivedEvent(this, new MessageReceivedArgs(json));
		}

		public void EmitConnected(string address, int port)
		{
			if (ConnectedEvent != null)
				ConnectedEvent(this, new ConnectedArgs(address, port));
		}
	}
} // namespace Core
} // namespace Crown
