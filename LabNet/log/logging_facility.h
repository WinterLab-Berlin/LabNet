#pragma once

#include <memory>
#include <string_view>
#include <string>
#include <stdexcept>

namespace LabNet::log
{
    template <typename... Args>
    std::string StringFormat(const std::string& format, Args... args)
    {
        size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
        if (size <= 0)
        {
            throw std::runtime_error("Error during formatting.");
        }
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

    class LoggingFacility
    {
    public:
        virtual ~LoggingFacility() = default;
        virtual void WriteInfoEntry(std::string_view entry) = 0;
        virtual void WriteWarnEntry(std::string_view entry) = 0;
        virtual void WriteErrorEntry(std::string_view entry) = 0;
        virtual void WriteFatalEntry(std::string_view entry) = 0;
    };

    using Logger = std::shared_ptr<LoggingFacility>;
};
