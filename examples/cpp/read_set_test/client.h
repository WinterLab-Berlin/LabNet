#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <so_5/all.hpp>

#include "prot/LabNet.pb.h"
#include "prot/LabNetClient.pb.h"
#include "prot/LabNetServer.pb.h"

class Client
    : public std::enable_shared_from_this<Client>
{
public:
    Client(std::string server, uint16_t port);
    ~Client();

    void SetRecvBox(so_5::mbox_t recv_box);
    void Start();
    void Stop();
    void SendMessage(std::shared_ptr<google::protobuf::Message> mes, LabNetProt::Client::ClientMessageType mesType);

private:
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
            so_5::send<std::shared_ptr<T>>(recv_box_, mes);
        }
        else
        {
            
        }
    };

    bool DecodeMsgTypeAndLength();

    void WriteErrorHandler(const boost::system::error_code& error, const size_t bytesTransferred);

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket tcp_socket_;
    so_5::mbox_t recv_box_;

    std::vector<uint8_t> read_buffer_;

    int32_t msg_type_, msg_length_;
    const uint8_t kMaxHeaderSize = 10;
    uint8_t header_buffer_pos_;
    std::vector<uint8_t> header_buffer_;
};
