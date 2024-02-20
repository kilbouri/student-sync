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
				auto startTime = std::chrono::steady_clock::now();

				function();

				auto endTime = std::chrono::steady_clock::now();
				auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

				auto remainingTime = timeout - elapsedTime;
				if (remainingTime > std::chrono::milliseconds(0)) {
					std::this_thread::sleep_for(remainingTime);
				}
			}
		};
	}
}