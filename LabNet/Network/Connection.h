#pragma once

#include "ProtocolAll.h"
#include <LoggingFacility.h>
#include <array>
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <mutex>
#include <so_5/all.hpp>
#include <vector>

class ConnectionManager;

class Connection
    : public std::enable_shared_from_this<Connection>
{
public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    /// Construct a connection with the given socket.
    explicit Connection(Logger logger, boost::asio::ip::tcp::socket socket, ConnectionManager& connection_manager, so_5::mbox_t labNetBox);

    /// Start the first asynchronous operation for the connection.
    void start();

    /// Stop all asynchronous operations associated with the connection.
    void stop();

    void refuse_conection();

    void send_message(std::shared_ptr<google::protobuf::Message> mes, LabNetProt::Server::ServerMessageType mesType);

    

private:
    void write_error_handler(const boost::system::error_code& error, const size_t bytesTransferred);
    void write_stop_handler(const boost::system::error_code& error, const size_t bytesTransferred);

    void start_read_header();
    void read_header();

    void start_read_body();

    void handle_request();
    template <typename T>
    void parse_and_send_message()
    {
        std::shared_ptr<T> mes = std::make_shared<T>();
        if (mes->ParseFromArray(&m_readBuffer[0], _msgLength))
        {
            so_5::send<std::shared_ptr<T>>(_labNetBox, mes);
        }
        else
        {
            m_logger->writeInfoEntry("invalid message");
        }
    };

    bool decode_msg_type_and_length();

    so_5::mbox_t _labNetBox;

    Logger m_logger;
    boost::asio::ip::tcp::socket m_socket;
    ConnectionManager& m_connection_manager;
    std::vector<uint8_t> m_readBuffer;

    int32_t _msgType, _msgLength;
    const uint8_t MAX_HEADER_SIZE = 10;
    uint8_t _headerBufferPos;
    std::vector<uint8_t> _headerBuffer;
};