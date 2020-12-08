#include "data_read_worker.h"

namespace LabNet::interface::rfid_board
{
    DataReadWorker::DataReadWorker(std::shared_ptr<MAXDevice> dev)
        : _device(dev)
        , _futureObj(_exitSignal.get_future())
    {
        _device->reset_buffers();

        std::thread worker { &DataReadWorker::read_rfid_thread, this };
        _readWorker = std::move(worker);
    }

    DataReadWorker::~DataReadWorker()
    {
        _exitSignal.set_value();
        _readWorker.join();

        _device->stop();
    }

    bool DataReadWorker::stop_requested()
    {
        // checks if value in future object is available
        if (_futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return false;
        return true;
    }

    void DataReadWorker::read_rfid_thread()
    {
        while (stop_requested() == false)
        {
            _device->read_all_and_set_antenna();

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};