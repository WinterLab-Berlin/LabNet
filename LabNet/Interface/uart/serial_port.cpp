#include "serial_port.h"
#include "../digital_messages.h"
#include "../stream_messages.h"
#include "private_messages.h"
#include <chrono>
#include <sys/ioctl.h>
#include <wiringPi.h>
#include <wiringSerial.h>

namespace LabNet::interface::uart
{
    SerialPort::SerialPort(const so_5::mbox_t parent, const so_5::mchain_t send_to_port_box, const so_5::mbox_t stream_data_box, const uint32_t port_id, const int32_t port_handler, const uint32_t baud)
        : parent_(parent)
        , port_id_(port_id)
        , baud_(baud)
        , send_to_port_box_(send_to_port_box)
        , stream_data_box_(stream_data_box)
        , port_handler_(port_handler)
        , future_obj_(exit_signal_.get_future())
        , is_active_(true)
        , is_pin_inverted_(false)
    {
        std::thread worker {
            &SerialPort::WorkerThread,
            this,
        };
        worker_thread_ = std::move(worker);
    }

    SerialPort::~SerialPort()
    {
        exit_signal_.set_value();
        send_to_port_box_->close(so_5::mchain_props::close_mode_t::drop_content);
        worker_thread_.join();
    }

    void SerialPort::SendData(std::shared_ptr<LabNetProt::Client::UartWriteData> data)
    {
        so_5::send<std::shared_ptr<LabNetProt::Client::UartWriteData>>(send_to_port_box_, data);
    }

    void SerialPort::InitDigitalIn(uint8_t pin, bool is_inverted)
    {
        so_5::send<DigitalInput>(send_to_port_box_, pin, is_inverted);
    }

    void SerialPort::InitDigitalOut(uint8_t pin, bool is_inverted)
    {
        so_5::send<DigitalOutput>(send_to_port_box_, pin, is_inverted);
    }

    void SerialPort::SetDigitalOut(so_5::mbox_t report, uint8_t pin, bool state)
    {
        so_5::send<private_messages::SetDigitalOut>(send_to_port_box_, report, pin, state);
    }

    void SerialPort::BoxMsgHandler()
    {
        using namespace std::chrono_literals;

        receive(
            from(send_to_port_box_).handle_all().empty_timeout(1ms),
            [this](std::shared_ptr<LabNetProt::Client::UartWriteData> data) {
                if (is_active_)
                {
                    for (int i = 0; i < data->data().size(); i++)
                    {
                        serialPutchar(port_handler_, data->data()[i]);
                    }

                    so_5::send<private_messages::SendDataComplete>(parent_, port_id_);
                }
            },
            [this](const so_5::mhood_t<DigitalInput> msg) {
                if (port_id_ != 100)
                {
                    auto it = inputs_.find(msg->pin);
                    if (it == inputs_.end())
                    {
                        inputs_[msg->pin] = std::make_unique<DigitalInput>(msg->pin, msg->is_inverted);
                    }
                    else
                    {
                        it->second->is_inverted = msg->is_inverted;
                        it->second->state = 2;
                    }
                }
            },
            [this](const so_5::mhood_t<DigitalOutput> msg) {
                if (port_id_ != 100 && msg->pin > 0 && msg->pin < 3)
                {
                    auto it = outputs_.find(msg->pin);
                    if (it == outputs_.end())
                    {
                        outputs_[msg->pin] = std::make_unique<DigitalOutput>(msg->pin, msg->is_inverted);
                    }
                    else
                    {
                        it->second->is_inverted = msg->is_inverted;
                    }

                    // set to low
                    int32_t line = 0;
                    if (msg->pin == 1)
                        line = TIOCM_DTR;
                    else if (msg->pin == 2)
                        line = TIOCM_RTS;

                    bool state = false ^ msg->is_inverted;
                    if (line != 0)
                    {
                        int rts = 0;
                        ioctl(port_handler_, TIOCMGET, &rts);
                        if (state)
                            rts &= ~line;
                        else
                            rts |= ~line;
                        ioctl(port_handler_, TIOCMSET, &rts);
                    }
                }
            },
            [this](const so_5::mhood_t<private_messages::SetDigitalOut> msg) {
                auto it = outputs_.find(msg->pin);
                if (it != outputs_.end())
                {
                    int32_t line = 0;
                    if (msg->pin == 1)
                        line = TIOCM_DTR;
                    else if (msg->pin == 2)
                        line = TIOCM_RTS;

                    bool state = msg->state ^ it->second->is_inverted;
                    if (line != 0)
                    {
                        int rts = 0;
                        ioctl(port_handler_, TIOCMGET, &rts);
                        if (state)
                            rts &= ~line;
                        else
                            rts |= ~line;
                        ioctl(port_handler_, TIOCMSET, &rts);

                        so_5::send<digital_messages::ReturnDigitalOutState>(msg->report_box, static_cast<Interfaces>(port_id_ + 100), msg->pin, state, std::chrono::high_resolution_clock::now());
                    }
                    else
                    {
                        so_5::send<digital_messages::InvalidDigitalOutPin>(msg->report_box, static_cast<Interfaces>(port_id_ + 100), msg->pin);
                    }
                }
                else
                {
                    so_5::send<digital_messages::InvalidDigitalOutPin>(msg->report_box, static_cast<Interfaces>(port_id_ + 100), msg->pin);
                }
            });
    }

    bool SerialPort::StopRequested()
    {
        // checks if value in future object is available
        if (future_obj_.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return false;
        return true;
    }

    void SerialPort::WorkerThread()
    {
        while (StopRequested() == false)
        {
            if (is_active_)
            {
                int c = serialDataAvail(port_handler_);

                if (c > 0)
                {
                    std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
                    do
                    {
                        c = serialGetchar(port_handler_);
                        data->push_back(c);
                    } while (serialDataAvail(port_handler_));

                    if (c < 0)
                    {
                        so_5::send<private_messages::PortUnexpectedClosed>(parent_, port_id_);

                        break;
                    }

                    so_5::send<stream_messages::NewDataFromPort>(stream_data_box_, static_cast<Interfaces>(port_id_ + 100), 0, data, std::chrono::high_resolution_clock::now());
                }
                else if (c < 0)
                {
                    so_5::send<private_messages::PortUnexpectedClosed>(parent_, port_id_, baud_);

                    break;
                }

                for (auto& inp : inputs_)
                {
                    int32_t line = 0;
                    if (inp.second->pin == 1)
                    {
                        line = TIOCM_CTS;
                    }
                    else if (inp.second->pin == 2)
                    {
                        line = TIOCM_DSR;
                    }

                    if (line != 0)
                    {
                        int rts = 0;
                        ioctl(port_handler_, TIOCMGET, &rts);

                        int32_t state = rts & line;
                        if (state != inp.second->state)
                        {
                            inp.second->state = state;

                            bool rep = inp.second->state ^ inp.second->is_inverted;
                            so_5::send<digital_messages::ReturnDigitalInState>(stream_data_box_, static_cast<Interfaces>(port_id_ + 100), inp.second->pin, rep, std::chrono::high_resolution_clock::now());
                        }
                    }
                }
            }

            //std::this_thread::sleep_for(std::chrono::milliseconds(1));
            BoxMsgHandler();
        }
    }
};