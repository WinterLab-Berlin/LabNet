#pragma once

#include <string>
#include <gattlib.h>
#include <so_5/all.hpp>
#include <logging_facility.h>

namespace LabNet::interface::ble_uart
{
    struct ConnectedMsg
    {
    };
    struct DisconnectedMsg
    {
    };

    class Connection
    {
    public:
        Connection(std::string device, log::Logger logger, const so_5::mbox_t parent);
        ~Connection();

        void Connect();
        void Terminate();
        void Disconnected();

        const so_5::mbox_t GetParentBox() { return parent_; };

    private:
        const so_5::mbox_t parent_;
        log::Logger logger_;
        std::string device_;
        gatt_connection_t* connection_;
        uuid_t nus_characteristic_tx_uuid_;
        uuid_t nus_characteristic_rx_uuid_;
    };
}