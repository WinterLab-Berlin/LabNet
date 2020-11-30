#pragma once

#include "Connection.h"

class shared_buffer
{
public:
    explicit shared_buffer(Logger logger, std::function<void(const boost::system::error_code&, const size_t)> callback)
        : m_logger(logger)
        , _callback(callback)
    {
    }

    void write_handler(const boost::system::error_code& error, const size_t bytesTransferred)
    {
        _callback(error, bytesTransferred);
    };

    std::shared_ptr<boost::asio::streambuf> data_ = std::make_shared<boost::asio::streambuf>();

private:
    Logger m_logger;
    std::function<void(const boost::system::error_code&, const size_t)> _callback;
};