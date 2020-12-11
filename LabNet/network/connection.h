#pragma once

#include "protocol_all.h"
#include <logging_facility.h>
#include <array>
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <mutex>
#include <so_5/all.hpp>
#include <vector>

namespace LabNet::network
{
    class ConnectionManager;

    class Connection
        : public std::enable_shared_from_this<Connection>
    {
    public:
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        /// Construct a connection with the given socket.
        explicit Connection(log::Logger logger, boost::asio::ip::tcp::socket socket, ConnectionManager& connection_manager, so_5::mbox_t server_in_box);

        /// Start the first asynchronous operation for the connection.
        void Start();

        /// Stop all asynchronous operations associated with the connection.
        void Stop();

        void RefuseConection();

        void SendMessage(std::shared_ptr<google::protobuf::Message> mes, LabNetProt::Server::ServerMessageType mesType);

    private:
        void WriteErrorHandler(const boost::system::error_code& error, const size_t bytesTransferred);
        void WriteStopHandler(const boost::system::error_code& error, const size_t bytesTransferred);

        void StartReadHeader();
        void ReadHeader();

        void StartReadBody();

        void HandleRequest();
        template <typename T>
        void ParseAndSendMessage()
        {
            std::shared_ptr<T> mes = std::make_shared<T>();
            if (mes->ParseFromArray(&read_buffer_[0], msg_length_))
            {
                so_5::send<std::shared_ptr<T>>(server_in_box_, mes);
            }
            else
            {
                logger_->WriteInfoEntry("invalid message");
            }
        };

        bool DecodeMsgTypeAndLength();

        const so_5::mbox_t server_in_box_;

        log::Logger logger_;
        boost::asio::ip::tcp::socket socket_;
        ConnectionManager& connection_manager_;
        std::vector<uint8_t> read_buffer_;

        int32_t msg_type_, msg_length_;
        const uint8_t kMaxHeaderSize = 10;
        uint8_t header_buffer_pos_;
        std::vector<uint8_t> header_buffer_;
    };
};