#pragma once

#include <so_5/all.hpp>
#include <thread>
#include <future>
#include <vector>
#include "../../network/LabNetClient.pb.h"

namespace LabNet::interface::uart
{
    class SerialPort final
    {
    public:
        SerialPort(const so_5::mbox_t parent, const so_5::mchain_t send_to_port_box, const so_5::mbox_t _stream_data_box, const int port_id, const int port_handler, const int baud);
        ~SerialPort();

        void SendData(std::shared_ptr<LabNetProt::Client::UartWriteData> data);
        void ActivateSendReceive() { _is_active = true; };
        void DeactivateSendReceive() { _is_active = false; };
        void SetDigitalOut(so_5::mbox_t report, char pin, bool state);

    private:
        void DataSendThread(so_5::mchain_t ch);

        bool StopRequested();

        void DataReadThread();

        const int kEn_uart0 = 2;
        const int _port_id;
        const int _port_handler;
        const int _baud;
        const so_5::mchain_t _send_to_port_box;
        const so_5::mbox_t _parent;
        const so_5::mbox_t _stream_data_box;
        bool _is_active;
        bool _is_pin_inverted;
        std::thread _send_worker;
        std::thread _read_worker;
        std::promise<void> _exit_signal;
        std::future<void> _future_obj;
    };
};