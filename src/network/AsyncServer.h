#pragma once

namespace crown
{
	// states for the server's authorization process
	typedef enum 
	{
		AS_WAIT = 0,	// we are waiting for a confirm/deny from auth
						// this is subject to timeout if we don't hear from auth
						// or a permanent wait if auth said so
		AS_OK,
		AS_ONLYLAN,
		AS_WAIT,
		AS_OK,
		AS_MAXSTATES
	} 
	authState;
	
//-------------------------------------------------------------------------------------

	// states from the auth server, while the client is in CDK_WAIT
	typedef enum 
	{
		AR_NONE = 0,	// no reply yet
		AR_OK,			// this client is good
		AR_WAIT,		// wait - keep sending me srvAuth though
		AR_DENY,		// denied - don't send me anything about this client anymore
		AR_MAXSTATES
	} 
	authReply;

//-------------------------------------------------------------------------------------

	// message from auth to be forwarded back to the client
	// auth server has the possibility to send a custom reply
	typedef enum 
	{
		ARM_WAITING = 0,	// waiting on an initial reply from auth
		ARM_UNKNOWN,		// client unknown to auth
		ARM_DENIED,			// access denied
		ARM_PRINT,			// custom message
		ARM_SRVWAIT,		// auth server replied and tells us he's working on it
		ARM_MAXSTATES
	} 
	authReplyMsg;

//-------------------------------------------------------------------------------------

	typedef enum 
	{
	  SCS_FREE = 0,			// can be reused for a new connection
	  SCS_ZOMBIE,			// client has been disconnected, but don't reuse connection for a couple seconds
	  SCS_WAIT,				// client needs to update it's pure checksums before we can go further
	  SCS_CONNECTED,		// client is connected
	  SCS_INGAME			// client is in the game
	} 
	serverClientState;

//-------------------------------------------------------------------------------------

	typedef struct serverClient_s 
	{
		int					OS;
		int					client_id;
		serverClientState	client_state;
		int					client_prediction;
		int					client_ahead_time;
		int					client_rate;
		int					client_ping;

		int					game_init_sequence;
		int					game_frame;
		int					game_time;

// 		idMsgChannel		channel;
		int					last_connect_time;
		int					last_empty_time;
		int					last_ping_time;
		int					last_snapshot_time;
		int					last_packet_time;
		int					last_input_time;
// 		int					snapshotSequence;
// 		int					acknowledgeSnapshotSequence;
// 		int					numDuplicatedUsercmds;

// 		char				guid[12];  // Even Balance - M. Quinn

	} 
	serverClient;

//-------------------------------------------------------------------------------------

	class IPv4Address;

//-------------------------------------------------------------------------------------

	/**
	 * Server arch over UDPSocket.
	 */
	class AsyncServer
	{
	public:
							AsyncServer();
							~AsyncServer();

		bool				init_port();
		void				close_port();
		
// 		void				Spawn();
// 		void				Kill();
// 		void				ExecuteMapChange();
		
		
		int					get_port() const;
		IPv4Address			get_bound_address() const;
		bool				is_active() const;
		int					get_delay() const;
		int					get_outgoing_rate() const;
		int					get_incoming_rate() const;
		bool				is_client_in_game(int client_num) const;
		int					get_client_ping(int client_num) const;
		int					get_client_prediction(int client_num) const;
		int					get_client_time_since_last_packet(int client_num) const;
		int					get_client_time_since_last_input(int client_num) const;
		int					get_client_outgoing_rate(int client_num) const;
		int					get_client_incoming_rate(int client_num) const;
		float				get_client_outgoing_compression(int client_num) const;
		float				get_client_incoming_compression(int client_num) const;
		float				get_client_incoming_packet_loss(int client_num) const;
		int					get_num_clients() const;
		int					get_num_idle_clients() const;
		int					get_local_client_num() const 
		
 		void				run_frame();
// 		void				processConnectionLessMessages( void );
// 		void				RemoteConsoleOutput( const char *string );
		void				send_reliable_game_message(int clientNum, const idBitMsg &msg);
		void				send_reliable_game_message_excluding(int clientNum, const idBitMsg &msg);
// 		void				LocalClientSendReliableMessage( const idBitMsg &msg );

		void				master_heartbeat( bool force = false );
		void				drop_client( int clientNum, const char *reason );

// 		void				PacifierUpdate( void );

// 		void				UpdateUI( int clientNum );

		void				update_async_stats_avg();
// 		void				get_async_stats_avg_msg(idStr &msg);

		void				print_local_server_info();
		
		const int 			MAX_ASYNC_CLIENTS = 30;	// tmp value
		
	private:

		bool				active;						// true if server is active
		int					real_time;					// absolute time

		int					server_time;				// local server time
		IPv4Address			server_addr;				// UDP port
		int					server_id;					// server identification
		int					server_data_checksum;		// checksum of the data used by the server
		int					local_client_num;			// local client on listen server

		serverClient		clients[MAX_ASYNC_CLIENTS];	// clients

		int					game_init_id;				// game initialization identification
		int					game_frame;					// local game frame
		int					game_time;					// local game time
		int					game_time_residual;			// left over time from previous frame
	
		int					next_heartbeat_time;
		int					next_async_stats_time;

		bool				noRconOutput;				// for default rcon response when command is silent

		int					last_auth_time;				// global for auth server timeout

		// track the max outgoing rate over the last few secs to watch for spikes
		// dependent on net_serverSnapshotDelay. 50ms, for a 3 seconds backlog -> 60 samples
		static const int	stats_numsamples = 60;
		int					stats_outrate[ stats_numsamples ];
		int					stats_current;
		int					stats_average_sum;
		int					stats_max;
		int					stats_max_index;	  
	};
}