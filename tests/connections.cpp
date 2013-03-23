#include <cstdio>
#include <cstring>

#include "OS.h"
#include "MallocAllocator.h"
#include "BitMessage.h"
#include "AsyncConnection.h"
#include "PacketQueue.h"

using namespace crown;

const unsigned int server_port = 30000;
const unsigned int client_port = 30001;
const uint32_t protocol_id = 0x00;
const float delta_time = 1.0f / 45.0f;
const real send_rate = 1.0f / 45.0f;

const float time_out = 10.0f;

enum Mode
{
    SERVER,
    CLIENT
};

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		os::printf("Usage: ./connections server/client.\n");
		return -1;
	}
	
	bool connected = false;
	real send_acc = 0.0f;
	Mode mode = SERVER;
	os::NetAddress addr(127, 0, 0, 1, server_port);
	
	// creates connection
	MallocAllocator alloc;
	network::AsyncConnection connection(alloc);
	
	if (strcmp(argv[1], "server") == 0)
	{
		mode = SERVER;
	}
	else if (strcmp(argv[1], "client") == 0)
	{
		mode = CLIENT;
	}
	else
	{
		os::printf("Usage: ./connections server/client.\n");
		return -1;
	}
	
	uint16_t port = mode == SERVER ? server_port : client_port;

	// connection init
	connection.init(protocol_id, 10.0f);
	//connection start
	if (!connection.start(port))
	{
        os::printf("Couldn't start connection on port %d\n", port);
		return -1;
	}
	
	if (mode == CLIENT)
	{
		connection.connect(addr);
	}
	else if (mode == SERVER)
	{
		connection.listen();
	}	
	
	// main loop
	while (true)
	{
       
        if (!connected && connection.is_connected())
        {
            os::printf("Client connected to server\n");
            connected = true;
        }
        
        if (!connected && connection.is_connect_fail())
        {
            os::printf("Connection failed\n");
            break;
        }
                
        send_acc += delta_time;
		
		if (mode == SERVER)
		{
			while (true)
			{
				network::BitMessage received(alloc);
				
				received.init(6);
				
				int32_t bytes = connection.receive_message(received, delta_time);
				if (bytes > 0)
				{
					uint8_t* header = received.get_header();
					uint32_t protocol_id = header[0] << 24 | header[1] << 16 | header[2] << 8 | header[3];
					uint16_t sequence = header[4] << 8 | header[5];
					uint16_t ack = header[6] << 8 | header[7];
					uint32_t ack_bits = header[8] << 24 | header[9] << 16 | header[10] << 8 | header[11];  
					char string[6];
					received.read_string(string, 6);
					os::printf("------------------\n");
					os::printf("protocol_id: %d\n", protocol_id);
					os::printf("sequence: %d\n", sequence);
					os::printf("ack: %d\n", ack);
					os::printf("ack_bits: %d\n", ack_bits);
 					os::printf("data: %s\n", string);
					os::printf("------------------\n");

				} 
				
				if (bytes <= 0)
				{
					break;
				}
			}
		}
		
		if (mode == CLIENT)
		{
			while (send_acc > 1 / send_rate)
			{
				network::BitMessage message(alloc);
			
				message.init(6);
				message.begin_writing();
				message.write_string("prova", 6);
				
				connection.send_message(message, delta_time);

				uint8_t* header = message.get_header();
				uint32_t protocol_id = header[0] << 24 | header[1] << 16 | header[2] << 8 | header[3];
				uint16_t sequence = header[4] << 8 | header[5];
				uint16_t ack = header[6] << 8 | header[7];
				uint32_t ack_bits = header[8] << 24 | header[9] << 16 | header[10] << 8 | header[11];
				os::printf("------------------\n");
				os::printf("protocol_id: %d\n", protocol_id);
				os::printf("sequence: %d\n", sequence);
				os::printf("ack: %d\n", ack);
				os::printf("ack_bits: %d\n", ack_bits);
				os::printf("------------------\n");

				send_acc -= 1.0f / send_rate;
			}
		}
	}
	
	connection.stop();
  
}
