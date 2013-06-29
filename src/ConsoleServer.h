#include "Types.h"
#include "TCPSocket.h"
#include "Thread.h"
#include "Mutex.h"
#include "Cond.h"

namespace crown
{

class ConsoleServer
{
public:

					ConsoleServer();

	void			start(uint32_t port);
	void			stop();
	void			receive_command();


private:

	void 			parse_command();

	os::TCPSocket	m_socket;

	uint8_t			data[1024];
};

} // namespace crown