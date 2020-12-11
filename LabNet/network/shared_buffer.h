#pragma once

#include <boost/asio.hpp>

class SharedBuffer
{
public:
    explicit SharedBuffer(std::function<void(const boost::system::error_code&, const size_t)> callback)
        : callback_(callback)
    {
    }

    void write_handler(const boost::system::error_code& error, const size_t bytesTransferred)
    {
        callback_(error, bytesTransferred);
    };

    std::shared_ptr<boost::asio::streambuf> data = std::make_shared<boost::asio::streambuf>();

private:
    std::function<void(const boost::system::error_code&, const size_t)> callback_;
};