#include "Types.h"
#include "TCPSocket.h"
#include "Thread.h"
#include "Mutex.h"

namespace crown
{

struct ConsoleCommand
{
	char	command[1024];
};

/// ConsoleServer runs scripts that provide some utilities for the console
class ConsoleServer
{
public:

	/// Constructor
							ConsoleServer();
	/// Start listening on @port
	void					init();
	/// Stop listening
	void					shutdown();

	void					read_eval_loop();

	void					add_command(const char* cmd);

	void					execute();
	/// Send data to client
	void					send(const void* data, size_t size = 1024);
	/// Receive data to client
	void					receive(char* data, size_t size = 1024);
	/// Return the list of command
	void 					command_list();

private:

	void 					parse_command(const uint8_t* cmd);

	static void*			background_thread(void* thiz);

private:

	os::TCPSocket			m_socket;

	bool					m_active;

	uint32_t				m_count;

	ConsoleCommand			m_buffer[16];

	os::Thread				m_thread;
	os::Mutex				m_command_mutex;

	/// Lua script which initializes ConsoleServer
	static const char*		init_console;
	/// Lua script which provides a list of all commands
	static const char*		retrieve_cmds;
};

} // namespace crown