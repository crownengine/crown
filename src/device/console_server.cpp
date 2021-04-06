/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/file_buffer.inl"
#include "core/filesystem/reader_writer.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/network/ip_address.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_stream.inl"
#include "core/thread/scoped_mutex.inl"
#include "device/console_server.h"
#include "device/log.h"

LOG_SYSTEM(CONSOLE_SERVER, "console_server")

namespace crown
{
namespace console_server_internal
{
	static void message_command(ConsoleServer& cs, u32 client_id, const char* json, void* /*user_data*/)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		JsonArray args(ta);

		sjson::parse(obj, json);
		sjson::parse_array(args, obj["args"]);

		DynamicString command_name(ta);
		sjson::parse_string(command_name, args[0]);

		ConsoleServer::CommandData cmd;
		cmd.command_function = NULL;
		cmd.user_data = NULL;
		cmd = hash_map::get(cs._commands, command_name.to_string_id(), cmd);

		if (cmd.command_function != NULL)
			cmd.command_function(cs, client_id, args, cmd.user_data);
	}

	static void command_help(ConsoleServer& cs, u32 client_id, JsonArray& args, void* /*user_data*/)
	{
		if (array::size(args) != 1)
		{
			cs.error(client_id, "Usage: help");
			return;
		}

		u32 longest = 0;

		auto cur = hash_map::begin(cs._commands);
		auto end = hash_map::end(cs._commands);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(cs._commands, cur);

			if (longest < strlen32(cur->second.name))
				longest = strlen32(cur->second.name);
		}

		cur = hash_map::begin(cs._commands);
		end = hash_map::end(cs._commands);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(cs._commands, cur);

			logi(CONSOLE_SERVER, "%s%*s%s"
				, cur->second.name
				, longest - strlen32(cur->second.name) + 2
				, " "
				, cur->second.brief
				);
		}
	}

	static u32 add_client(ConsoleServer& cs, const TCPSocket& socket)
	{
		ScopedMutex scoped_mutex(cs._clients_mutex);

		ConsoleServer::Client client;
		client.socket = socket;
		client.id = cs._next_client_id++;
		vector::push_back(cs._clients, client);

		return client.id;
	}

	static void remove_client_by_socket(ConsoleServer& cs, const TCPSocket& socket)
	{
		ScopedMutex scoped_mutex(cs._clients_mutex);

		const u32 last = vector::size(cs._clients) - 1;
		for (u32 cc = 0; cc < vector::size(cs._clients); ++cc)
		{
			if (cs._clients[cc].socket == socket)
			{
				cs._clients[cc] = cs._clients[last];
				vector::pop_back(cs._clients);
				return;
			}
		}
	}

	static u32 get_client_id(ConsoleServer& cs, const TCPSocket& socket)
	{
		ScopedMutex scoped_mutex(cs._clients_mutex);

		const u32 num_clients = vector::size(cs._clients);
		for (u32 cc = 0; cc < num_clients; ++cc)
		{
			if (cs._clients[cc].socket == socket)
				return cs._clients[cc].id;
		}

		return UINT32_MAX;
	}

	static bool get_socket_by_id(TCPSocket* socket, ConsoleServer& cs, u32 id)
	{
		ScopedMutex scoped_mutex(cs._clients_mutex);

		const u32 num_clients = vector::size(cs._clients);
		for (u32 cc = 0; cc < num_clients; ++cc)
		{
			if (cs._clients[cc].id == id)
			{
				*socket = cs._clients[cc].socket;
				return true;
			}
		}

		return false;
	}

} // namespace console_server_internal

ConsoleServer::ConsoleServer(Allocator& a)
	: _port(UINT16_MAX)
	, _next_client_id(0)
	, _clients(a)
	, _messages(a)
	, _commands(a)
	, _thread_exit(false)
	, _input_0(a)
	, _input_1(a)
	, _input_write(&_input_0)
	, _input_read(&_input_1)
	, _output_0(a)
	, _output_1(a)
	, _output_write(&_output_0)
	, _output_read(&_output_1)
{
	this->register_message_type("command", console_server_internal::message_command, this);
	this->register_command_name("help", "List all commands", console_server_internal::command_help, this);
}

void ConsoleServer::listen(u16 port, bool wait)
{
	_port = port;
	_server.bind(port);
	_server.listen(5);
	_active_socket_set.set(&_server);

	_input_thread.start([](void* thiz) { return ((ConsoleServer*)thiz)->run_input_thread(); }, this);
	_output_thread.start([](void* thiz) { return ((ConsoleServer*)thiz)->run_output_thread(); }, this);

	if (wait)
		_client_connected.wait();
}

void ConsoleServer::shutdown()
{
	_thread_exit = true;

	if (_input_thread.is_running())
	{
		// Unlock input thread if it is stuck waiting for _handlers_semaphore.
		execute_message_handlers(false);

		// Unlock input thread if it is stuck inside the select().
		TCPSocket dummy;
		dummy.connect(IP_ADDRESS_LOOPBACK, _port);

		_input_thread.stop();
		dummy.close();
	}

	_output_condition.signal();
	if (_output_thread.is_running())
		_output_thread.stop();

	ScopedMutex scoped_mutex(_clients_mutex);
	for (u32 i = 0; i < vector::size(_clients); ++i)
		_clients[i].socket.close();

	_server.close();
}

void ConsoleServer::send(u32 client_id, const char* json)
{
	TCPSocket socket;
	if (!console_server_internal::get_socket_by_id(&socket, *this, client_id))
		return;

	const u32 msg_len = strlen32(json);

	_output_mutex.lock();
	FileBuffer fb(*_output_write);
	fb.seek_to_end();
	BinaryWriter bw(fb);
	bw.write(client_id);
	bw.write(msg_len);
	bw.write(json, msg_len);
	_output_condition.signal();
	_output_mutex.unlock();
}

void ConsoleServer::error(u32 client_id, const char* msg)
{
	TempAllocator4096 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"error\",\"message\":\"" << msg << "\"}";
	send(client_id, string_stream::c_str(ss));
}

void ConsoleServer::broadcast(const char* json)
{
	for (u32 i = 0; i < vector::size(_clients); ++i)
		send(_clients[i].id, json);
}

void ConsoleServer::execute_message_handlers(bool sync)
{
	bool locked = true;
	if (sync)
		_input_semaphore.wait();
	else
		locked = _input_semaphore.try_wait();

	if (!locked)
		return;

	Buffer* temp = _input_read;
	_input_read = _input_write;
	_input_write = temp;
	_handlers_semaphore.post();

	// Do not execute message handlers at exit, because when _thread_exit is
	// set by shutdown(), handlers may reference stale objects.
	if (_thread_exit)
		return;

	FileBuffer fb(*_input_read);
	BinaryReader br(fb);
	while (!fb.end_of_file())
	{
		// Read client, message size and message.
		u32 client_id;
		u32 msg_len;
		br.read(client_id);
		br.read(msg_len);
		const char* msg = array::begin(*_input_read) + fb.position();
		br.skip(msg_len);


		// Process message.
		JsonObject obj(default_allocator());
		sjson::parse(obj, msg);

		if (!json_object::has(obj, "type"))
		{
			error(client_id, "Missing command type");
			continue;
		}

		// Find handler for the message type.
		CommandData cmd;
		cmd.message_function = NULL;
		cmd.user_data = NULL;
		cmd = hash_map::get(_messages
			, sjson::parse_string_id(obj["type"])
			, cmd
			);
		if (!cmd.message_function)
		{
			error(client_id, "Unknown command type");
			continue;
		}

		// Call the handler.
		cmd.message_function(*this, client_id, msg, cmd.user_data);
	}

	array::clear(*_input_read);
}

void ConsoleServer::register_command_name(const char* name, const char* brief, CommandTypeFunction function, void* user_data)
{
	CE_ENSURE(NULL != name);
	CE_ENSURE(NULL != brief);
	CE_ENSURE(NULL != function);

	CommandData cmd;
	cmd.command_function = function;
	cmd.user_data = user_data;
	strncpy(cmd.name, name, sizeof(cmd.name)-1);
	strncpy(cmd.brief, brief, sizeof(cmd.brief)-1);
	hash_map::set(_commands, StringId32(name), cmd);
}

void ConsoleServer::register_message_type(const char* type, MessageTypeFunction function, void* user_data)
{
	CE_ENSURE(NULL != type);
	CE_ENSURE(NULL != function);

	CommandData cmd;
	cmd.message_function = function;
	cmd.user_data = user_data;
	hash_map::set(_messages, StringId32(type), cmd);
}

s32 ConsoleServer::run_input_thread()
{
	while (!_thread_exit)
	{
		// Wait for input from one of the sockets in _active_socket_set.
		_read_socket_set = _active_socket_set;
		SelectResult ret = _read_socket_set.select(UINT32_MAX);
		if (ret.error == SelectResult::GENERIC_ERROR)
		{
			return -1;
		}
		else if (ret.error == SelectResult::TIMEOUT)
		{
			continue;
		}

		FileBuffer fb(*_input_write);
		BinaryWriter bw(fb);
		// Read data from all clients that are ready.
		const u32 num_sockets = _read_socket_set.num();
		for (u32 ii = 0; ii < num_sockets; ++ii)
		{
			TCPSocket cur_socket = _read_socket_set.get(ii);

			// Skip if socket is not ready for reading.
			if (_read_socket_set.isset(&cur_socket) == false)
				continue;

			// If ready socket is the one listening for incoming connections.
			if (cur_socket == _server)
			{
				if (_thread_exit)
					break;

				// Accept the incoming connection.
				TCPSocket client;
				AcceptResult ar = _server.accept_nonblock(client);
				if (ar.error == AcceptResult::SUCCESS)
				{
					console_server_internal::add_client(*this, client);
					_active_socket_set.set(&client);
					_client_connected.post();
				}
			}
			else // Check if any other socket is ready for reading.
			{
				u32 msg_len = 0;
				ReadResult rr = cur_socket.read(&msg_len, 4);

				if (rr.error != ReadResult::SUCCESS)
				{
					console_server_internal::remove_client_by_socket(*this, cur_socket);
					_active_socket_set.clr(&cur_socket);
					cur_socket.close();
					continue;
				}

				const u32 client_id = console_server_internal::get_client_id(*this, cur_socket);

				// Add client header and message length.
				bw.write(client_id);
				bw.write(msg_len);

				// Read message.
				u32 num_read;
				for (num_read = 0; num_read < msg_len;)
				{
					char buf[4096];
					const u32 num_pending = min(u32(sizeof(buf)), msg_len - num_read);
					rr = cur_socket.read(buf, num_pending);
					if (rr.error != ReadResult::SUCCESS)
					{
						console_server_internal::remove_client_by_socket(*this, cur_socket);
						_active_socket_set.clr(&cur_socket);
						cur_socket.close();
						break;
					}

					bw.write(buf, rr.bytes_read);
					num_read += rr.bytes_read;
				}

				if (num_read != msg_len)
				{
					// Remove partial data that has been written to the input buffer.
					for (u32 cc = 0; cc < 4 + 4 + num_read; ++cc)
						array::pop_back(*_input_write);
				}
			}
		}

		if (array::size(*_input_write) > 0)
		{
			_input_semaphore.post();
			_handlers_semaphore.wait();
		}
	}

	return 0;
}

s32 ConsoleServer::run_output_thread()
{
	while (1)
	{
		_output_mutex.lock();
		while (array::size(*_output_write) == 0 && !_thread_exit)
			_output_condition.wait(_output_mutex);

		if (_thread_exit)
		{
			_output_mutex.unlock();
			break;
		}

		Buffer* temp = _output_read;
		_output_read = _output_write;
		_output_write = temp;
		_output_mutex.unlock();

		FileBuffer fb(*_output_read);
		BinaryReader br(fb);
		while (!fb.end_of_file())
		{
			// Read client, message size and message.
			u32 client_id;
			u32 msg_len;
			br.read(client_id);
			br.read(msg_len);
			const char* msg = array::begin(*_output_read) + fb.position();
			br.skip(msg_len);

			// Lookup socket by its ID.
			TCPSocket socket;
			if (console_server_internal::get_socket_by_id(&socket, *this, client_id) != true)
				continue;

			socket.write(msg-4, msg_len+4);
		}

		array::clear(*_output_read);
	}

	return 0;
}

namespace console_server_globals
{
	ConsoleServer* _console_server = NULL;

	void init()
	{
		_console_server = CE_NEW(default_allocator(), ConsoleServer)(default_allocator());
	}

	void shutdown()
	{
		_console_server->shutdown();
		CE_DELETE(default_allocator(), _console_server);
		_console_server = NULL;
	}

} // namespace console_server_globals

ConsoleServer* console_server()
{
	return console_server_globals::_console_server;
}

} // namespace crown
