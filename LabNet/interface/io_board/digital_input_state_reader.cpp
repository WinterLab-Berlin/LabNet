#include "digital_input_state_reader.h"
#include "../digital_messages.h"
#include "../interface_messages.h"
#include <chrono>
#include <wiringPi.h>

using namespace LabNet::interface::io_board;

DigitalInputStateReader::DigitalInputStateReader(const so_5::mbox_t parent, const so_5::mchain_t reader_box, log::Logger logger)
    : parent_(parent)
    , inputs_()
    , reader_box_(reader_box)
    , future_obj_(exit_signal_.get_future())
    , logger_(logger)
    , check_inputs_(true)
{
    std::thread read_worker { &DigitalInputStateReader::DataReadThread, this };
    read_worker_ = std::move(read_worker);
}

DigitalInputStateReader::~DigitalInputStateReader()
{
    exit_signal_.set_value();
    read_worker_.join();
}

bool DigitalInputStateReader::StopRequested()
{
    // checks if value in future object is available
    if (future_obj_.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return false;
    return true;
}

void DigitalInputStateReader::BoxMsgHandler()
{
    using namespace std::chrono_literals;

    receive(
        from(reader_box_).handle_all().empty_timeout(1ms),
        [&](so_5::mhood_t<DigitalInput> msg) {
            inputs_[msg->pin_l] = std::make_unique<DigitalInput>(*msg);

            pinMode(msg->pin_h, INPUT);
            switch (msg->resistor_state)
            {
                case Resistor::Off:
                    pullUpDnControl(msg->pin_h, PUD_OFF);
                    break;
                case Resistor::PullDown:
                    pullUpDnControl(msg->pin_h, PUD_DOWN);
                    break;
                case Resistor::PullUp:
                    pullUpDnControl(msg->pin_h, PUD_UP);
                    break;
            }
        },
        [&](so_5::mhood_t<PauseInterface> pause) {
            check_inputs_ = false;
        },
        [&](so_5::mhood_t<ContinueInterface> cont) {
            check_inputs_ = true;
        });
}

void DigitalInputStateReader::DataReadThread()
{
    int res = 0;
    while (StopRequested() == false)
    {
        if (check_inputs_)
        {
            for (auto& inp : inputs_)
            {
                res = digitalRead(inp.second->pin_h);
                if (res != inp.second->state)
                {
                    inp.second->state = res;
                    if (inp.second->is_inverted)
                        so_5::send<digital_messages::ReturnDigitalInState>(parent_, Interfaces::IoBoard, inp.second->pin_l, !res, std::chrono::high_resolution_clock::now());
                    else
                        so_5::send<digital_messages::ReturnDigitalInState>(parent_, Interfaces::IoBoard, inp.second->pin_l, res, std::chrono::high_resolution_clock::now());
                }
            }
        }

        BoxMsgHandler();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}