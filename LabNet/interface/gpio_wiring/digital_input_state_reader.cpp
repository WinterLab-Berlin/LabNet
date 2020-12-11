#include "digital_input_state_reader.h"
#include "../digital_messages.h"
#include "../interface_messages.h"
#include <chrono>
#include <wiringPi.h>

using namespace LabNet::interface::gpio_wiring;

DigitalInputStateReader::DigitalInputStateReader(const so_5::mbox_t parent, const so_5::mchain_t reader_box, log::Logger logger)
    : parent_(parent)
    , inputs_()
    , reader_box_(reader_box)
    , future_obj_(exit_signal_.get_future())
    , logger_(logger)
    , check_inputs_(true)
{
    std::thread read_worker { &DigitalInputStateReader::data_read_thread, this };
    read_worker_ = std::move(read_worker);
}

DigitalInputStateReader::~DigitalInputStateReader()
{
    exit_signal_.set_value();
    read_worker_.join();
}

bool DigitalInputStateReader::stop_requested()
{
    // checks if value in future object is available
    if (future_obj_.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return false;
    return true;
}

void DigitalInputStateReader::box_msg_handler()
{
    using namespace std::chrono_literals;

    receive(
        from(reader_box_).handle_all().empty_timeout(1ms),
        [&](so_5::mhood_t<DigitalInput> msg) {
            inputs_[msg->pin] = std::make_shared<DigitalInput>(*msg);

            pinMode(msg->pin, INPUT);
            switch (msg->resistor_state)
            {
            case Resistor::Off:
                pullUpDnControl(msg->pin, PUD_OFF);
                break;
            case Resistor::PullDown:
                pullUpDnControl(msg->pin, PUD_DOWN);
                break;
            case Resistor::PullUp:
                pullUpDnControl(msg->pin, PUD_UP);
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

void DigitalInputStateReader::data_read_thread()
{
    int res = 0;
    while (stop_requested() == false)
    {
        if (check_inputs_)
        {
            for (auto& inp : inputs_)
            {
                res = digitalRead(inp.second->pin);
                if (res != inp.second->state)
                {
                    inp.second->state = res;
                    if (inp.second->is_inverted)
                        so_5::send<digital_messages::ReturnDigitalInState>(parent_, Interfaces::GpioWiring, inp.second->pin, !res, std::chrono::high_resolution_clock::now());
                    else
                        so_5::send<digital_messages::ReturnDigitalInState>(parent_, Interfaces::GpioWiring, inp.second->pin, res, std::chrono::high_resolution_clock::now());
                }
            }
        }

        box_msg_handler();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}