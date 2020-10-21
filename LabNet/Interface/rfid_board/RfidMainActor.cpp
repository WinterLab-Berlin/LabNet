#include "RfidMainActor.h"
#include "../../Network/LabNet.pb.h"
#include "../../Network/LabNetClient.pb.h"
#include "../InterfaceMessages.h"

using namespace rfid_board;

SamMainActor::SamMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
{
}

SamMainActor::~SamMainActor()
{
}

void SamMainActor::so_evt_start()
{
    _logger->writeInfoEntry("rfid board started");
}

void SamMainActor::so_evt_finish()
{
    _worker.reset();
    _device.reset();

    so_5::send<Interface::interface_stopped>(_interfaces_manager_box, Interface::Interfaces::RFID_BOARD);
    _logger->writeInfoEntry("rfid board finished");
}

void SamMainActor::so_define_agent()
{
    this >>= init_state;

    init_state
        .event(_self_box,
            [this](const mhood_t<init_interface>& msg) {
                _phase1 = msg->antenna_phase1;
                _phase2 = msg->antenna_phase2;
                _phase_dur = msg->phase_duration;
                _is_inverted = msg->is_inverted;

                _device = std::make_shared<MAX14830::MAXDevice>(_logger, _events_box);
                _device->set_phase_matrix(_phase1, _phase2, _phase_dur);
                _device->init(_is_inverted);

                _worker = std::make_unique<MAX14830::DataReadWorker>(_device);

                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::RFID_BOARD, true);

                this >>= running_state;
            });

    running_state
        .event(_self_box,
            [this](const mhood_t<init_interface>& msg) {
                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::RFID_BOARD, true);
            })
        .event(_self_box,
            [this](const mhood_t<set_phase_matrix>& msg) {
                _phase1 = msg->antenna_phase1;
                _phase2 = msg->antenna_phase2;
                _phase_dur = msg->phase_duration;
                _device->set_phase_matrix(_phase1, _phase2, _phase_dur);
            })
        .event(_self_box,
            [this](mhood_t<Interface::pause_interface>) {
                _worker.reset();

                this >>= paused_state;
            });

    paused_state
        .event(_self_box,
            [this](mhood_t<Interface::continue_interface>) {
                _worker = std::make_unique<MAX14830::DataReadWorker>(_device);
                this >>= running_state;
            });
}