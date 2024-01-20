#include "timer.hpp"

namespace StudentSync::Common {
	void Timer::Stop() {
		shouldStop.store(true);
	}

	Timer::~Timer() noexcept {
		Stop();

		if (executor.joinable()) {
			executor.join();
		}
	}
}