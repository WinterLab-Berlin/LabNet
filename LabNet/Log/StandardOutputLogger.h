#pragma once

#include "LoggingFacility.h"
#include <iostream>

class StandardOutputLogger : public LoggingFacility {
public:
	virtual void writeInfoEntry(std::string_view entry) override {
		std::cout << "[INFO] " << entry << std::endl;
	}
	virtual void writeWarnEntry(std::string_view entry) override {
		std::cout << "[WARNING] " << entry << std::endl;
	}
	virtual void writeErrorEntry(std::string_view entry) override {
		std::cout << "[ERROR] " << entry << std::endl;
	}
	virtual void writeFatalEntry(std::string_view entry) override {
		std::cout << "[FATAL] " << entry << std::endl;
	}
};