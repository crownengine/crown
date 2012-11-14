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
void			printf(const char* string, ...);				//!< Printf wrapper
void			vprintf(const char* string, va_list arg);		//!< VPrintf wrapper

void			log_debug(const char* string, va_list arg);		//!< Print debug message
void			log_error(const char* string, va_list arg);		//!< Print error message
void			log_warning(const char* string, va_list arg);	//!< Print warning message
void			log_info(const char* string, va_list arg);		//!< Print info message

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

bool			create_render_window(uint x, uint y, uint width, uint height, bool fullscreen);
bool			destroy_render_window();
void			get_render_window_metrics(uint& width, uint& height);
void			swap_buffers();

void			event_loop();

void			init_input();
void			get_cursor_xy(int& x, int& y);
void			set_cursor_xy(int x, int y);

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
	int				data_a;
	int				data_b;
	int				data_c;
	int				data_d;
};

//! Pushes @a event into @a event_queue
void				push_event(OSEventType type, int data_a, int data_b, int data_c, int data_d);

//! Returns the event on top of the @a event_queue	
OSEvent&			pop_event();



//-----------------------------------------------------------------------------
//		Networking
//-----------------------------------------------------------------------------

struct IPv4Address
{
	uchar 	address[4];
	ushort 	port;
	
	uint get_address()
	{
		uint addr = address[0] << 24 | address[1] << 16 | address[2] << 8 | address[3];
		
		return addr;
	}
	
	ushort get_port()
	{
		return port;
	}
	
	void set(uint a, ushort p)
	{
		address[0] = (uchar) a >> 24;
		address[1] = (uchar) a >> 16;
		address[2] = (uchar) a >> 8;
		address[3] = (uchar) a;

		port = p;
	}
};

class UDPSocket
{
	public:

					// Constructor
					UDPSocket();
					// Destructor
					~UDPSocket();
					// Open connection
		bool 		open(ushort port);
					 // Send data through socket
		bool 		send(IPv4Address &receiver, const void* data, int size );
					// Receive data through socket
		int 		receive(IPv4Address &sender, void* data, int size);
					// Close connection
		void 		close();
					// Is connection open?
		bool 		is_open();

	private:
					// Socket descriptor
		int 		m_socket;  
};

class TCPSocket
{
	public:

					// Constructor
					TCPSocket();
					// Destructor
					~TCPSocket();
					// Open connection (server side)
		bool 		open(ushort port);
					// Connect (client side)
		bool		connect(IPv4Address& destination);
					// Close connection
		int			close();
					// Send data through socket
		bool 		send(const void* data, int size);
					// Receive data through socket
		int			receive(void* data, int size);
					// Is connection open?
		bool 		is_open();
					// Getter method for socket descriptor
		int 		get_socket_id();
					// Getter method for active socket descriptor
		int 		get_active_socket_id();
				  // Setter method for socket descriptor

	private:

		void 		set_socket_id(int socket);
					// Setter method for ative socket descriptor
		void 		set_active_socket_id(int socket);
		
					// Generated by ::socket
		int 		m_socket;
				  // Generated by ::accept
		int 		m_active_socket;
}; 

} // namespace os

} // namespace crown