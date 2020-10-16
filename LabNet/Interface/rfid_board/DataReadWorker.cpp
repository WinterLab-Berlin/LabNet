#include "DataReadWorker.h"

MAX14830::DataReadWorker::DataReadWorker(std::shared_ptr<MAX14830::MAXDevice> dev)
    : _device(dev)
    , _futureObj(_exitSignal.get_future())
{
    _device->reset_buffers();

    std::thread worker { &MAX14830::DataReadWorker::read_rfid_thread, this };
    _readWorker = std::move(worker);
}

MAX14830::DataReadWorker::~DataReadWorker()
{
    _exitSignal.set_value();
    _readWorker.join();

    _device->stop();
}

bool MAX14830::DataReadWorker::stop_requested()
{
    // checks if value in future object is available
    if (_futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return false;
    return true;
}

void MAX14830::DataReadWorker::read_rfid_thread()
{
    while (stop_requested() == false)
    {
        _device->read_all_and_set_antenna();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}