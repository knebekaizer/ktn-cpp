
#include "log_ctest.h"

#include "spdlog/spdlog.h"

#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"

Logger* log_create(void)
{
	auto console = spdlog::stdout_color_mt("console");
	return (Logger*)console.get();
}


void log_write(Logger* log, const char* msg)
{
	((spdlog::logger*)log)->info(std::string("This is spdlog console: ") + msg);
}
