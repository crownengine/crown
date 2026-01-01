/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
/// Manages communication with engine executable.
public class ConsoleClient : GLib.Object
{
	public SocketConnection _connection;

	// Signals
	public signal void connected(string address, int port);
	public signal void disconnected();
	public signal void message_received(ConsoleClient client, uint8[] json);

	public ConsoleClient()
	{
		_connection = null;
	}

	public new void connect(string address, int port)
	{
		try {
			GLib.SocketClient client = new GLib.SocketClient();
			_connection = client.connect(new InetSocketAddress.from_string(address, port), null);
			if (_connection != null) {
				connected(address, port);
				receive_async();
			}
		} catch (Error e) {
			// Ignore
		}
	}

	public void close()
	{
		try {
			if (_connection != null) {
				_connection.close();
				_connection = null;
				disconnected();
			}
		} catch (Error e) {
			loge(e.message);
		}
	}

	public bool is_connected()
	{
		return _connection != null && _connection.is_connected();
	}

	// Sends the JSON-encoded data to the target
	public void send(string json)
	{
		if (!is_connected())
			return;

		try {
			// FIXME: Add bit conversion utils
			uint32 len = json.length;
			uint8* ptr = (uint8*)(&len);
			uint8 header[4];
			for (var i = 0; i < 4; ++i)
				header[i] = ptr[i];

			size_t bytes_read;
			_connection.output_stream.write_all(header, out bytes_read);
			_connection.output_stream.write_all(json.data, out bytes_read);
		} catch (Error e) {
			loge(e.message);
		}
	}

	// Sends the lua script to the target
	public void send_script(string lua)
	{
		send("{\"type\":\"repl\",\"repl\":\"\"\"%s\"\"\"}".printf(lua));
	}

	public void receive_async()
	{
		_connection.input_stream.read_bytes_async.begin(4, GLib.Priority.DEFAULT, null, on_read);
	}

	public void on_read(Object? obj, AsyncResult ar)
	{
		try {
			InputStream input_stream = (InputStream)obj;
			uint8[] header = input_stream.read_bytes_async.end(ar).get_data();

			// Connection closed gracefully
			if (header.length == 0) {
				close();
				return;
			}
			assert(header.length > 0);

			// FIXME: Add bit conversion utils
			uint32 size = 0;
			size |= header[3] << 24;
			size |= header[2] << 16;
			size |= header[1] << 8;
			size |= header[0] << 0;

			uint8[] data = new uint8[size];
			size_t bytes_read = 0;
			if (input_stream.read_all(data, out bytes_read)) {
				message_received(this, data);
				receive_async();
			}
		} catch (Error e) {
			if (e.code == 44) // An existing connection was forcibly closed by the remote host.
				close();
		}
	}
}

} // namespace Crown
