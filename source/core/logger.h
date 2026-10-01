//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef LOGGER_HEADER
#define LOGGER_HEADER

//--- Standard includes ---
#include <vector>
#include <memory>

//--- External includes ---
// The spdlog library does not use the latest fmt library, which triggers
// a compile error for 'formattable'. Disable this for now until patched!
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable: 4459)
#elif defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <spdlog/spdlog.h>
#ifdef _MSC_VER
	#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic pop
#endif

namespace gfx
{
	// Logger enums
	enum class LoggerMessageLevel : uint8_t
	{
		kTrace,
		kDebug,
		kInfo,
		kWarning,
		kError,
		kCritical,
		kOff
	};

	enum class LoggerType : uint8_t
	{
		kNone = 0x0,
		kConsole = 0x1,
		kFile = 0x2,
	};

	inline LoggerType operator&(const LoggerType& lhs, LoggerType rhs)
	{
		return static_cast<LoggerType>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
	}

	inline LoggerType operator|(const LoggerType lhs, LoggerType rhs)
	{
		return static_cast<LoggerType>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
	}

	inline void operator&=(LoggerType& lhs, LoggerType rhs)
	{
		lhs = static_cast<LoggerType>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
	}

	inline void operator|=(LoggerType& lhs, LoggerType rhs)
	{
		lhs = static_cast<LoggerType>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
	}

	// Logger
	class Logger final
	{
		//--- Data members ---
		std::shared_ptr<spdlog::logger> logger_{ nullptr };
		std::vector<spdlog::sink_ptr> sinks_{};
		LoggerType enabled_logger_types_{ LoggerType::kConsole };
		LoggerMessageLevel level_{ LoggerMessageLevel::kTrace };
		const char* filename_{ "report.log" };

	public:
		//--- Constructors & Destructor ---
		Logger(const LoggerType enabled_logger_types = LoggerType::kConsole,
			const LoggerMessageLevel enabled_level = LoggerMessageLevel::kTrace);
		~Logger();
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		Logger(Logger&&) noexcept = delete;
		Logger& operator=(Logger&&) noexcept = delete;

		//--- Functions ---
		[[nodiscard]] const char* GetReportFilename() const { return filename_; }
		[[nodiscard]] bool IsLoggerTypeEnabled(LoggerType enabled_logger_type) const;

		template<typename... Args>
		void LogTrace(const char* msg, Args&&... args) const
		{
			logger_->trace(fmt::runtime(msg), std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogDebug(const char* msg, Args&&... args) const
		{
			logger_->debug(fmt::runtime(msg), std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogInfo(const char* msg, Args&&... args) const
		{
			logger_->info(fmt::runtime(msg), std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogWarning(const char* msg, Args&&... args) const
		{
			logger_->warn(fmt::runtime(msg), std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogError(const char* msg, Args&&... args) const
		{
			logger_->error(fmt::runtime(msg), std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogCritical(const char* msg, Args&&... args) const
		{
			logger_->critical(fmt::runtime(msg), std::forward<Args>(args)...);
		}
	};
}
#endif //LOGGER_HEADER