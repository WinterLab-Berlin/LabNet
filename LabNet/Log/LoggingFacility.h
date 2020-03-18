#pragma once

#include <memory>
#include <string_view>

template<typename ... Args>
	std::string string_format(const std::string& format, Args ... args)
	{
		size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;    // Extra space for '\0'
		if(size <= 0){ throw std::runtime_error("Error during formatting."); }
		std::unique_ptr<char[]> buf(new char[size]); 
		snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

class LoggingFacility {
public:
	virtual ~LoggingFacility() = default;
	virtual void writeInfoEntry(std::string_view entry) = 0;
	virtual void writeWarnEntry(std::string_view entry) = 0;
	virtual void writeErrorEntry(std::string_view entry) = 0;
	virtual void writeFatalEntry(std::string_view entry) = 0;
};

using Logger = std::shared_ptr<LoggingFacility>;