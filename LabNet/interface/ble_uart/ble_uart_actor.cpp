#include "ble_uart_actor.h"
#include <chrono>
#include "../interface_messages.h"
#include "../stream_messages.h"

using namespace LabNet::interface::ble_uart;
using namespace std::chrono_literals;

struct TryReconnectMsg
{
};

BleUartActor::BleUartActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t parent, const so_5::mbox_t stream_data_box, std::string device, const char id, log::Logger logger)
    : so_5::agent_t(ctx)
    , self_box_(self_box)
    , parent_(parent)
    , device_(device)
    , id_(id)
    , stream_data_box_(stream_data_box)
    , logger_(logger)
    , connection_(device, logger, self_box)
{
    
}

BleUartActor::~BleUartActor()
{
    
}

void BleUartActor::so_evt_finish()
{
    connection_.Terminate();
}

void BleUartActor::so_evt_start()
{
    connection_trials_ = 0;
    connection_.Connect();
}

void BleUartActor::so_define_agent()
{
    this >>= init_state_;

    init_state_
        .event(self_box_, [this](const mhood_t<ConnectedMsg>& mes)
            {
                this >>= running_state_;
                send_data_ = true;
                so_5::send<InterfaceInitResult>(parent_, Interfaces::BleUart, true); })
        .event(self_box_, [this](const mhood_t<DisconnectedMsg>& mes)
            {
                connection_trials_++;
                if(connection_trials_ < 5)
                    so_5::send_delayed<TryReconnectMsg>(self_box_, 1s);
                else
                    so_5::send<InterfaceInitResult>(parent_, Interfaces::BleUart, false); })
        .event(self_box_, [this](const mhood_t<TryReconnectMsg>& mes)
            {
                connection_.Connect();
            });

    running_state_
        .event(self_box_, [this](mhood_t<PauseInterface>)
            {
                send_data_ = false;
            })
        .event(self_box_, [this](mhood_t<ContinueInterface>)
            {
                send_data_ = true;
            })
        .event(self_box_, [this](const mhood_t<DisconnectedMsg>& mes)
            {
                so_5::send_delayed<TryReconnectMsg>(self_box_, 1s);
                so_5::send<InterfaceLost>(parent_, Interfaces::BleUart);
                this >>= disconnected_state_;
            })
        .event(self_box_, [this](const mhood_t<stream_messages::NewDataFromPort>& mes)
        {
            if(send_data_)
            {
                so_5::send<stream_messages::NewDataFromPort>(stream_data_box_, Interfaces::BleUart, id_, mes->data, mes->time);
            }
        });

    disconnected_state_
        .event(self_box_, [this](const mhood_t<ConnectedMsg>& mes)
            {
                this >>= running_state_;
                so_5::send<InterfaceReconnected>(parent_, Interfaces::BleUart);
            })
        .event(self_box_, [this](const mhood_t<DisconnectedMsg>& mes)
            {
                so_5::send_delayed<TryReconnectMsg>(self_box_, 1s);
            })
        .event(self_box_, [this](const mhood_t<TryReconnectMsg>& mes)
            {
                connection_.Connect();
            });
}