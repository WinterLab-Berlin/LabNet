#include "connection.h"
#include "connection_manager.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>
#include <iostream>
#include "shared_buffer.h"

namespace LabNet::network
{
    Connection::Connection(log::Logger logger, boost::asio::ip::tcp::socket socket, ConnectionManager& connection_manager, so_5::mbox_t server_in_box)
        : logger_(logger)
        , socket_(std::move(socket))
        , connection_manager_(connection_manager)
        , server_in_box_(server_in_box)
    {
        header_buffer_.resize(kMaxHeaderSize);
    }

    void Connection::Start()
    {
        StartReadHeader();
    }

    void Connection::Stop()
    {
        logger_->WriteInfoEntry("connection stopped");

        socket_.close();
    }

    void Connection::StartReadHeader()
    {
        read_buffer_.resize(2);
        header_buffer_pos_ = 0;

        boost::asio::async_read(
            socket_,
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
                    connection_manager_.Stop(shared_from_this());
                }
            });
    }

    void Connection::ReadHeader()
    {
        read_buffer_.resize(1);

        boost::asio::async_read(
            socket_,
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
                    connection_manager_.Stop(shared_from_this());
                }
            });
    }

    void Connection::StartReadBody()
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
                socket_,
                boost::asio::buffer(read_buffer_),
                [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                    if (!ec)
                    {
                        HandleRequest();

                        StartReadHeader();
                    }
                    else if (ec != boost::asio::error::operation_aborted)
                    {
                        connection_manager_.Stop(shared_from_this());
                    }
                });
        }
    }

    void Connection::HandleRequest()
    {
        using namespace LabNetProt::Client;

        switch (msg_type_)
        {
            case ClientMessageType::NONE:
                break;
            case ClientMessageType::LABNET_RESET_REQUEST:
                ParseAndSendMessage<LabNetResetRequest>();
                break;
            case ClientMessageType::LABNET_ID_REQUEST:
                ParseAndSendMessage<LabNetIdRequest>();
                break;
            case ClientMessageType::IO_BOARD_INIT:
                ParseAndSendMessage<IoBoardInit>();
                break;
            case ClientMessageType::IO_BOARD_INIT_DIGITAL_IN:
                ParseAndSendMessage<IoBoardInitDigitalIn>();
                break;
            case ClientMessageType::IO_BOARD_INIT_DIGITAL_OUT:
                ParseAndSendMessage<IoBoardInitDigitalOut>();
                break;
            case ClientMessageType::RFID_BOARD_INIT:
                ParseAndSendMessage<RfidBoardInit>();
                break;
            case ClientMessageType::RFID_BOARD_SET_PHASE_MATRIX:
                ParseAndSendMessage<RfidBoardSetPhaseMatrix>();
                break;
            case ClientMessageType::UART_INIT:
                ParseAndSendMessage<UartInit>();
                break;
            case ClientMessageType::UART_WRITE_DATA:
                ParseAndSendMessage<UartWriteData>();
                break;
            case ClientMessageType::DIGITAL_OUT_SET:
                ParseAndSendMessage<DigitalOutSet>();
                break;
            case ClientMessageType::DIGITAL_OUT_PULSE:
                ParseAndSendMessage<DigitalOutPulse>();
                break;
            case ClientMessageType::START_DIGITAL_OUT_LOOP:
                ParseAndSendMessage<StartDigitalOutLoop>();
                break;
            case ClientMessageType::STOP_DIGITAL_OUT_LOOP:
                ParseAndSendMessage<StopDigitalOutLoop>();
                break;
            case ClientMessageType::GPIO_WIRINGPI_INIT:
                ParseAndSendMessage<GpioWiringPiInit>();
                break;
            case ClientMessageType::GPIO_WIRINGPI_INIT_DIGITAL_IN:
                ParseAndSendMessage<GpioWiringPiInitDigitalIn>();
                break;
            case ClientMessageType::GPIO_WIRINGPI_INIT_DIGITAL_OUT:
                ParseAndSendMessage<GpioWiringPiInitDigitalOut>();
                break;
            case ClientMessageType::INIT_SOUND:
                ParseAndSendMessage<InitSound>();
                break;
            case ClientMessageType::INIT_SOUND_SIGNAL:
                ParseAndSendMessage<InitSoundSignal>();
                break;
            case ClientMessageType::UART_INIT_DIGITAL_IN:
                ParseAndSendMessage<UartInitDigitalIn>();
                break;
            case ClientMessageType::UART_INIT_DIGITAL_OUT:
                ParseAndSendMessage<UartInitDigitalOut>();
                break;
            case ClientMessageType::CHI_BIO_INIT:
                ParseAndSendMessage<ChiBioInit>();
                break;
            case ClientMessageType::MOVE_CHI_BIO_PUMP:
                ParseAndSendMessage<MoveChiBioPump>();
                break;
            case ClientMessageType::BLE_UART_INIT:
                ParseAndSendMessage<BleUartInit>();
                break;
            default:
                break;
        }
    }

    void Connection::RefuseConection()
    {
        auto self(shared_from_this());

        SharedBuffer b(std::bind(&Connection::WriteStopHandler, this, std::placeholders::_1, std::placeholders::_2));
        {
            std::ostream os(b.data.get());
            google::protobuf::io::OstreamOutputStream oos(&os);
            google::protobuf::io::CodedOutputStream st(&oos);

            st.WriteVarint32(LabNetProt::Server::ServerMessageType::ONLY_ONE_CONNECTION_ALLOWED);

            LabNetProt::Server::OnlyOneConnectionAllowed mes;
            st.WriteVarint32(mes.ByteSize());
            mes.SerializeWithCachedSizes(&st);
        }

        boost::asio::async_write(socket_,
            *b.data.get(),
            boost::bind(
                &SharedBuffer::write_handler,
                b,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void Connection::SendMessage(std::shared_ptr<google::protobuf::Message> mes, LabNetProt::Server::ServerMessageType mesType)
    {
        auto self(shared_from_this());

        //std::shared_ptr<boost::asio::streambuf> b = std::make_shared<boost::asio::streambuf>();
        SharedBuffer b(std::bind(&Connection::WriteErrorHandler, this, std::placeholders::_1, std::placeholders::_2));
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

        boost::asio::async_write(socket_,
            *b.data.get(),
            boost::bind(
                &SharedBuffer::write_handler,
                b,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void Connection::WriteErrorHandler(const boost::system::error_code& error, const size_t bytesTransferred)
    {
        if (error)
        {
            logger_->WriteErrorEntry(log::StringFormat("could not write: %s", boost::system::system_error(error).what()));
            connection_manager_.Stop(shared_from_this());
            return;
        }
    }

    void Connection::WriteStopHandler(const boost::system::error_code& error, const size_t bytesTransferred)
    {
        connection_manager_.Stop(shared_from_this());
    }

    bool Connection::DecodeMsgTypeAndLength()
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
}