#pragma once

#include "logging_facility.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

namespace LabNet::log
{
    class EasyLogger : public LoggingFacility
    {
    public:
        virtual void WriteInfoEntry(std::string_view entry) override
        {
            LOG(INFO) << entry;
        }
        virtual void WriteWarnEntry(std::string_view entry) override
        {
            LOG(WARNING) << entry;
        }
        virtual void WriteErrorEntry(std::string_view entry) override
        {
            LOG(ERROR) << entry;
        }
        virtual void WriteFatalEntry(std::string_view entry) override
        {
            LOG(FATAL) << entry;
        }
    };
};