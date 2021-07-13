#include "serial_ports_manager.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"

#include <fstream>
#include <string>
#include <unistd.h>
#include <wiringSerial.h>

using namespace LabNet::interface::uart;

SerialPortsManager::SerialPortsManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
    : so_5::agent_t(ctx)
    , self_box_(self_box)
    , interfaces_manager_box_(interfaces_manager_box)
    , events_box_(events_box)
    , logger_(logger)
{
}

SerialPortsManager::~SerialPortsManager()
{
}

void SerialPortsManager::so_evt_start()
{
    GetRaspiRevision();

    handle_for_port_[0] = -1;
    handle_for_port_[1] = -1;
    handle_for_port_[2] = -1;
    handle_for_port_[3] = -1;
    handle_for_port_[4] = -1;

    if (IsRaspiTypeValid())
        logger_->WriteInfoEntry("serial ports manager starts on known raspi version");
    else
        logger_->WriteInfoEntry("serial ports manager starts on unknown raspi version");
}

void SerialPortsManager::so_evt_finish()
{
    for (auto& handle : handle_for_port_)
        handle.second = -1;

    ports_.clear();
}

void SerialPortsManager::so_define_agent()
{
    this >>= running_state_;

    running_state_
        .event(self_box_, &SerialPortsManager::InitNewPortEvent)
        .event(self_box_, &SerialPortsManager::TryToReconnectEvent)
        .event(self_box_,
            [this](const mhood_t<private_messages::PortUnexpectedClosed>& msg) {
                logger_->WriteInfoEntry(log::StringFormat("port unexpected closed serial port %d", msg->port_id));

                ports_.erase(msg->port_id);
                handle_for_port_[msg->port_id] = -1;

                so_5::send<InterfaceLost>(interfaces_manager_box_, static_cast<Interfaces>(msg->port_id + 100));
                so_5::send_delayed<private_messages::TryToReconnect>(self_box_, std::chrono::seconds(1), msg->port_id, msg->baud);
            })
        .event(self_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartWriteData> data) {
                uint32_t port = data->port() - 100;
                auto it = ports_.find(port);
                if (it != ports_.end())
                {
                    it->second->SendData(data);
                }
            })
        .event(self_box_,
            [this](mhood_t<PauseInterface>) {
                for (auto& port : ports_)
                {
                    port.second->DeactivateSendReceive();
                }

                this >>= paused_state_;
            })
        .event(self_box_, &SerialPortsManager::StopInterfaceEvent)
        .event(self_box_,
            [this](const private_messages::SendDataComplete& mes) {
                so_5::send<stream_messages::SendDataComplete>(events_box_, static_cast<Interfaces>(mes.pin + 100));
            })
        .event(self_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartInitDigitalIn> msg) {
                uint32_t port = static_cast<uint32_t>(msg->port()) - 100;
                auto it = ports_.find(port);
                if (it != ports_.end() && port != 100 && msg->pin() > 0 && msg->pin() < 3)
                {
                    it->second->InitDigitalIn(msg->pin(), msg->is_inverted());
                    so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), true);
                }
                else
                {
                    so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), false);
                }
            })
        .event(self_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartInitDigitalOut> msg) {
                uint32_t port = static_cast<uint32_t>(msg->port()) - 100;
                auto it = ports_.find(port);
                if (it != ports_.end() && port != 100 && msg->pin() > 0 && msg->pin() < 3)
                {
                    it->second->InitDigitalOut(msg->pin(), msg->is_inverted());
                    so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), true);
                }
                else
                {
                    so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, static_cast<Interfaces>(msg->pin() + 100), msg->pin(), false);
                }
            })
        .event(self_box_,
            [this](const mhood_t<digital_messages::SetDigitalOut>& msg) {
                uint32_t port = static_cast<uint32_t>(msg->interface) - 100;
                auto it = ports_.find(port);
                if (it != ports_.end())
                {
                    it->second->SetDigitalOut(msg->mbox, msg->pin, msg->state);
                }
            });

    paused_state_
        .event(self_box_, &SerialPortsManager::StopInterfaceEvent)
        .event(self_box_,
            [this](mhood_t<ContinueInterface> msg) {
                for (auto& port : ports_)
                {
                    port.second->ActivateSendReceive();
                }

                this >>= running_state_;
            });
}

void SerialPortsManager::StopInterfaceEvent(mhood_t<StopInterface> msg)
{
    ports_.clear();
    handle_for_port_.clear();

    so_5::send<InterfaceStopped>(interfaces_manager_box_, Interfaces::Uart0);
    logger_->WriteInfoEntry("all uarts stopped");
}

void SerialPortsManager::InitNewPortEvent(const so_5::mhood_t<InitSerialPort> ev)
{
    bool port_init_succes = false;
    int port_id = static_cast<uint32_t>(ev->port_id) - 100;

    if (IsRaspiTypeValid())
    {
        if (port_id > -1 && port_id < 5)
        {
            //auto it = handle_for_port_.find(port_id);
            if (handle_for_port_[port_id] <= 0)
            {
                std::string port = PortNameForId(port_id);
                if (port.size() == 0)
                {
                    logger_->WriteInfoEntry(log::StringFormat("failed to init uart port %d: does not exist", port_id));
                }
                else
                {
                    int handle = serialOpen(port.c_str(), ev->baud);
                    if (handle < 0)
                    {
                        logger_->WriteInfoEntry(log::StringFormat("failed to init uart port %d: could not open", port_id));
                    }
                    else
                    {
                        so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());

                        ports_[port_id] = std::make_unique<SerialPort>(self_box_, sendToPortBox, events_box_, port_id, handle, ev->baud);
                        handle_for_port_[port_id] = handle;

                        logger_->WriteInfoEntry(log::StringFormat("open uart port %d", port_id));
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

    so_5::send<InterfaceInitResult>(interfaces_manager_box_, static_cast<Interfaces>(port_id + 100), port_init_succes);
}

void SerialPortsManager::TryToReconnectEvent(const so_5::mhood_t<private_messages::TryToReconnect> ev)
{
    if (ev->port_id > -1 && ev->port_id < 5)
    {
        if (handle_for_port_[ev->port_id] < 0)
        {
            std::string port = PortNameForId(ev->port_id);
            if (port.size() == 0)
            {
                so_5::send_delayed<private_messages::TryToReconnect>(self_box_, std::chrono::seconds(1), ev->port_id, ev->baud);
                return;
            }

            int handle = serialOpen(port.c_str(), ev->baud);
            if (handle < 0)
            {
                so_5::send_delayed<private_messages::TryToReconnect>(self_box_, std::chrono::seconds(1), ev->port_id, ev->baud);
            }
            else
            {
                so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());

                ports_[ev->port_id] = std::make_unique<SerialPort>(self_box_, sendToPortBox, events_box_, ev->port_id, handle, ev->baud);
                handle_for_port_[ev->port_id] = handle;

                // reconnected
                so_5::send<InterfaceReconnected>(interfaces_manager_box_, static_cast<Interfaces>(ev->port_id + 100));

                logger_->WriteInfoEntry(log::StringFormat("successfully reconnected to uart port %d", ev->port_id));
            }
        }
    }
}

bool SerialPortsManager::IsRaspiTypeValid()
{
    if (raspi_type_ > 0)
        return true;
    else
    {
        return false;
    }
}

std::string SerialPortsManager::PortNameForId(int id)
{
    /* 
    * befor Raspi3 the path length from "readlink" was always 84 bytes long
	* and it was easy to identify on which USB port the UART converter connected to.
    * Now it is a bit more complicated and must be implemented separately for each raspi ype.
	*/

    if (raspi_type_ > 0)
    {
        if (id == 0)
        {
            if (raspi_type_ > 2)
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
                if (raspi_type_ == 3)
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
                else if (raspi_type_ < 3)
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

void SerialPortsManager::GetRaspiRevision()
{
    using namespace std;

    raspi_revision_ = 0;
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
                    raspi_revision_ = std::stoll(rev, &sz, 16);
                }
            }
        }
    }

    bool is_new_style = raspi_revision_ & 0x800000;
    raspi_type_ = 0;
    if (is_new_style)
    {
        raspi_type_ = (raspi_revision_ & 0xFF0) >> 4;
        if (raspi_type_ >= 0 && raspi_type_ < 4) // A, B, A+, B+
        {
            raspi_type_ = 1;
        }
        else if (raspi_type_ == 4) // 2B
        {
            raspi_type_ = 2;
        }
        else if (raspi_type_ == 8 || raspi_type_ == 0xD || raspi_type_ == 0xE) // 3B, 3B+, 3A+
        {
            raspi_type_ = 3;
        }
        else // Raspi 4, all CMs and Zeros are not supported
        {
            raspi_type_ = 0;
        }
    }
    else
    {
        raspi_type_ = 1;
    }
}