
namespace crown
{
	AsyncServer::AsyncServer()
	{
	  
	}

	AsyncServer::~AsyncServer()
	{
	  
	}

	bool AsyncServer::init_port()
	{
	  
	}

	void AsyncServer::close_port()
	{
	  
	}
			
	int	AsyncServer::get_port() const
	{
	  
	}

	IPv4Address AsyncServer::get_bound_address() const
	{
	  
	}

	bool AsyncServer::is_active() const
	{
	  
	}

	int	AsyncServer::get_delay() const
	{
	  
	}

	int	AsyncServer::get_outgoing_rate() const
	{
	  
	}

	int	get_incoming_rate() const
	{
	
	}

	bool is_client_in_game(int client_num) const
	{
	  
	}

	int	AsyncServer::get_client_ping(int client_num) const
	{
	  
	}

	int	AsyncServer::get_client_prediction(int client_num) const
	{
	  
	}

	int	AsyncServer::get_client_time_since_last_packet(int client_num) const
	{
	  
	}

	int	AsyncServer::get_client_time_since_last_input(int client_num) const
	{
	  
	}

	int	AsyncServer::get_client_outgoing_rate(int client_num) const
	{
	  
	}

	int	AsyncServer::get_client_incoming_rate(int client_num) const
	{
	  
	}

	float AsyncServer::get_client_outgoing_compression(int client_num) const
	{
	  
	}

	float AsyncServer::get_client_incoming_compression(int client_num) const
	{
	  
	}

	float AsyncServer::get_client_incoming_packet_loss(int client_num) const
	{
	  
	}

	int	AsyncServer::get_num_clients() const
	{
	  
	}

	int	AsyncServer::get_num_idle_clients() const
	{
	  
	}

	int	AsyncServer::get_local_client_num() const 
	{
	  
	}
			
	void AsyncServer::run_frame()
	{
	  
	}

	void AsyncServer::send_reliable_game_message(int clientNum, const idBitMsg &msg)
	{
	  
	}

	void AsyncServer::send_reliable_game_message_excluding(int clientNum, const idBitMsg &msg)
	{
	  
	}

	void AsyncServer::master_heartbeat( bool force = false )
	{
	  
	}

	void AsyncServer::drop_client( int clientNum, const char *reason )
	{
	  
	}

	void AsyncServer::update_async_stats_avg()
	{
	  
	}

	void AsyncServer::print_local_server_info()
	{
	}

}