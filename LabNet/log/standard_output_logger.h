#pragma once

#include "logging_facility.h"
#include <iostream>

namespace LabNet::log
{
    class StandardOutputLogger : public LoggingFacility
    {
    public:
        virtual void WriteInfoEntry(std::string_view entry) override
        {
            std::cout << "[INFO] " << entry << std::endl;
        }
        virtual void WriteWarnEntry(std::string_view entry) override
        {
            std::cout << "[WARNING] " << entry << std::endl;
        }
        virtual void WriteErrorEntry(std::string_view entry) override
        {
            std::cout << "[ERROR] " << entry << std::endl;
        }
        virtual void WriteFatalEntry(std::string_view entry) override
        {
            std::cout << "[FATAL] " << entry << std::endl;
        }
    };
};