#pragma once

#include "LoggingFacility.h"
#include "StandardOutputLogger.h"
#include "EasyLogger.h"

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