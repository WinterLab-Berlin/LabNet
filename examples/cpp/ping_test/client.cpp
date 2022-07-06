#include "client.h"
#include "shared_buffer.h"
#include "start_mes.h"
#include <iostream>
#include <boost/bind.hpp>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>

Client::Client(std::string server, uint16_t port)
    : io_context_(1)
    , tcp_socket_(io_context_)
    , msg_type_(0)
    , msg_length_(0)
    , header_buffer_pos_(0)
{
    header_buffer_.resize(kMaxHeaderSize);
    boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(server), port);

    tcp_socket_.async_connect(
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(server), port),
        [this](const boost::system::error_code& ec) {
            if (!ec)
            {
                tcp_socket_.set_option(boost::asio::ip::tcp::no_delay(true));
                so_5::send<start>(recv_box_);

                StartReadHeader();
            }
            else
            {
                std::cout << "something went wrong" << std::endl;
            }
        });
}

Client::~Client()
{
}

void Client::SetRecvBox(so_5::mbox_t recv_box)
{
    recv_box_ = recv_box;
}

void Client::Start()
{
    io_context_.run();
}

void Client::Stop()
{
    tcp_socket_.close();
    io_context_.stop();
}

void Client::StartReadHeader()
{
    read_buffer_.resize(2);
    header_buffer_pos_ = 0;

    boost::asio::async_read(
        tcp_socket_,
        boost::asio::buffer(read_buffer_),
        [this](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec)
            {
                for (size_t i = 0; i < bytes_transferred; i++)
                {
                    header_buffer_[header_buffer_pos_++] = read_buffer_[i];
                }

                if (DecodeMsgTypeAndLength())
                {
                    StartReadBody();
                }
                else
                {
                    ReadHeader();
                }
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                std::cout << "connection closed" << std::endl;
            }
        });
}

void Client::ReadHeader()
{
    read_buffer_.resize(1);

    boost::asio::async_read(
        tcp_socket_,
        boost::asio::buffer(read_buffer_),
        [this](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec)
            {
                header_buffer_[header_buffer_pos_++] = read_buffer_[0];

                if (DecodeMsgTypeAndLength())
                {
                    StartReadBody();
                }
                else if (header_buffer_pos_ >= kMaxHeaderSize)
                {
                    StartReadHeader();
                }
                else
                {
                    ReadHeader();
                }
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                std::cout << "connection closed" << std::endl;
            }
        });
}

void Client::StartReadBody()
{
    auto self(shared_from_this());

    if (msg_length_ == 0)
    {
        HandleRequest();
        StartReadHeader();
    }
    else
    {
        read_buffer_.resize(msg_length_);

        boost::asio::async_read(
            tcp_socket_,
            boost::asio::buffer(read_buffer_),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (!ec)
                {
                    HandleRequest();

                    StartReadHeader();
                }
                else if (ec != boost::asio::error::operation_aborted)
                {
                    std::cout << "connection closed" << std::endl;
                }
            });
    }
}

void Client::HandleRequest()
{
    using namespace LabNetProt::Server;

    switch (msg_type_)
    {
        case ServerMessageType::NONE:
            break;
        case ServerMessageType::LABNET_ID_REPLY:
            ParseAndSendMessage<LabNetIdReply>();
            break;
        case ServerMessageType::LABNET_RESET_REPLY:
            ParseAndSendMessage<LabNetResetReply>();
            break;
        case ServerMessageType::DIGITAL_OUT_STATE:
            ParseAndSendMessage<DigitalOutState>();
            break;
        case ServerMessageType::DIGITAL_IN_STATE:
            ParseAndSendMessage<DigitalInState>();
            break;
        case ServerMessageType::NEW_BYTE_DATA:
            ParseAndSendMessage<NewByteData>();
            break;
        case ServerMessageType::DATA_WRITE_COMPLETE:
            ParseAndSendMessage<DataWriteComplete>();
            break;
        case ServerMessageType::INTERFACE_INIT_RESULT:
            ParseAndSendMessage<InterfaceInitResult>();
            break;
        case ServerMessageType::DIGITAL_IN_INIT_RESULT:
            ParseAndSendMessage<DigitalInInitResult>();
            break;
        case ServerMessageType::DIGITAL_OUT_INIT_RESULT:
            ParseAndSendMessage<DigitalOutInitResult>();
            break;
        case ServerMessageType::ONLY_ONE_CONNECTION_ALLOWED:
            ParseAndSendMessage<OnlyOneConnectionAllowed>();
            break;
        case ServerMessageType::INTERFACE_LOST:
            ParseAndSendMessage<InterfaceLost>();
            break;
        case ServerMessageType::INTERFACE_RECONNECTED:
            ParseAndSendMessage<InterfaceReconnected>();
            break;
        case ServerMessageType::DIGITAL_OUT_LOOP_START_RESULT:
            ParseAndSendMessage<DigitalOutLoopStartResult>();
            break;
        case ServerMessageType::DIGITAL_OUT_LOOP_STOPPED:
            ParseAndSendMessage<DigitalOutLoopStopped>();
            break;
        default:
            break;
    }
}

void Client::SendMessage(std::shared_ptr<google::protobuf::Message> mes, LabNetProt::Client::ClientMessageType mesType)
{
    auto self(shared_from_this());

    //std::shared_ptr<boost::asio::streambuf> b = std::make_shared<boost::asio::streambuf>();
    SharedBuffer b(std::bind(&Client::WriteErrorHandler, this, std::placeholders::_1, std::placeholders::_2));
    {
        std::ostream os(b.data.get());
        google::protobuf::io::OstreamOutputStream oos(&os);
        google::protobuf::io::CodedOutputStream st(&oos);

        st.WriteVarint32(mesType);
        st.WriteVarint32(mes->ByteSize());
        mes->SerializeWithCachedSizes(&st);

        //mes->SerializePartialToOstream(&os);

        //google::protobuf::util::SerializeDelimitedToCodedStream(*mes.get(), &st);
    }

    boost::asio::async_write(tcp_socket_,
        *b.data.get(),
        boost::bind(
            &SharedBuffer::write_handler,
            b,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void Client::WriteErrorHandler(const boost::system::error_code& error, const size_t bytesTransferred)
{
    if (error)
    {
        std::cout << "connection closed" << std::endl;
        return;
    }
}

bool Client::DecodeMsgTypeAndLength()
{
    msg_type_ = 0;
    msg_length_ = 0;

    bool type_present = false;
    int32_t shift = 0;

    for (size_t i = 0; i < header_buffer_pos_; i++)
    {
        if ((header_buffer_[i] & 0x80) == 0)
        {
            if (type_present)
            {
                msg_length_ = (msg_length_ | (header_buffer_[i] << shift));
                return true;
            }
            else
            {
                type_present = true;
                msg_type_ = (msg_type_ | (header_buffer_[i] << shift));
                shift = 0;
            }
        }
        else
        {
            if (type_present)
            {
                msg_length_ = (msg_length_ | ((header_buffer_[i] ^ 0x80) << shift));
                shift += 7;
            }
            else
            {
                msg_type_ = (msg_type_ | ((header_buffer_[i] ^ 0x80) << shift));
                shift += 7;
            }
        }
    }

    return false;
}