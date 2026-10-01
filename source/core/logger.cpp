//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <logger.h>
using namespace gfx;

//--- Standard includes ---
#include <limits>
#include <ctime>

//--- External includes ---
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/callback_sink.h"

// Logger
Logger::Logger(const LoggerType enabled_logger_types, const LoggerMessageLevel enabled_level)
{
	// Cache incoming information.
	level_ = enabled_level;
	enabled_logger_types_ = enabled_logger_types;
	// Add all possible loggers but enable/disable based on passed variable.
	// We do this so we can enable/disable them at runtime without having to recreate the logger.
	// Each sink is paired with its corresponding LoggerType bit flag.
	LoggerType sink_types[] = { LoggerType::kConsole, LoggerType::kFile };
	sinks_.reserve(std::size(sink_types));
	sinks_.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()); //kConsole
	sinks_.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename_, true)); //kFile
	for (uint32_t i = 0; i < static_cast<uint32_t>(sinks_.size()); ++i)
	{
		if ((sink_types[i] & enabled_logger_types_) == sink_types[i])
			sinks_.at(i)->set_level(static_cast<spdlog::level::level_enum>(level_));
		else
			sinks_.at(i)->set_level(spdlog::level::off);
	}
	// Create the logger using the sinks defined earlier. Make sure to set level and flush.
	logger_ = std::make_shared<spdlog::logger>("", std::begin(sinks_), std::end(sinks_));
	logger_->set_level(static_cast<spdlog::level::level_enum>(level_));
	logger_->flush_on(static_cast<spdlog::level::level_enum>(level_));
}

Logger::~Logger()
{
	logger_.reset();
	for (spdlog::sink_ptr sink : sinks_)
		sink.reset();
	sinks_.clear();
}

bool Logger::IsLoggerTypeEnabled(LoggerType enabled_logger_type) const
{
	return (enabled_logger_type & enabled_logger_types_) == enabled_logger_type;
}
