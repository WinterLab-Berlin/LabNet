#include "serial_ports_manager.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"

#include <fstream>
#include <string>
#include <unistd.h>
#include <wiringSerial.h>

using namespace LabNet::interface::uart;

SerialPortsManager::SerialPortsManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
{
}

SerialPortsManager::~SerialPortsManager()
{
}

void SerialPortsManager::so_evt_start()
{
    get_raspi_revision();

    _handle_for_port[0] = -1;
    _handle_for_port[1] = -1;
    _handle_for_port[2] = -1;
    _handle_for_port[3] = -1;
    _handle_for_port[4] = -1;

    if (is_raspi_type_valid())
        _logger->writeInfoEntry("serial ports manager starts on known raspi version");
    else
        _logger->writeInfoEntry("serial ports manager starts on unknown raspi version");
}

void SerialPortsManager::so_evt_finish()
{
    for (auto& handle : _handle_for_port)
        handle.second = -1;

    _ports.clear();
}

void SerialPortsManager::so_define_agent()
{
    this >>= running_state;

    running_state
        .event(_self_box, &SerialPortsManager::init_new_port_event)
        .event(_self_box, &SerialPortsManager::try_to_reconnect_event)
        .event(_self_box,
            [this](const mhood_t<private_messages::PortUnexpectedClosed>& msg) {
                _logger->writeInfoEntry(string_format("port unexpected closed serial port %d", msg->port_id));

                _ports.erase(msg->port_id);
                _handle_for_port[msg->port_id] = -1;

                so_5::send<InterfaceLost>(_interfaces_manager_box, static_cast<Interfaces>(msg->port_id + 100));
                so_5::send_delayed<private_messages::TryToReconnect>(_self_box, std::chrono::seconds(1), msg->port_id, msg->baud);
            })
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::UartWriteData> data) {
                uint32_t port = data->port() - 100;
                auto it = _ports.find(port);
                if (it != _ports.end())
                {
                    it->second->SendData(data);
                }
            })
        .event(_self_box,
            [this](mhood_t<PauseInterface>) {
                for (auto& port : _ports)
                {
                    port.second->DeactivateSendReceive();
                }

                this >>= paused_state;
            })
        .event(_self_box, &SerialPortsManager::stop_interface_event)
        .event(_self_box,
            [this](const private_messages::SendDataComplete& mes) {
                so_5::send<stream_messages::SendDataComplete>(_events_box, static_cast<Interfaces>(mes.pin + 100));
            })
        .event(_self_box,
            [this](const mhood_t<LabNetProt::Client::UartInitDigitalIn> msg) {
                uint32_t port = static_cast<uint32_t>(msg->pin()) - 100;
                auto it = _ports.find(port);
                if (it != _ports.end() && port != 100 && msg->pin() > 0 && msg->pin() < 3)
                {
                    it->second->InitDigitalIn(msg->pin(), msg->is_inverted());
                    so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), true);
                }
                else
                {
                    so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), false);
                }
            })
        .event(_self_box,
            [this](const mhood_t<LabNetProt::Client::UartInitDigitalOut> msg) {
                uint32_t port = static_cast<uint32_t>(msg->pin()) - 100;
                auto it = _ports.find(port);
                if (it != _ports.end() && port != 100 && msg->pin() > 0 && msg->pin() < 3)
                {
                    it->second->InitDigitalOut(msg->pin(), msg->is_inverted());
                    so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), true);
                }
                else
                {
                    so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), false);
                }
            })
        .event(_self_box,
            [this](const mhood_t<digital_messages::SetDigitalOut>& msg) {
                uint32_t port = static_cast<uint32_t>(msg->interface) - 100;
                auto it = _ports.find(port);
                if (it != _ports.end())
                {
                    it->second->SetDigitalOut(msg->mbox, msg->pin, msg->state);
                }
            });

    paused_state
        .event(_self_box, &SerialPortsManager::stop_interface_event)
        .event(_self_box,
            [this](mhood_t<ContinueInterface> msg) {
                for (auto& port : _ports)
                {
                    port.second->ActivateSendReceive();
                }

                this >>= running_state;
            });
}

void SerialPortsManager::stop_interface_event(mhood_t<StopInterface> msg)
{
    _ports.clear();
    _handle_for_port.clear();

    so_5::send<InterfaceStopped>(_interfaces_manager_box, Interfaces::UART0);
    _logger->writeInfoEntry("all uarts stopped");
}

void SerialPortsManager::init_new_port_event(const so_5::mhood_t<InitSerialPort> ev)
{
    bool port_init_succes = false;
    int port_id = static_cast<uint32_t>(ev->port_id) - 100;

    if (is_raspi_type_valid())
    {
        if (port_id > -1 && port_id < 5)
        {
            //auto it = _handle_for_port.find(port_id);
            if (_handle_for_port[port_id] <= 0)
            {
                std::string port = port_name_for_id(port_id);
                if (port.size() == 0)
                {
                    _logger->writeInfoEntry(string_format("failed to init uart port %d: does not exist", port_id));
                }
                else
                {
                    int handle = serialOpen(port.c_str(), ev->baud);
                    if (handle < 0)
                    {
                        _logger->writeInfoEntry(string_format("failed to init uart port %d: could not open", port_id));
                    }
                    else
                    {
                        so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());

                        _ports[port_id] = std::make_unique<SerialPort>(_self_box, sendToPortBox, _events_box, port_id, handle, ev->baud);
                        _handle_for_port[port_id] = handle;

                        _logger->writeInfoEntry(string_format("open uart port %d", port_id));
                        port_init_succes = true;
                    }
                }
            }
            else
            {
                port_init_succes = true;
            }
        }
    }

    so_5::send<InterfaceInitResult>(_interfaces_manager_box, static_cast<Interfaces>(port_id + 100), port_init_succes);
}

void SerialPortsManager::try_to_reconnect_event(const so_5::mhood_t<private_messages::TryToReconnect> ev)
{
    if (ev->port_id > -1 && ev->port_id < 5)
    {
        if (_handle_for_port[ev->port_id] < 0)
        {
            std::string port = port_name_for_id(ev->port_id);
            if (port.size() == 0)
            {
                so_5::send_delayed<private_messages::TryToReconnect>(_self_box, std::chrono::seconds(1), ev->port_id, ev->baud);
                return;
            }

            int handle = serialOpen(port.c_str(), ev->baud);
            if (handle < 0)
            {
                so_5::send_delayed<private_messages::TryToReconnect>(_self_box, std::chrono::seconds(1), ev->port_id, ev->baud);
            }
            else
            {
                so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());

                _ports[ev->port_id] = std::make_unique<SerialPort>(_self_box, sendToPortBox, _events_box, ev->port_id, handle, ev->baud);
                _handle_for_port[ev->port_id] = handle;

                // reconnected
                so_5::send<InterfaceReconnected>(_interfaces_manager_box, static_cast<Interfaces>(ev->port_id + 100));

                _logger->writeInfoEntry(string_format("successfully reconnected to uart port %d", ev->port_id));
            }
        }
    }
}

bool SerialPortsManager::is_raspi_type_valid()
{
    if (_raspi_type > 0)
        return true;
    else
    {
        return false;
    }
}

std::string SerialPortsManager::port_name_for_id(int id)
{
    /* 
    * befor Raspi3 the path length from "readlink" was always 84 bytes long
	* and it was easy to identify on which USB port the UART converter connected to.
    * Now it is a bit more complicated and must be implemented separately for each raspi ype.
	*/

    if (_raspi_type > 0)
    {
        if (id == 0)
        {
            if (_raspi_type > 2)
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

            
            int len = readlink(path, buffer, sizeof(buffer) - 1);
            if (len > 0)
            {
                if (_raspi_type == 3)
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
                else if (_raspi_type < 3)
                {
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

void SerialPortsManager::get_raspi_revision()
{
    using namespace std;

    _raspi_revision = 0;
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

    bool is_new_style = _raspi_revision & 0x800000;
    _raspi_type = 0;
    if (is_new_style)
    {
        _raspi_type = (_raspi_revision & 0xFF0) >> 4;
        if (_raspi_type >= 0 && _raspi_type < 4) // A, B, A+, B+
        {
            _raspi_type = 1;
        }
        else if (_raspi_type == 4) // 2B
        {
            _raspi_type = 2;
        }
        else if (_raspi_type == 8 || _raspi_type == 0xD || _raspi_type == 0xE) // 3B, 3B+, 3A+
        {
            _raspi_type = 3;
        }
        else // Raspi 4, all CMs and Zeros are not supported
        {
            _raspi_type = 0;
        }
    }
    else
    {
        _raspi_type = 1;
    }
}