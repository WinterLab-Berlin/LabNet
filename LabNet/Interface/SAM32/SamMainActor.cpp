#include "SamMainActor.h"
#include "SamMessages.h"
#include "../InitMessages.h"
#include "../InterfaceMessages.h"

SAM::SamMainActor::SamMainActor(context_t ctx, const so_5::mbox_t mbox, Logger logger)
	: so_5::agent_t(ctx)
	, _parentMbox(mbox)
	, _logger(logger)
	, _interfaces(ctx.env().create_mbox("ManageInterfaces"))
{
}

SAM::SamMainActor::~SamMainActor()
{
	_worker.reset();
	_device.reset();
}

void SAM::SamMainActor::so_define_agent()
{
	this >>= wait_for_init;
	
	wait_for_init
		.event([this](mhood_t<init_interface>) {
			so_5::send<Interface::InitMessages::init_sam32_request>(_interfaces, so_direct_mbox());
		})
		.event([this](mhood_t<Interface::InitMessages::can_init_no> msg) {
			so_5::send <init_failed>(_parentMbox);
		})
		.event([this](mhood_t<Interface::InitMessages::can_init_yes> msg) {
			_device = std::make_shared<MAX14830::MAXDevice>(_logger, _parentMbox);
			_device->init();
	
			_worker = std::make_unique<MAX14830::DataReadWorker>(_device);
			
			so_5::send <init_succeed>(_parentMbox);
			
			this >>= running;
		}
	);
	
	running
		.event([this](mhood_t<set_phase_matrix> msg) {
			_device->set_phase_matrix(msg->antenna_phase1, msg->antenna_phase2, msg->phase_duration);
		})
		.event([this](mhood_t<set_signal_inversion> msg) {
			_device->invert(msg->inverted);
		})
		.event([this](mhood_t<Interface::pause_all_interface>) {
			_worker.reset();
			
			this >>= paused;
		})
		.event([this](mhood_t<Interface::reset_all_interface>) {
			_worker.reset();
			_device.reset();
			
			this >>= wait_for_init;
		}
	);
	
	paused
		.event([this](mhood_t<Interface::continue_all_interface>) {
			_worker = std::make_unique<MAX14830::DataReadWorker>(_device);
			this >>= running;
		}
	);
}