===========
Console API
===========

Any number of clients can connect to the engine via TCP/IP and communicate
with simple JSON API. The default port number is 10001 as defined by ``CROWN_DEFAULT_CONSOLE_PORT``.
You can change the port number with ``--console-port`` option.

TCP protocol
------------

Every message starts with u32 header containing the size in bytes of the
following JSON message.

JSON messages
-------------

Every JSON message has a ``type`` key which identifies its type.
Sending messages with unknown ``type`` will result with an error message:

.. code::

	{
		"type" : "error",
		"message" : "Unknown command"
	}

Execute Lua scripts
-------------------

Executing Lua script is as simple as sending the following message:

.. code::

	{
		"type" : "script",
		"script" : "print('Hello, world!')"
	}

All the tools are controlled by sending simple Lua scripts to them.
