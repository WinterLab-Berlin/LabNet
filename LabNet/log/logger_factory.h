#pragma once

#include "logging_facility.h"
#include "standard_output_logger.h"
#include "easy_logger.h"

namespace LabNet::log
{
    class LoggerFactory
    {
    public:
        static Logger create()
        {
            // read config from file
            std::string fileName = "logger.conf";
            std::ifstream fileStream(fileName);
            if (fileStream.good())
            {
                el::Configurations conf(fileName);
                el::Loggers::reconfigureAllLoggers(conf);

                return std::make_shared<EasyLogger>();
            }
            else
            {
                return std::make_shared<StandardOutputLogger>();
            }
        }
    };
};