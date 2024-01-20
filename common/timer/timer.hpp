#pragma once

#include <concepts>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace StudentSync::Common {
	struct Timer {

		Timer(std::invocable auto function, std::chrono::milliseconds timeout);
		void Stop();

		// Since the executor captures `this`, the object
		// MUST NEVER be moved or copied.
		Timer(const Timer&) = delete;				// Copy constructor
		Timer& operator=(const Timer&) = delete;	// Copy assignment
		Timer(Timer&&) = delete;					// Move constructor
		Timer& operator=(Timer&&) = delete;			// Move assignment
		~Timer() noexcept;

	private:
		//! shouldStop should come first so that it beats executor in initialization order
		std::atomic<bool> shouldStop;
		std::jthread executor;

		auto CreateJob(std::invocable auto function, std::chrono::milliseconds timeout);
	};

	Timer::Timer(std::invocable auto function, std::chrono::milliseconds timeout)
		: shouldStop{ false }
		, executor{ CreateJob(function, timeout) } {}


	inline auto Timer::CreateJob(std::invocable auto function, std::chrono::milliseconds timeout) {
		return [=]() {
			while (!shouldStop.load()) {
				function();

				// TODO: use chrono::steady_clock and account for function() execution time
				std::this_thread::sleep_for(timeout);
			}
		};
	}
}