#include "SerialPortsManager.h"
#include "../../Network/LabNet.pb.h"
#include "../../Network/LabNetClient.pb.h"

#include <fstream>
#include <string>
#include <unistd.h>
#include <wiringSerial.h>

using namespace uart::private_messages;

uart::SerialPortsManager::SerialPortsManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
{
}

uart::SerialPortsManager::~SerialPortsManager()
{
}

void uart::SerialPortsManager::so_evt_start()
{
    get_raspi_revision();

    _handle_for_port[0] = -1;
    _handle_for_port[1] = -1;
    _handle_for_port[2] = -1;
    _handle_for_port[3] = -1;
    _handle_for_port[4] = -1;
}

void uart::SerialPortsManager::so_evt_finish()
{
    for (auto& handle : _handle_for_port)
        handle.second = -1;

    _ports.clear();
}

void uart::SerialPortsManager::so_define_agent()
{
    this >>= running_state;

    running_state
        .event(_self_box, &SerialPortsManager::init_new_port)
        .event(_self_box,
            [this](const mhood_t<uart::private_messages::port_unexpected_closed>& msg) {
                _logger->writeInfoEntry(string_format("port unexpected closed serial port %d", msg->port_id));

                _ports.erase(msg->port_id);
                _handle_for_port[msg->port_id] = -1;

                so_5::send<Interface::interface_lost>(_interfaces_manager_box, static_cast<Interface::Interfaces>(msg->port_id + 100));
                so_5::send_delayed<try_to_reconnect>(_self_box, std::chrono::seconds(1), msg->port_id, msg->baud);
            })
        .event(_self_box, &SerialPortsManager::try_to_reconnect_event)
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::UartWriteData> data) {
                uint32_t port = data->port() - 100;
                auto it = _ports.find(port);
                if (it != _ports.end())
                {
                    it->second->send_data(data);
                }
            })
        .event(_self_box,
            [this](mhood_t<Interface::pause_interface>) {
                for (auto& port : _ports)
                {
                    port.second->deactivate_send_receive();
                }

                this >>= paused_state;
            })
        .event(_self_box,
            [this](mhood_t<Interface::stop_interface>) {
                so_deregister_agent_coop_normally();
            })
        .event(_self_box,
            [this](const uart::private_messages::send_data_complete& mes) {
                so_5::send<StreamMessages::send_data_complete>(_interfaces_manager_box, static_cast<Interface::Interfaces>(mes.pin + 100));
            })
        .event(_self_box,
            [this](const mhood_t<DigitalMessages::set_digital_out> &msg) {
                uint32_t port = msg->interface - 100;
                auto it = _ports.find(port);
                if (it != _ports.end())
                {
                    it->second->set_digital_out(msg->mbox, msg->pin, msg->state);
                }
            });

     paused_state
        .event(_self_box,
            [this](mhood_t<Interface::stop_interface> msg) {
                so_deregister_agent_coop_normally();
            })
        .event(_self_box,
            [this](mhood_t<Interface::continue_interface> msg) {
                for (auto& port : _ports)
                {
                    port.second->activate_send_receive();
                }

                this >>= running_state;
            });
}

void uart::SerialPortsManager::init_new_port(const init_serial_port& ev)
{
    bool port_init_succes = false;
    int port_id = ev.port_id - 100;

    if (_raspi_revision == R3BV1_2 || _raspi_revision == R3BPV1_3)
    {
        if (port_id > -1 && port_id < 5)
        {
            if (_handle_for_port[port_id] < 0)
            {
                std::string port = port_name_for_id(port_id);
                if (port.size() == 0)
                {
                    _logger->writeInfoEntry(string_format("failed to init uart port %d: does not exist", port_id));
                    return;
                }

                int handle = serialOpen(port.c_str(), ev.baud);
                if (handle < 0)
                {
                    _logger->writeInfoEntry(string_format("failed to init uart port %d: could not open", port_id));
                }
                else
                {
                    so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());

                    _ports[port_id] = std::make_unique<SerialPort>(_self_box, sendToPortBox, _events_box, port_id, handle, ev.baud);
                    _handle_for_port[port_id] = handle;

                    _logger->writeInfoEntry(string_format("open uart port %d", port_id));
                    port_init_succes = true;
                }
            }
            else
            {
                port_init_succes = true;
            }
        }
    }

    so_5::send<Interface::interface_init_result>(_interfaces_manager_box, static_cast<Interface::Interfaces>(port_id + 100), port_init_succes);
}

void uart::SerialPortsManager::try_to_reconnect_event(const try_to_reconnect& ev)
{
    if (ev.port_id > -1 && ev.port_id < 5)
    {
        if (_handle_for_port[ev.port_id] < 0)
        {
            std::string port = port_name_for_id(ev.port_id);
            if (port.size() == 0)
            {
                so_5::send_delayed<try_to_reconnect>(_self_box, std::chrono::seconds(1), ev.port_id, ev.baud);
                return;
            }

            int handle = serialOpen(port.c_str(), ev.baud);
            if (handle < 0)
            {
                so_5::send_delayed<try_to_reconnect>(_self_box, std::chrono::seconds(1), ev.port_id, ev.baud);
            }
            else
            {
                so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());

                _ports[ev.port_id] = std::make_unique<SerialPort>(_self_box, sendToPortBox, _events_box, ev.port_id, handle, ev.baud);
                _handle_for_port[ev.port_id] = handle;

                // reconnected
                so_5::send<Interface::interface_reconnected>(_interfaces_manager_box, static_cast<Interface::Interfaces>(ev.port_id + 100));
            }
        }
    }
}

std::string uart::SerialPortsManager::port_name_for_id(int id)
{
    if (_raspi_revision > 0)
    {
        if (id == 0)
        {
            if (_raspi_revision == R3BPV1_3 || _raspi_revision == R3BV1_2)
            {
                return std::string("/dev/ttyS0");
            }
            else
            {
                return std::string("/dev/ttyAMA0");
            }
        }

        char path[23] = "/sys/class/tty/ttyUSB ";
        char buffer[1024];
        for (int i = 0; i < 4; i++)
        {
            path[21] = i + '0';
            int portId = -1;

            /* befor Raspi3B+ V1.3 the path length from "readlink" was always 84
			 * and it was easy to identify on which USB port the UART converter connected to.
			 **/
            int len = readlink(path, buffer, sizeof(buffer) - 1);
            if (len > 0)
            {
                if (_raspi_revision == R3BPV1_3 || _raspi_revision == R3BV1_2)
                {
                    if (len == 84)
                    {
                        if (buffer[59] == '3')
                            portId = 3;
                        else if (buffer[59] == '2')
                            portId = 4;
                    }
                    else if (len == 94)
                    {
                        if (buffer[69] == '3')
                            portId = 2;
                        else if (buffer[69] == '2')
                            portId = 1;
                    }
                }
                else
                {
                    /* for Raspi 1 until Raspi3 V1.2*/
                    if (len == 84)
                    {
                        if (buffer[59] == '2')
                            portId = 1;
                        else if (buffer[59] == '3')
                            portId = 2;
                        else if (buffer[59] == '4')
                            portId = 3;
                        else if (buffer[59] == '5')
                            portId = 4;
                    }
                }
            }

            if (portId == id)
            {
                std::string portName("/dev/ttyUSB");
                portName += ('0' + i);

                return portName;
            }
        }
    }

    return "";
}

void uart::SerialPortsManager::get_raspi_revision()
{
    using namespace std;

    _raspi_revision = -1;
    string line;
    ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open())
    {
        while (getline(cpuinfo, line))
        {
            if (line.rfind("Revision", 0) == 0)
            {
                int pos = line.rfind(':');
                if (pos != string::npos)
                {
                    string rev = line.substr(pos + 2, (line.size() - pos - 2));
                    std::string::size_type sz = 0;
                    _raspi_revision = std::stoll(rev, &sz, 16);
                }
            }
        }
    }
}