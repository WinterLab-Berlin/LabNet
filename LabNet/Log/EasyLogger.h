#pragma once

#include "LoggingFacility.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

class EasyLogger : public LoggingFacility {
public:
	virtual void writeInfoEntry(std::string_view entry) override {
		LOG(INFO) << entry;
	}
	virtual void writeWarnEntry(std::string_view entry) override {
		LOG(WARNING) << entry;
	}
	virtual void writeErrorEntry(std::string_view entry) override {
		LOG(ERROR) << entry;
	}
	virtual void writeFatalEntry(std::string_view entry) override {
		LOG(FATAL) << entry;
	}
}
;