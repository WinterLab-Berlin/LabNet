#include "uart_board_main_actor.h"
#include "../resources/resources_actor.h"
#include "../interface_messages.h"
#include "../digital_messages.h"
#include <LabNetClient.pb.h>

using namespace LabNet::interface::uart_board;

UartBordMainActor::UartBordMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
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
    
UartBordMainActor::~UartBordMainActor()
{
}

void UartBordMainActor::so_evt_start()
{
    logger_->WriteInfoEntry("uart board started");
}

void UartBordMainActor::so_evt_finish()
{
    worker_.reset();
    device_.reset();

    if (res_reserved_)
    {
        so_5::send<LabNet::resources::ReleaseResourcesRequest>(res_box_, self_box_, self_box_, resources_, static_cast<uint16_t>(0));
    }

    so_5::send<InterfaceStopped>(interfaces_manager_box_, Interfaces::UartBoard);
    logger_->WriteInfoEntry("uart board finished");
}

void UartBordMainActor::so_define_agent()
{
    this >>= init_state_;

    init_state_
        .event(self_box_,
            [this](const mhood_t<InitUartBoard>& msg)
            {
                baud_ = msg->baud;
                is_inverted_ = msg->is_inverted;

                so_5::send<resources::ReserveResourcesRequest>(res_box_, self_box_, self_box_, resources_, 1);
            })
        .event(self_box_,
            [this](const mhood_t<resources::ReserveResourcesReply> msg)
            {
                if (msg->result)
                {
                    res_reserved_ = true;
                    device_ = std::make_shared<MaxUartBoard>(logger_, baud_, is_inverted_);
                    device_->Init();

                    data_worker_box_ = so_environment().create_mchain(so_5::make_unlimited_mchain_params());
                    worker_ = std::make_unique<DataWorker>(device_, events_box_, data_worker_box_);

                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::UartBoard, true);

                    this >>= running_state_;
                }
                else
                {
                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::UartBoard, false);
                }
            });

    running_state_
        .event(self_box_,
            [this](const mhood_t<InitUartBoard>& msg) {
                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::UartBoard, true);
            })
        .event(self_box_,
            [this](mhood_t<PauseInterface>) {
                worker_.reset();

                this >>= paused_state_;
            })
        .event(self_box_,
            [this](const mhood_t<digital_messages::SetDigitalOut>& msg)
            {
                uint32_t port = msg->pin % 32;
                uint32_t pin = msg->pin / 32;

                if(pin < 4)
                {
                    so_5::send<SetGpioMessage>(data_worker_box_, port, pin, msg->state);

                    so_5::send<digital_messages::ReturnDigitalOutState>(msg->mbox, Interfaces::UartBoard, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                }
            }
        )
        .event(self_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartBoardWriteData> data)
            {
                if(data->port() > 0 && data->port() < 33)
                {
                    so_5::send<std::shared_ptr<LabNetProt::Client::UartBoardWriteData>>(data_worker_box_, data);
                }
            }
        );

    paused_state_
        .event(self_box_,
            [this](mhood_t<ContinueInterface>) {
                worker_ = std::make_unique<DataWorker>(device_, events_box_, data_worker_box_);
                this >>= running_state_;
            });
}
