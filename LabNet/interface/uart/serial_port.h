#pragma once

#include <so_5/all.hpp>
#include <thread>
#include <future>
#include <vector>
#include <map>
#include <LabNetClient.pb.h>
#include "digital_input.h"
#include "digital_output.h"

namespace LabNet::interface::uart
{
    class SerialPort final
    {
    public:
        SerialPort(const so_5::mbox_t parent, const so_5::mchain_t send_to_port_box, const so_5::mbox_t stream_data_box, const uint32_t port_id, const int32_t port_handler, const uint32_t baud);
        ~SerialPort();

        void SetDigitalOut(so_5::mbox_t report, uint8_t pin, bool state);
        void SendData(std::shared_ptr<LabNetProt::Client::UartWriteData> data);
        void InitDigitalIn(uint8_t pin, bool is_inverted);
        void InitDigitalOut(uint8_t pin, bool is_inverted);

        void ActivateSendReceive() { is_active_ = true; };
        void DeactivateSendReceive() { is_active_ = false; };

    private:
        bool StopRequested();

        void WorkerThread();
        void BoxMsgHandler();

        const uint32_t port_id_;
        const int32_t port_handler_;
        const uint32_t baud_;
        const so_5::mchain_t send_to_port_box_;
        const so_5::mbox_t parent_;
        const so_5::mbox_t stream_data_box_;
        bool is_active_;
        bool is_pin_inverted_;
        std::thread worker_thread_;
        std::promise<void> exit_signal_;
        std::future<void> future_obj_;

        std::map<uint8_t, std::unique_ptr<DigitalInput>> inputs_;
        std::map<uint8_t, std::unique_ptr<DigitalOutput>> outputs_;
    };
};
