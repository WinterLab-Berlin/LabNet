#include "rfid_main_actor.h"
#include <LabNet.pb.h>
#include <LabNetClient.pb.h>
#include "../interface_messages.h"
#include "../resources/resources_actor.h"

using namespace LabNet::interface::rfid_board;

RfidBoardMainActor::RfidBoardMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
    : so_5::agent_t(ctx)
    , self_box_(self_box)
    , interfaces_manager_box_(interfaces_manager_box)
    , events_box_(events_box)
    , logger_(logger)
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
    logger_->WriteInfoEntry("rfid board started");
}

void RfidBoardMainActor::so_evt_finish()
{
    worker_.reset();
    device_.reset();

    if (res_reserved_)
    {
        so_5::send<LabNet::resources::ReleaseResourcesRequest>(res_box_, self_box_, self_box_, resources_, static_cast<uint16_t>(0));
    }

    so_5::send<InterfaceStopped>(interfaces_manager_box_, Interfaces::RfidBoard);
    logger_->WriteInfoEntry("rfid board finished");
}

void RfidBoardMainActor::so_define_agent()
{
    this >>= init_state_;

    init_state_
        .event(self_box_,
            [this](const mhood_t<InitRfidBoard>& msg) {
                phase1_ = msg->antenna_phase1;
                phase2_ = msg->antenna_phase2;
                phase_dur_ = msg->phase_duration;
                is_inverted_ = msg->is_inverted;

                so_5::send<resources::ReserveResourcesRequest>(res_box_, self_box_, self_box_, resources_, 1);
            })
        .event(self_box_,
            [this](const mhood_t<resources::ReserveResourcesReply> msg)
            {
                if (msg->result)
                {
                    res_reserved_ = true;
                    device_ = std::make_shared<MAXDevice>(logger_, events_box_);
                    device_->SetPhaseMatrix(phase1_, phase2_, phase_dur_);
                    device_->Init(is_inverted_);

                    worker_ = std::make_unique<DataReadWorker>(device_);

                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::RfidBoard, true);

                    this >>= running_state_;
                }
                else
                {
                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::RfidBoard, false);
                }
            });

    running_state_
        .event(self_box_,
            [this](const mhood_t<InitRfidBoard>& msg) {
                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::RfidBoard, true);
            })
        .event(self_box_,
            [this](const mhood_t<SetPhaseMatrix>& msg) {
                phase1_ = msg->antenna_phase1;
                phase2_ = msg->antenna_phase2;
                phase_dur_ = msg->phase_duration;
                device_->SetPhaseMatrix(phase1_, phase2_, phase_dur_);
            })
        .event(self_box_,
            [this](mhood_t<PauseInterface>) {
                worker_.reset();

                this >>= paused_state_;
            });

    paused_state_
        .event(self_box_,
            [this](mhood_t<ContinueInterface>) {
                worker_ = std::make_unique<DataReadWorker>(device_);
                this >>= running_state_;
            });
}
