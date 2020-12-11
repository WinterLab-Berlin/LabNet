#include "data_read_worker.h"

namespace LabNet::interface::rfid_board
{
    DataReadWorker::DataReadWorker(std::shared_ptr<MAXDevice> dev)
        : device_(dev)
        , futureObj_(exitSignal_.get_future())
    {
        device_->ResetBuffers();

        std::thread worker { &DataReadWorker::ReadRfidThread, this };
        readWorker_ = std::move(worker);
    }

    DataReadWorker::~DataReadWorker()
    {
        exitSignal_.set_value();
        readWorker_.join();

        device_->Stop();
    }

    bool DataReadWorker::StopRequested()
    {
        // checks if value in future object is available
        if (futureObj_.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return false;
        return true;
    }

    void DataReadWorker::ReadRfidThread()
    {
        while (StopRequested() == false)
        {
            device_->ReadAllAndSetAntenna();

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};