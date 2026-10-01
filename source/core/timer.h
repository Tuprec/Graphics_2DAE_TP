//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef TIMER_HEADER
#define TIMER_HEADER

//--- Standard Includes ---
#include <chrono>
#include <limits>

namespace gfx
{
	//--- Supporting Structs ---
	struct BenchmarkData final
	{
		double minimum_ms{ std::numeric_limits<double>::max() };
		double maximum_ms{ std::numeric_limits<double>::lowest() };
		double average_ms{ 0.0 };
		uint32_t initial_frames_to_capture{ 0 };
		uint32_t remaining_frames_captured{ 0 };
	};

	//--- Class ----
	class Logger;
	class Timer final
	{
		//--- Data members ---
		BenchmarkData benchmark_data_{};
		std::chrono::time_point<std::chrono::high_resolution_clock> start_time_{};
		std::chrono::time_point<std::chrono::high_resolution_clock> last_time_{};
		std::chrono::time_point<std::chrono::high_resolution_clock> pause_time_{};
		uint64_t invocation_count_{ 0 };

		mutable double last_print_time_{ 0.0 };
		double delta_time_milliseconds_{ 0.0 };
		double delta_time_seconds_{ 0.0 };
		double elapsed_time_{ 0.0 };
		double fps_{ 0.0 };
		bool is_paused_{ false };

	public:
		//--- Constructors & Destructor ---
		Timer();
		~Timer() = default;
		Timer(const Timer&) = default;
		Timer& operator=(const Timer&) = default;
		Timer(Timer&&) = default;
		Timer& operator=(Timer&&) = default;

		//--- Functions ---
		void Tick();
		void Pause();
		void Resume();
		void StartBenchmark(const uint32_t frames_to_capture = 10);
		[[nodiscard]] bool IsPaused() const;
		[[nodiscard]] double GetDeltaTime() const; // in milliseconds
		[[nodiscard]] double GetElapsedSeconds() const;
		[[nodiscard]] double GetFps() const;
		[[nodiscard]] bool DidBenchmarkComplete() const;
		[[nodiscard]] const BenchmarkData& GetBenchmarkData() const;
		void PrintInformation(const float interval_seconds, const Logger* const logger = nullptr) const;
	};
}
#endif //TIMER_HEADER