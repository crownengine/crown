/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Config.h"
#include "Str.h"
#include "List.h"
#include "Types.h"

namespace crown
{

/**
	OS-specific functions and parameters.
*/
namespace os
{

//-----------------------------------------------------------------------------
#ifdef LINUX
const size_t	MAX_OS_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '/';
const size_t	MAX_OS_EVENTS = 512;

#endif

#ifdef WINDOWS
const size_t	MAX_OS_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '\\';

const size_t	MAX_OS_EVENTS = 512;
#endif

//-----------------------------------------------------------------------------
void			printf(const char* string, ...);				//!< Print32_tf wrapper
void			vprintf(const char* string, va_list arg);		//!< VPrint32_tf wrapper

void			log_debug(const char* string, va_list arg);		//!< Print32_t debug message
void			log_error(const char* string, va_list arg);		//!< Print32_t error message
void			log_warning(const char* string, va_list arg);	//!< Print32_t warning message
void			log_info(const char* string, va_list arg);		//!< Print32_t info message

bool			exists(const char* path);		//!< Returns whether the path is a file or directory on the disk

bool			is_dir(const char* path);		//!< Returns whether the path is a directory. (May not resolve symlinks.)
bool			is_reg(const char* path);		//!< Returns whether the path is a regular file. (May not resolve symlinks.)

bool			mknod(const char* path);		//! Creates a regular file. Returns true if success, false if not
bool			unlink(const char* path);		//! Deletes a regular file. Returns true if success, false if not
bool			mkdir(const char* path);		//! Creates a directory. Returns true if success, false if not
bool			rmdir(const char* path);		//! Deletes a directory. Returns true if success, false if not

const char*		get_cwd();						//! Fills ret with the path of the current working directory. Returns true if success, false if not 
const char*		get_home();						//! Fills ret with the path of the user home directory
const char*		get_env(const char* env);		//! Returns the content of the 'env' environment variable or the empty string

bool			ls(const char* path, List<Str>& fileList);	//! Returns the list of filenames in a directory.

//-----------------------------------------------------------------------------
void			init_os();

bool			create_render_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool fullscreen);
bool			destroy_render_window();
void			get_render_window_metrics(uint32_t& width, uint32_t& height);
void			swap_buffers();

void			event_loop();

void			init_input();
void			get_cursor_xy(int32_t& x, int32_t& y);
void			set_cursor_xy(int32_t x, int32_t y);

void			hide_cursor();
void			show_cursor();

//-----------------------------------------------------------------------------

enum OSEventType
{
	OSET_NONE				= 0,

	OSET_KEY_PRESS			= 1,
	OSET_KEY_RELEASE		= 2,

	OSET_BUTTON_PRESS		= 3,
	OSET_BUTTON_RELEASE		= 4,
	OSET_MOTION_NOTIFY		= 5
};

struct OSEvent
{
	OSEventType		type;
	int32_t			data_a;
	int32_t			data_b;
	int32_t			data_c;
	int32_t			data_d;
};

//! Pushes @a event int32_to @a event_queue
void				push_event(OSEventType type, int32_t data_a, int32_t data_b, int32_t data_c, int32_t data_d);

//! Returns the event on top of the @a event_queue	
OSEvent&			pop_event();



//-----------------------------------------------------------------------------
//		Networking
//-----------------------------------------------------------------------------

struct IPv4Address
{
	uint8_t 	address[4];
	uint16_t 	port;
	
	uint32_t get_address()
	{
		uint32_t addr = address[0] << 24 | address[1] << 16 | address[2] << 8 | address[3];
		
		return addr;
	}
	
	uint16_t get_port()
	{
		return port;
	}
	
	void set(uint32_t a, uint16_t p)
	{
		address[0] = (uint8_t) a >> 24;
		address[1] = (uint8_t) a >> 16;
		address[2] = (uint8_t) a >> 8;
		address[3] = (uint8_t) a;

		port = p;
	}
};

//-----------------------------------------------------------------------------

class UDPSocket
{
public:

				// Constructor
				UDPSocket();
				// Destructor
				~UDPSocket();
				// Open connection
	bool 		open(uint16_t port);
				 // Send data through socket
	bool 		send(IPv4Address &receiver, const void* data, int32_t size );
				// Receive data through socket
	int32_t 	receive(IPv4Address &sender, void* data, int32_t size);
				// Close connection
	void 		close();
				// Is connection open?
	bool 		is_open();

private:
				// Socket descriptor
	int32_t 	m_socket;
};
//-----------------------------------------------------------------------------

class TCPSocket
{
public:

				// Constructor
				TCPSocket();
				// Destructor
				~TCPSocket();
				// Open connection (server side)
	bool 		open(uint16_t port);
				// Connect (client side)
	bool		connect(IPv4Address& destination);
				// Close connection
	int32_t		close();
				// Send data through socket
	bool 		send(const void* data, int32_t size);
				// Receive data through socket
	int32_t		receive(void* data, int32_t size);
				// Is connection open?
	bool 		is_open();
				// Getter method for socket descriptor
	int32_t 	get_socket_id();
				// Getter method for active socket descriptor
	int32_t 	get_active_socket_id();
				// Setter method for socket descriptor

private:

	void 		set_socket_id(int32_t socket);
				// Setter method for ative socket descriptor
	void 		set_active_socket_id(int32_t socket);
	
				// Generated by ::socket
	int32_t 	m_socket;
			  	// Generated by ::accept
	int32_t 	m_active_socket;
}; 

} // namespace os
} // namespace crown

