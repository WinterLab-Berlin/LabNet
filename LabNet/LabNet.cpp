#define BOOST_BIND_NO_PLACEHOLDERS

#include <LoggerFactory.h>
#include <wiringPi.h>
#include <so_5/all.hpp>
#include "Interface/SAM32/SamMainActor.h"

//struct turn_on
//{
//};
//
//struct turn_off
//{
//};
//
//
//class test final : public so_5::agent_t
//{
//public:
//	test(context_t ctx, Logger logger)
//		: so_5::agent_t{ ctx }
//		, m_logger(logger)
//	{
//		
//	}
//
//	void so_define_agent() override
//	{
//		so_default_state()
//			.event(&test::pin_init_success_event)
//			.event(&test::pin_init_failed_event)
//			.event(&test::return_digital_out_state_event)
//			.event(&test::return_digital_in_state_event)
//			.event(&test::turn_on_event)
//			.event(&test::turn_off_event);
//	}
//
//	void so_evt_start() override
//	{
//		so_5::introduce_child_coop( *this,
//			[&](so_5::coop_t & coop) {
//				auto gpio = coop.make_agent< GPIO::GPIOManager >(so_direct_mbox(), m_logger);
//				m_mbox = gpio->so_direct_mbox();
//			});
//		
//		so_5::send<GPIO::init_digital_in>(m_mbox, 1);
//		so_5::send<GPIO::init_digital_out>(m_mbox, 1);
//		
//		so_5::send_delayed<turn_on>(so_direct_mbox(), std::chrono::seconds(1));
//	}
//	
//private:
//	void pin_init_success_event(const GPIO::pin_init_success& ev)
//	{
//		m_logger->writeInfoEntry(string_format("pin %d init success", ev.pin));
//	}
//	void pin_init_failed_event(const GPIO::pin_init_failed& ev)
//	{
//		m_logger->writeInfoEntry(string_format("pin %d init failed", ev.pin));
//	}
//	void return_digital_out_state_event(const GPIO::return_digital_out_state& ev)
//	{
//		m_logger->writeInfoEntry(string_format("dig out %d state %d", ev.pin, ev.state));
//	}
//	void return_digital_in_state_event(const GPIO::return_digital_in_state& ev)
//	{
//		m_logger->writeInfoEntry(string_format("dig in %d state %d", ev.pin, ev.state));
//	}
//	void turn_on_event(const turn_on& ev)
//	{
//		m_logger->writeInfoEntry("send turn on");
//		so_5::send<GPIO::set_digital_out>(m_mbox, 1, 1, so_direct_mbox());	
//		so_5::send_delayed<turn_off>(so_direct_mbox(), std::chrono::seconds(1));
//	}
//	void turn_off_event(const turn_off& ev)
//	{
//		m_logger->writeInfoEntry("send turn off");
//		so_5::send<GPIO::set_digital_out>(m_mbox, 1, 0, so_direct_mbox());	
//		so_5::send_delayed<turn_on>(so_direct_mbox(), std::chrono::seconds(1));
//	}
//	
//	
//	so_5::mbox_t m_mbox;
//	Logger m_logger;
//};

class sam_test final : public so_5::agent_t
{
public:
	sam_test(context_t ctx, Logger logger)
	: so_5::agent_t{ ctx }
	, _logger(logger)
	{
	}
	
	void so_define_agent() override
	{
		so_default_state()
			.event(&sam_test::stop_worker_event)
			.event(&sam_test::start_worker_event);
	}

	void so_evt_start() override
	{
		so_5::introduce_child_coop( *this,
			[&](so_5::coop_t & coop) {
				auto gpio = coop.make_agent< SAM::SamMainActor >(so_direct_mbox(), _logger);
				_mbox = gpio->so_direct_mbox();
			});
		
		so_5::send_delayed<SAM::stop_worker>(so_direct_mbox(), std::chrono::seconds(10));
	}
	
private:
	void stop_worker_event(const SAM::stop_worker& stop)
	{
		so_5::send<SAM::stop_worker>(_mbox);
		so_5::send_delayed<SAM::start_worker>(so_direct_mbox(), std::chrono::seconds(10));
	}
	
	void start_worker_event(const SAM::start_worker& stop)
	{
		so_5::send<SAM::start_worker>(_mbox);
		so_5::send_delayed<SAM::stop_worker>(so_direct_mbox(), std::chrono::seconds(10));
	}
	
	so_5::mbox_t _mbox;
	Logger _logger;
};

int main(int argc, char *argv[])
{
	Logger logger = LoggerFactory::create();
	logger->writeInfoEntry("LabNet starting");
	
	int err = wiringPiSetup();
	if (err == -1)
	{
		logger->writeInfoEntry("GPIO init failed");
		return 0;
	}
	else
	{
		logger->writeInfoEntry("GPIO init success");
	}
	
//	so_5::launch([&](so_5::environment_t & env) {
//		so_5::mbox_t m;
//		env.introduce_coop([&](so_5::coop_t & coop) {
//			env.register_agent_as_coop(env.make_agent<test>(logger));
//		});
//	});

	so_5::launch([&](so_5::environment_t & env) {
		so_5::mbox_t m;
		env.introduce_coop([&](so_5::coop_t & coop) {
			env.register_agent_as_coop(env.make_agent<sam_test>(logger));
		});
	});
	
//	NetworkProxyActor proxy(logger);
//	ConnectionManager connection_manager(logger, proxy);
//	
//	try
//	{
//		Server server(logger, connection_manager, 8080);
//		server.run();
//	} 
//	catch (std::exception& e)
//	{
//		logger->writeFatalEntry(std::string("network server error ") + e.what());
//	}
	
	
	
	return 0;
}