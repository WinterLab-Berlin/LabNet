#include "Connection.h"
#include "ConnectionManager.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>
#include <iostream>

Connection::Connection(Logger logger, boost::asio::ip::tcp::socket socket, ConnectionManager& connection_manager, so_5::mbox_t labNetBox)
    : m_logger(logger)
    , m_socket(std::move(socket))
    , m_connection_manager(connection_manager)
    , _labNetBox(labNetBox)
{
    _headerBuffer.resize(MAX_HEADER_SIZE);
}

void Connection::start()
{
    start_read_header();
}

void Connection::stop()
{
    m_logger->writeInfoEntry("connection stopped");

    m_socket.close();
}

void Connection::start_read_header()
{
    m_readBuffer.resize(2);
    _headerBufferPos = 0;

    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_readBuffer),
        [this](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec)
            {
                for (size_t i = 0; i < bytes_transferred; i++)
                {
                    _headerBuffer[_headerBufferPos++] = m_readBuffer[i];
                }

                if (decode_msg_type_and_length())
                {
                    start_read_body();
                }
                else
                {
                    read_header();
                }
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                m_connection_manager.stop(shared_from_this());
            }
        });
}

void Connection::read_header()
{
    m_readBuffer.resize(1);

    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_readBuffer),
        [this](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec)
            {
                _headerBuffer[_headerBufferPos++] = m_readBuffer[0];

                if (decode_msg_type_and_length())
                {
                    start_read_body();
                }
                else if (_headerBufferPos >= MAX_HEADER_SIZE)
                {
                    start_read_header();
                }
                else
                {
                    read_header();
                }
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                m_connection_manager.stop(shared_from_this());
            }
        });
}

void Connection::start_read_body()
{
    auto self(shared_from_this());

    if (_msgLength == 0)
    {
        handle_request();
        start_read_header();
    }
    else
    {
        m_readBuffer.resize(_msgLength);

        boost::asio::async_read(
            m_socket,
            boost::asio::buffer(m_readBuffer),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (!ec)
                {
                    handle_request();

                    start_read_header();
                }
                else if (ec != boost::asio::error::operation_aborted)
                {
                    m_connection_manager.stop(shared_from_this());
                }
            });
    }
}

void Connection::handle_request()
{
    using namespace LabNetProt::Client;

    switch (_msgType)
    {
    case ClientMessageType::NONE:
        break;
    case ClientMessageType::LABNET_RESET_REQUEST:
        parse_and_send_message<LabNetResetRequest>();
        break;
    case ClientMessageType::LABNET_ID_REQUEST:
        parse_and_send_message<LabNetIdRequest>();
        break;
    case ClientMessageType::IO_BOARD_INIT:
        parse_and_send_message<IoBoardInit>();
        break;
    case ClientMessageType::IO_BOARD_INIT_DIGITAL_IN:
        parse_and_send_message<IoBoardInitDigitalIn>();
        break;
    case ClientMessageType::IO_BOARD_INIT_DIGITAL_OUT:
        parse_and_send_message<IoBoardInitDigitalOut>();
        break;
    case ClientMessageType::RFID_BOARD_INIT:
        parse_and_send_message<RfidBoardInit>();
        break;
    case ClientMessageType::RFID_BOARD_SET_PHASE_MATRIX:
        parse_and_send_message<RfidBoardSetPhaseMatrix>();
        break;
    case ClientMessageType::UART_INIT:
        parse_and_send_message<UartInit>();
        break;
    case ClientMessageType::UART_WRITE_DATA:
        parse_and_send_message<UartWriteData>();
        break;
    case ClientMessageType::DIGITAL_OUT_SET:
        parse_and_send_message<DigitalOutSet>();
        break;
    case ClientMessageType::DIGITAL_OUT_PULSE:
        parse_and_send_message<DigitalOutPulse>();
        break;
    case ClientMessageType::START_DIGITAL_OUT_LOOP:
        parse_and_send_message<StartDigitalOutLoop>();
        break;
    case ClientMessageType::STOP_DIGITAL_OUT_LOOP:
        parse_and_send_message<StopDigitalOutLoop>();
        break;
    case ClientMessageType::GPIO_WIRINGPI_INIT:
        parse_and_send_message<GpioWiringPiInit>();
        break;
    case ClientMessageType::GPIO_WIRINGPI_INIT_DIGITAL_IN:
        parse_and_send_message<GpioWiringPiInitDigitalIn>();
        break;
    case ClientMessageType::GPIO_WIRINGPI_INIT_DIGITAL_OUT:
        parse_and_send_message<GpioWiringPiInitDigitalOut>();
        break;
    default:
        break;
    }
}

void Connection::refuse_conection()
{
    auto self(shared_from_this());

    std::shared_ptr<boost::asio::streambuf> b = std::make_shared<boost::asio::streambuf>();
    {
        std::ostream os(b.get());
        google::protobuf::io::OstreamOutputStream oos(&os);
        google::protobuf::io::CodedOutputStream st(&oos);
        st.WriteVarint32(LabNetProt::Server::ServerMessageType::ONLY_ONE_CONNECTION_ALLOWED);

        LabNetProt::Server::OnlyOneConnectionAllowed mes;
        google::protobuf::util::SerializeDelimitedToCodedStream(mes, &st);
    }

    boost::asio::async_write(m_socket,
        *b.get(),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec)
            {
                m_connection_manager.stop(shared_from_this());
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                m_connection_manager.stop(shared_from_this());
            }
        });
}

void Connection::send_message(std::shared_ptr<google::protobuf::Message> mes, LabNetProt::Server::ServerMessageType mesType)
{
    auto self(shared_from_this());

    std::shared_ptr<boost::asio::streambuf> b = std::make_shared<boost::asio::streambuf>();

    {
        std::ostream os(b.get());
        google::protobuf::io::OstreamOutputStream oos(&os);
        google::protobuf::io::CodedOutputStream st(&oos);
        st.WriteVarint32(mesType);

        google::protobuf::util::SerializeDelimitedToCodedStream(*mes.get(), &st);
    }

    write(b);
}

void Connection::write(std::shared_ptr<boost::asio::streambuf> buffer)
{
    boost::asio::async_write(m_socket,
        *buffer.get(),
        boost::bind(
            &Connection::write_handler,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void Connection::write_handler(const boost::system::error_code& error, const size_t bytesTransferred)
{
    if (error)
    {
        m_logger->writeErrorEntry(string_format("could not write: %s", boost::system::system_error(error).what()));
        m_connection_manager.stop(shared_from_this());
        return;
    }
}

bool Connection::decode_msg_type_and_length()
{
    _msgType = 0;
    _msgLength = 0;

    bool type_present = false;
    int32_t shift = 0;

    for (size_t i = 0; i < _headerBufferPos; i++)
    {
        if ((_headerBuffer[i] & 0x80) == 0)
        {
            if (type_present)
            {
                _msgLength = (_msgLength | (_headerBuffer[i] << shift));
                return true;
            }
            else
            {
                type_present = true;
                _msgType = (_msgType | (_headerBuffer[i] << shift));
                shift = 0;
            }
        }
        else
        {
            if (type_present)
            {
                _msgLength = (_msgLength | ((_headerBuffer[i] ^ 0x80) << shift));
                shift += 7;
            }
            else
            {
                _msgType = (_msgType | ((_headerBuffer[i] ^ 0x80) << shift));
                shift += 7;
            }
        }
    }

    return false;
}