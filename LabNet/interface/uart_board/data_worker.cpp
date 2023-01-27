#include "data_worker.h"
#include <chrono>
#include <so_5/all.hpp>
#include <LabNetClient.pb.h>

using namespace LabNet::interface::uart_board;

DataWorker::DataWorker(std::shared_ptr<MaxUartBoard> dev, const so_5::mbox_t send_to_box, const so_5::mchain_t receive_box)
    : device_(dev)
    , future_obj_(exit_signal_.get_future())
    , send_to_box_(send_to_box)
    , receive_box_(receive_box)
{
    std::thread worker { &DataWorker::ReadRfidThread, this };
    read_worker_ = std::move(worker);
}

DataWorker::~DataWorker()
{
    exit_signal_.set_value();
    read_worker_.join();

    device_->Stop();
}

bool DataWorker::StopRequested()
{
    // checks if value in future object is available
    if (future_obj_.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return false;
    return true;
}

void DataWorker::ReadRfidThread()
{
    while (StopRequested() == false)
    {
        device_->ReadAllUarts(send_to_box_);

        BoxMsgHandler();
    }
}

void DataWorker::BoxMsgHandler()
{
    using namespace std::chrono_literals;

    receive(
        from(receive_box_).handle_all().empty_timeout(5ms),
        [this](so_5::mhood_t<SetGpioMessage> msg)
        {
            device_->SetGpio(msg->port, msg->pin, msg->state);
        },
        [this](std::shared_ptr<LabNetProt::Client::UartBoardWriteData> data)
        {
            device_->WriteData(data->port(), data->data());
        }
    );
}
