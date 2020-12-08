#include "digital_input_state_reader.h"
#include "../digital_messages.h"
#include "../interface_messages.h"
#include <chrono>
#include <wiringPi.h>

using namespace LabNet::interface::io_board;

DigitalInputStateReader::DigitalInputStateReader(const so_5::mbox_t parent, const so_5::mchain_t reader_box, Logger logger)
    : _parent(parent)
    , _inputs()
    , _reader_box(reader_box)
    , _future_obj(_exit_signal.get_future())
    , _logger(logger)
    , _check_inputs(true)
{
    std::thread read_worker { &DigitalInputStateReader::data_read_thread, this };
    _read_worker = std::move(read_worker);
}

DigitalInputStateReader::~DigitalInputStateReader()
{
    _exit_signal.set_value();
    _read_worker.join();
}

bool DigitalInputStateReader::stop_requested()
{
    // checks if value in future object is available
    if (_future_obj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return false;
    return true;
}

void DigitalInputStateReader::box_msg_handler()
{
    using namespace std::chrono_literals;

    receive(
        from(_reader_box).handle_all().empty_timeout(1ms),
        [&](so_5::mhood_t<DigitalInput> msg) {
            _inputs[msg->pin_l] = std::make_unique<DigitalInput>(*msg);

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
            _check_inputs = false;
        },
        [&](so_5::mhood_t<ContinueInterface> cont) {
            _check_inputs = true;
        });
}

void DigitalInputStateReader::data_read_thread()
{
    int res = 0;
    while (stop_requested() == false)
    {
        if (_check_inputs)
        {
            for (auto& inp : _inputs)
            {
                res = digitalRead(inp.second->pin_h);
                if (res != inp.second->state)
                {
                    inp.second->state = res;
                    if (inp.second->is_inverted)
                        so_5::send<digital_messages::ReturnDigitalInState>(_parent, Interfaces::IO_BOARD, inp.second->pin_l, !res, std::chrono::high_resolution_clock::now());
                    else
                        so_5::send<digital_messages::ReturnDigitalInState>(_parent, Interfaces::IO_BOARD, inp.second->pin_l, res, std::chrono::high_resolution_clock::now());
                }
            }
        }

        box_msg_handler();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}