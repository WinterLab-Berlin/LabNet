#include "SamMainActor.h"

SAM::SamMainActor::SamMainActor(context_t ctx, const so_5::mbox_t mbox, Logger logger)
	: so_5::agent_t(ctx)
	, _parentMbox(mbox)
	, _logger(logger)
	, _device(logger, so_direct_mbox())
{
	_device.init();
	_worker = std::make_unique<MAX14830::DataReadWorker>(_device);
}

SAM::SamMainActor::~SamMainActor()
{
	
} 

void SAM::SamMainActor::so_define_agent()
{
	so_subscribe_self()
		.event(&SamMainActor::stop_worker_event)
		.event(&SamMainActor::start_worker_event);
}

void SAM::SamMainActor::so_evt_start()
{
	
}

void SAM::SamMainActor::stop_worker_event(const stop_worker& ev)
{
	_worker.reset();
	//_logger->writeInfoEntry("stop worker");
}

void SAM::SamMainActor::start_worker_event(const start_worker& ev)
{
	if (!_worker)
	{
		_worker = std::make_unique<MAX14830::DataReadWorker>(_device);
		//_logger->writeInfoEntry("start worker");
	}
}