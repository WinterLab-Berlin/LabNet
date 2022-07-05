#include "connection.h"
#include <stdlib.h>
#include <vector>
#include <chrono>
#include "../stream_messages.h"
#include "../interfaces.h"

using namespace LabNet::interface::ble_uart;

#define NUS_CHARACTERISTIC_TX_UUID	"6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_CHARACTERISTIC_RX_UUID	"6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define DEVICE "4E:46:C1:F7:73:7C"

Connection::Connection(std::string device, log::Logger logger, const so_5::mbox_t parent)
    : device_(device)
    , logger_(logger)
    , parent_(parent)
{
}

Connection::~Connection()
{
    if(connection_ != NULL)
    {
        gattlib_disconnect(connection_);
        gattlib_notification_stop(connection_, &nus_characteristic_rx_uuid_);
        gattlib_register_notification(connection_, NULL, NULL);
        gattlib_register_on_disconnect(connection_, NULL, NULL);
        connection_ = NULL;
    }
}

void notification_cb(const uuid_t *uuid, const uint8_t *data, size_t data_length, void *user_data)
{
    std::shared_ptr<std::vector<char>> send_data = std::make_shared<std::vector<char>>();
    for(int i = 0; i < data_length; i++) {
		send_data->push_back(data[i]);
	}

    Connection* parent = static_cast<Connection *>(user_data);
    //so_5::send<std::shared_ptr<std::vector<char>>>(parent->GetParentBox(), send_data);
    so_5::send<LabNet::interface::stream_messages::NewDataFromPort>(parent->GetParentBox(), LabNet::interface::Interfaces::BleUart, 0, send_data, std::chrono::high_resolution_clock::now());
    fflush(stdout);
}

void notification_disconnect(void* user_data)
{
    Connection *c = static_cast<Connection *>(user_data);
    c->Disconnected();
}

void Connection::Terminate()
{
    if(connection_ != NULL)
    {
        gattlib_notification_stop(connection_, &nus_characteristic_rx_uuid_);
        gattlib_register_notification(connection_, NULL, NULL);
        gattlib_register_on_disconnect(connection_, NULL, NULL);
        gattlib_disconnect(connection_);
        connection_ = NULL;
    }
}

void Connection::Disconnected()
{
    logger_->WriteInfoEntry("ble_uart: diconnected");
    so_5::send<DisconnectedMsg>(parent_);

    if(connection_ != NULL)
    {
        gattlib_notification_stop(connection_, &nus_characteristic_rx_uuid_);
        gattlib_register_notification(connection_, NULL, NULL);
        gattlib_register_on_disconnect(connection_, NULL, NULL);
        connection_ = NULL;
    }
}

void Connection::Connect()
{
    connection_ = gattlib_connect(NULL, device_.c_str(),
				       GATTLIB_CONNECTION_OPTIONS_LEGACY_BDADDR_LE_RANDOM |
				       GATTLIB_CONNECTION_OPTIONS_LEGACY_BT_SEC_LOW);
	if (connection_ == NULL) {
		logger_->WriteInfoEntry("ble_uart: fail to connect to the bluetooth device.");
		so_5::send<DisconnectedMsg>(parent_);
        return;
	}

    int ret;
    
    // Convert characteristics to their respective UUIDs
    ret = gattlib_string_to_uuid(NUS_CHARACTERISTIC_TX_UUID, strlen(NUS_CHARACTERISTIC_TX_UUID) + 1, &nus_characteristic_tx_uuid_);
	if (ret) {
		logger_->WriteInfoEntry("ble_uart: fail to convert characteristic TX to UUID.");
		so_5::send<DisconnectedMsg>(parent_);
        return;
	}

    ret = gattlib_string_to_uuid(NUS_CHARACTERISTIC_RX_UUID, strlen(NUS_CHARACTERISTIC_RX_UUID) + 1, &nus_characteristic_rx_uuid_);
	if (ret) {
		logger_->WriteInfoEntry("ble_uart: fail to convert characteristic RX to UUID.");
		so_5::send<DisconnectedMsg>(parent_);
        return;
	}

    // Look for handle for NUS_CHARACTERISTIC_TX_UUID
	gattlib_characteristic_t* characteristics;
	int characteristic_count;
	ret = gattlib_discover_char(connection_, &characteristics, &characteristic_count);
	if (ret) {
		logger_->WriteInfoEntry("ble_uart: fail to discover characteristic.");
		so_5::send<DisconnectedMsg>(parent_);
        return;
	}

    uint16_t tx_handle = 0, rx_handle = 0;
	for (int i = 0; i < characteristic_count; i++) {
		if (gattlib_uuid_cmp(&characteristics[i].uuid, &nus_characteristic_tx_uuid_) == 0) {
			tx_handle = characteristics[i].value_handle;
		} else if (gattlib_uuid_cmp(&characteristics[i].uuid, &nus_characteristic_rx_uuid_) == 0) {
			rx_handle = characteristics[i].value_handle;
		}
	}
    if (tx_handle == 0) {
		logger_->WriteInfoEntry("ble_uart: fail to find NUS TX characteristic.");
		so_5::send<DisconnectedMsg>(parent_);
        return;
	} else if (rx_handle == 0) {
		logger_->WriteInfoEntry("ble_uart: fail to find NUS RX characteristic.");
		so_5::send<DisconnectedMsg>(parent_);
        return;
	}
	free(characteristics);

    // Register notification handler
	gattlib_register_notification(connection_, notification_cb, this);
    gattlib_register_on_disconnect(connection_, notification_disconnect, this);

    ret = gattlib_notification_start(connection_, &nus_characteristic_rx_uuid_);
	if (ret)
    {
		logger_->WriteInfoEntry("ble_uart: fail to start notification.");
		so_5::send<DisconnectedMsg>(parent_);
        return;
    }

    logger_->WriteInfoEntry("ble_uart: connected to the bluetooth device.");
    so_5::send<ConnectedMsg>(parent_);
}
