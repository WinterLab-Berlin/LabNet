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
    SerialPort::SerialPort(const so_5::mbox_t parent, const so_5::mchain_t send_to_port_box, const so_5::mbox_t stream_data_box, const int port_id, const int port_handler, const int baud)
        : _parent(parent)
        , _port_id(port_id)
        , _baud(baud)
        , _send_to_port_box(send_to_port_box)
        , _stream_data_box(stream_data_box)
        , _port_handler(port_handler)
        , _future_obj(_exit_signal.get_future())
        , _is_active(true)
        , _is_pin_inverted(false)
    {
        if (_port_id == 0)
        {
            pinMode(kEn_uart0, OUTPUT);
            digitalWrite(kEn_uart0, 0);
        }

        std::thread send_worker { &SerialPort::DataSendThread, this, _send_to_port_box };
        _send_worker = std::move(send_worker);

        std::thread read_worker { &SerialPort::DataReadThread, this };
        _read_worker = std::move(read_worker);
    }

    SerialPort::~SerialPort()
    {
        _exit_signal.set_value();
        _send_to_port_box->close(so_5::mchain_props::close_mode_t::drop_content);
        _send_worker.join();
        _read_worker.join();
    }

    void SerialPort::SendData(std::shared_ptr<LabNetProt::Client::UartWriteData> data)
    {
        so_5::send<std::shared_ptr<LabNetProt::Client::UartWriteData>>(_send_to_port_box, data);
    }

    void SerialPort::DataSendThread(so_5::mchain_t ch)
    {
        // write data messages until mchain will be closed.
        receive(from(ch).handle_all(),
            [&](std::shared_ptr<LabNetProt::Client::UartWriteData> data) {
                if (_is_active)
                {
                    for (int i = 0; i < data->data().size(); i++)
                    {
                        serialPutchar(_port_handler, data->data()[i]);
                    }

                    so_5::send<private_messages::SendDataComplete>(_parent, _port_id);
                }
            });
    }

    bool SerialPort::StopRequested()
    {
        // checks if value in future object is available
        if (_future_obj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return false;
        return true;
    }

    void SerialPort::DataReadThread()
    {
        while (StopRequested() == false)
        {
            if (_is_active)
            {
                int c = serialDataAvail(_port_handler);

                if (c > 0)
                {
                    std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
                    do
                    {
                        c = serialGetchar(_port_handler);
                        data->push_back(c);
                    } while (serialDataAvail(_port_handler));

                    if (c < 0)
                    {
                        so_5::send<private_messages::PortUnexpectedClosed>(_parent, _port_id);

                        break;
                    }

                    so_5::send<stream_messages::NewDataFromPort>(_stream_data_box, static_cast<Interfaces>(_port_id + 100), 0, data, std::chrono::high_resolution_clock::now());
                }
                else if (c < 0)
                {
                    so_5::send<private_messages::PortUnexpectedClosed>(_parent, _port_id, _baud);

                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void SerialPort::SetDigitalOut(so_5::mbox_t report, char pin, bool state)
    {
        if (pin == 1)
        {
            if (_port_id == 0)
            {
                digitalWrite(kEn_uart0, state);
            }
            else
            {
                int rts = 0;
                if (state)
                {
                    if (ioctl(_port_handler, TIOCMGET, &rts) == -1)
                    {
                        // error
                    }

                    rts &= ~TIOCM_RTS;

                    if (ioctl(_port_handler, TIOCMSET, &rts) == -1)
                    {
                        // error
                    }
                }
                else
                {
                    if (ioctl(_port_handler, TIOCMGET, &rts) != -1)
                    {
                        // error
                    }

                    rts |= TIOCM_RTS;
                    if (ioctl(_port_handler, TIOCMSET, &rts) == -1)
                    {
                        // error
                    }
                }
            }

            //_logger->writeInfoEntry(string_format("gpio set %d", msg->state));
            so_5::send<digital_messages::ReturnDigitalOutState>(report, static_cast<Interfaces>(_port_id + 100), pin, state, std::chrono::high_resolution_clock::now());
        }
        else
        {
            so_5::send<digital_messages::InvalidDigitalOutPin>(report, static_cast<Interfaces>(_port_id + 100), pin);
        }
    }
};