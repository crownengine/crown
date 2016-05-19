/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using GLib;

namespace Crown
{
	/// Manages communication with engine executable.
	public class ConsoleClient : GLib.Object
	{
		private SocketClient _socket;
		private SocketConnection _connection;

		// Signals
		public signal void connected();
		public signal void disconnected();
		public signal void message_received(ConsoleClient client, uint8[] json);

		public ConsoleClient()
		{
			_socket = new SocketClient();
			_connection = null;
		}

		public new void connect(string address, int port)
		{
			try
			{
				_connection = _socket.connect(new InetSocketAddress.from_string(address, port), null);
				if (_connection != null)
					connected();
			}
			catch (Error e)
			{
				stderr.printf("%s\n", e.message);
			}
		}

		public void close()
		{
			try
			{
				_connection.close();
				_connection = null;
				disconnected();
			}
			catch (Error e)
			{
				stderr.printf("%s\n", e.message);
			}
		}

		public bool is_connected()
		{
			return _connection != null && _connection.is_connected();
		}

		// Sends the JSON-encoded data to the target
		public void send(string json)
		{
			if (_connection == null)
				return;

			try
			{
				// FIXME: Add bit conversion utils
				uint32 len = json.length;
				uint8* ptr = (uint8*)(&len);
				var array = new uint8[4];
				for (var i = 0; i < 4; ++i)
				    array[i] = ptr[i];

				_connection.output_stream.write(array);
				_connection.output_stream.write(json.data);
			}
			catch (Error e)
			{
				stderr.printf("%s\n", e.message);
			}
		}

		// Sends the lua script to the target
		public void send_script(string lua)
		{
			send("{\"type\":\"script\",\"script\":\"" + lua.replace("\"", "\\\"") + "\"}");
		}

		public void receive_async()
		{
			try
			{
				_connection.input_stream.read_bytes_async.begin(4, GLib.Priority.HIGH, null, on_read);
			}
			catch (Error e)
			{
				stderr.printf("%s\n", e.message);
			}
		}

		private void on_read(Object? obj, AsyncResult ar)
		{
			try
			{
				InputStream input_stream = (InputStream)obj;
				uint8[] header = input_stream.read_bytes_async.end(ar).get_data();

				// FIXME: Add bit conversion utils
				uint32 size = 0;
				size |= header[3] << 24;
				size |= header[2] << 16;
				size |= header[1] << 8;
				size |= header[0] << 0;

				uint8[] data = new uint8[size];
				input_stream.read(data);
				message_received(this, data);
			}
			catch (Error e)
			{
				stderr.printf("%s\n", e.message);
			}
		}
	}

} // namespace Crown
