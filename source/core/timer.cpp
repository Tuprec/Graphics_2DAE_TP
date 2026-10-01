//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <timer.h>
#include <logger.h>
#include <iostream>
#include <iomanip>
using namespace gfx;

Timer::Timer()
{
    // Initialize time points to the current time
    const auto current_time	= std::chrono::high_resolution_clock::now();
    start_time_ = current_time;
    last_time_ = current_time;
    pause_time_ = current_time;
}

void Timer::Tick()
{
    if (is_paused_)
    {
        // If paused, don't update delta time but keep track of current time for when we resume
        delta_time_milliseconds_ = 0.0;
        delta_time_seconds_ = 0.0;
        fps_ = 0.0;
        return;
    }

    // Get the current time (nanoseconds).
    const auto current_time = std::chrono::high_resolution_clock::now();

    // Calculate delta time in milliseconds.
    const auto delta_duration = current_time - last_time_;
    delta_time_milliseconds_ = std::chrono::duration<double, std::milli>(delta_duration).count();
    delta_time_seconds_ = delta_time_milliseconds_ / 1000.0;

    // Calculate elapsed time since start (excluding paused time).
    const auto total_duration = current_time - start_time_;
    elapsed_time_ = std::chrono::duration<double>(total_duration).count();

    // Calculate FPS (frames per second).
    if (delta_time_seconds_ > 0.0)
        fps_ = 1.0 / delta_time_seconds_;
    else
        fps_ = 0.0;

    // Update last time for next frame and increment count.
    last_time_ = current_time;
    ++invocation_count_;

    // Perform accumulated benchmarking (when needed).
    if (benchmark_data_.remaining_frames_captured != 0)
    {
        const uint32_t frame_count = benchmark_data_.initial_frames_to_capture - benchmark_data_.remaining_frames_captured;
        benchmark_data_.average_ms += (delta_time_milliseconds_ - benchmark_data_.average_ms) / (frame_count + 1);
        benchmark_data_.minimum_ms = std::min(benchmark_data_.minimum_ms, delta_time_milliseconds_);
        benchmark_data_.maximum_ms = std::max(benchmark_data_.maximum_ms, delta_time_milliseconds_);
        benchmark_data_.remaining_frames_captured -= 1;
    }
}

void Timer::Pause()
{
    if (!is_paused_)
    {
        is_paused_ = true;
        pause_time_ = std::chrono::high_resolution_clock::now();
    }
}

void Timer::Resume()
{
    if (is_paused_)
    {
        is_paused_ = false;

        // Calculate how long we were paused.
        const auto current_time = std::chrono::high_resolution_clock::now();
        const auto pause_duration = current_time - pause_time_;

        // Adjust start time to exclude the paused duration from elapsed time calculations.
        start_time_ += pause_duration;

        // Reset last time to current time to avoid a large delta time spike.
        last_time_ = current_time;
    }
}

void Timer::StartBenchmark(const uint32_t frames_to_capture)
{
    // Do not enable/reset when we are currently tracking a benchmark
    if (!DidBenchmarkComplete())
        return;

    // Reset data.
    benchmark_data_ = {};
    benchmark_data_.initial_frames_to_capture = frames_to_capture;
    benchmark_data_.remaining_frames_captured = frames_to_capture;
}

bool Timer::IsPaused() const
{
    return is_paused_;
}

double Timer::GetDeltaTime() const
{
    return delta_time_milliseconds_;
}

double Timer::GetElapsedSeconds() const
{
    if (is_paused_)
    {
        // Return elapsed time up to when we paused.
        const auto pause_duration = pause_time_ - start_time_;
        return std::chrono::duration<double>(pause_duration).count();
    }

    return elapsed_time_;
}

double Timer::GetFps() const
{
    return fps_;
}

bool Timer::DidBenchmarkComplete() const
{
    return benchmark_data_.remaining_frames_captured == 0;
}

const BenchmarkData& Timer::GetBenchmarkData() const
{
    return benchmark_data_;
}

void Timer::PrintInformation(const float interval_seconds, const Logger* const logger) const
{
    const double current_elapsed = GetElapsedSeconds();
    if (current_elapsed - last_print_time_ >= interval_seconds)
    {
        if (logger)
        {
            logger->LogInfo("Timer Information | Delta Time: {:.2f} ms -- FPS: {:.2f} |",
                delta_time_milliseconds_, fps_);
        }
        else
        {
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Timer Information | ";
            std::cout << "Delta Time: " << delta_time_milliseconds_ << " ms -- ";
            std::cout << "FPS: " << fps_ << " |\n";
        }
        last_print_time_ = current_elapsed;
    }
}