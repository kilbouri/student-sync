#include "serversession.h"

#include <functional>

namespace StudentSync::Server {
	Session::Session(TCPSocket&& socket)
		: lock{ std::mutex() }
		, notifier{ std::condition_variable() }
		, socket{ std::move(socket) }
		, state{ State::Idle }
		, lastState{ State::Idle }
		, executor{ nullptr }
	{
		executor = std::make_unique<std::jthread>(std::bind(&Session::ThreadEntry, this));
	}

	void Session::SetState(State state) {
		{
			std::scoped_lock<std::mutex> guard{ lock };
			this->lastState = this->state;
			this->state = state;
		}

		notifier.notify_all();
	}

	void Session::Join() {
		{
			std::scoped_lock<std::mutex> guard{ lock };
			if (this->state != State::Terminated) {
				throw "Session::Join must only be called while the Session is in the Terminated state!";
			}
		}

		executor->join();
	}

	void Session::ThreadEntry() {
		std::unique_lock guard{ lock };

		while (this->state != State::Terminated) {
			notifier.wait_for(guard, std::chrono::seconds(5));

			// todo: do stuff with updated state...
		}
	}
}
