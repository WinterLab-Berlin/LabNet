#include "Log/easylogging++.h"
//#include "Network/Server.h"
#include <so_5/all.hpp>

INITIALIZE_EASYLOGGINGPP

//void connect_handler()
//{
//	std::cout << "connected" << std::endl;
//}
//
//void disconnect_handler()
//{
//	std::cout << "disconnected" << std::endl;
//}
//
//void data_handler(std::shared_ptr<std::vector<char>> data)
//{
//	std::cout << "data: " << data->data() << std::endl;
//}

// Types of signals for the agents.
struct msg_ping final : public so_5::signal_t {}
;
struct msg_pong final : public so_5::signal_t {};

// Class of pinger agent.
class a_pinger_t final : public so_5::agent_t
{
public:
	a_pinger_t(context_t ctx, so_5::mbox_t mbox, int pings_to_send)
		:	so_5::agent_t{ ctx }
	, m_mbox{ std::move(mbox) }
	, m_pings_left{ pings_to_send }
	{}

	void so_define_agent() override
	{
		so_subscribe(m_mbox).event(&a_pinger_t::evt_pong);
	}

	void so_evt_start() override
	{
		send_ping();
	}

private:
	const so_5::mbox_t m_mbox;

	int m_pings_left;

	void evt_pong(mhood_t< msg_pong >)
	{
		if (m_pings_left > 0)
		{
			
			send_ping();
		}
		else
			so_environment().stop();
	}

	void send_ping()
	{
		//LOG(INFO) << "ping";
		so_5::send< msg_ping >(m_mbox);
		--m_pings_left;
	}
}
;

class a_ponger_t final : public so_5::agent_t
{
public:
	a_ponger_t(context_t ctx, const so_5::mbox_t & mbox)
		: so_5::agent_t(std::move(ctx))
	{
		so_subscribe(mbox).event(
			[mbox](mhood_t<msg_ping>) {
			//LOG(INFO) << "pong";
			so_5::send< msg_pong >(mbox);
		});
	}
};
	
int main(int argc, char *argv[])
{
	//LOG(INFO) << "starting";
	
//	ConnectionManager connection_manager;
//	
//	connection_manager.add_connect_handler(&connect_handler);
//	connection_manager.add_disconnect_handler(&disconnect_handler);
//	connection_manager.add_data_received_handler(&data_handler);
//	
//	try
//	{
//		Server server(connection_manager, 8080);
//		server.run();
//	} 
//	catch (std::exception& e)
//	{
//		LOG(FATAL) << "server start error " << e.what();
//	}
	
	try
	{
		LOG(INFO) << "start";
		so_5::launch([](so_5::environment_t & env) {
			env.introduce_coop([&env](so_5::coop_t & coop) {
				// Mbox for agent's interaction.
				auto mbox = env.create_mbox();

				// Pinger.
				coop.make_agent< a_pinger_t >(mbox, 100000);

				// Ponger agent.
				coop.make_agent< a_ponger_t >(std::cref(mbox));
			});
		});
		
		LOG(INFO) << "stop";

		return 0;
	}
	catch(const std::exception & x)
	{
		std::cerr << "*** Exception caught: " << x.what() << std::endl;
	}
	
	return 0;
}