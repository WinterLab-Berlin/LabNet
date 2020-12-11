#include "rfid_main_actor.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"
#include "../interface_messages.h"
#include "../resources/resources_actor.h"

using namespace LabNet::interface::rfid_board;

RfidBoardMainActor::RfidBoardMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
    , res_box_(ctx.env().create_mbox("res_man"))
{
    for (auto& p : pins_)
    {
        resources::Resource res = resources::WiringToResource(p);
        resources_.push_back(res);
    }
}

RfidBoardMainActor::~RfidBoardMainActor()
{
}

void RfidBoardMainActor::so_evt_start()
{
    _logger->writeInfoEntry("rfid board started");
}

void RfidBoardMainActor::so_evt_finish()
{
    _worker.reset();
    _device.reset();

    if (res_reserved_)
    {
        so_5::send<LabNet::resources::ReleaseResourcesRequest>(res_box_, _self_box, _self_box, resources_, static_cast<uint16_t>(0));
    }

    so_5::send<InterfaceStopped>(_interfaces_manager_box, Interfaces::RFID_BOARD);
    _logger->writeInfoEntry("rfid board finished");
}

void RfidBoardMainActor::so_define_agent()
{
    this >>= init_state;

    init_state
        .event(_self_box,
            [this](const mhood_t<init_interface>& msg) {
                _phase1 = msg->antenna_phase1;
                _phase2 = msg->antenna_phase2;
                _phase_dur = msg->phase_duration;
                _is_inverted = msg->is_inverted;

                so_5::send<resources::ReserveResourcesRequest>(res_box_, _self_box, _self_box, resources_, 1);
            })
        .event(_self_box,
            [this](const mhood_t<resources::ReserveResourcesReply> msg)
            {
                if (msg->result)
                {
                    res_reserved_ = true;
                    _device = std::make_shared<MAXDevice>(_logger, _events_box);
                    _device->set_phase_matrix(_phase1, _phase2, _phase_dur);
                    _device->init(_is_inverted);

                    _worker = std::make_unique<DataReadWorker>(_device);

                    so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::RFID_BOARD, true);

                    this >>= running_state;
                }
                else
                {
                    so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::RFID_BOARD, false);
                }
            });

    running_state
        .event(_self_box,
            [this](const mhood_t<init_interface>& msg) {
                so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::RFID_BOARD, true);
            })
        .event(_self_box,
            [this](const mhood_t<set_phase_matrix>& msg) {
                _phase1 = msg->antenna_phase1;
                _phase2 = msg->antenna_phase2;
                _phase_dur = msg->phase_duration;
                _device->set_phase_matrix(_phase1, _phase2, _phase_dur);
            })
        .event(_self_box,
            [this](mhood_t<PauseInterface>) {
                _worker.reset();

                this >>= paused_state;
            });

    paused_state
        .event(_self_box,
            [this](mhood_t<ContinueInterface>) {
                _worker = std::make_unique<DataReadWorker>(_device);
                this >>= running_state;
            });
}