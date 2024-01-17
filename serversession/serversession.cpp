#include "serversession.h"

#include <functional>
#include "../common/message/message.h"

using namespace StudentSync::Common;

namespace StudentSync::Server {
	Session::Session(unsigned long identifier, TCPSocket&& socket, std::shared_ptr<EventDispatcher> dispatcher)
		: identifier{ identifier }
		, lock{ std::mutex() }
		, notifier{ std::condition_variable() }
		, socket{ std::move(socket) }
		, __state{ State::Idle }
		, executor{ nullptr }
		, dispatcher{ dispatcher }
	{
		dispatcher->SessionStarted(*this);
		executor = std::make_unique<std::jthread>(std::bind(&Session::ThreadEntry, this));
	}

	bool Session::SetState(State state) {
		std::unique_lock<std::mutex> guard{ lock };
		bool terminated = __state == State::Terminated;
		if (!terminated) {
			__state = state;
		}
		guard.unlock();

		notifier.notify_all();
		return !terminated;
	}

	void Session::Join() {
		std::unique_lock<std::mutex> guard{ lock };
		if (__state != State::Terminated) {
			throw "Session::Join must only be called while the Session is in the Terminated state!";
		}
		guard.unlock();

		executor->join();
	}

	void Session::Terminate() {
		std::scoped_lock<std::mutex> guard{ lock };
		__state = State::Terminated;
		socket.Close();

		dispatcher->SessionEnded(*this);
	}

	Session::State Session::GetState() {
		std::scoped_lock<std::mutex> guard{ lock };
		return __state;
	}

	void Session::ThreadEntry() {
		auto hello = Messages::TryReceive<Messages::Hello>(socket);
		if (!hello) {
			return this->Terminate();
		}

		dispatcher->ClientRegistered(*this, *hello);

		if (!Messages::Ok{}.ToNetworkMessage().Send(socket)) {
			return this->Terminate();
		}

		// We store a copy of the state so that we don't have to contend the lock.
		// We, then, are assuming that our state will not change within a single
		// iteration.
		State currentState = State::Idle;
		while ((currentState = GetState()) != State::Terminated) {
			// wait until we have a state update
			std::unique_lock guard{ lock };
			notifier.wait(guard);
			currentState = __state;
			guard.unlock();

			if (currentState == State::Streaming) {
				// start streaming (which will come back to this method
				// when the state stops being Streaming in the future)
				this->ThreadStreaming();
			}
		}

		// session termination/cleanup (occurs under lock)
		return this->Terminate();
	}

	void Session::ThreadStreaming() {
		if (!Messages::GetStreamParams{}.ToNetworkMessage().Send(socket)) {
			return this->Terminate();
		}

		auto clientParams = Messages::TryReceive<Messages::StreamParams>(socket);
		if (!clientParams) {
			return this->Terminate();
		}

		// todo: make this take into account local preferences
		Messages::InitializeStream initMessage{
			.frameRate = std::min(60l, clientParams->frameRate),
			.resolution = clientParams->resolution
		};
		if (!initMessage.ToNetworkMessage().Send(socket)) {
			return this->Terminate();
		}

		// We store a copy of the state so that we don't have to contend the lock.
		// We, then, are assuming that our state will not change within a single
		// iteration.
		State state = State::Idle;
		while ((state = GetState()) == State::Streaming) {
			auto frame = Messages::TryReceive<Messages::StreamFrame>(socket);
			if (!frame) {
				break;
			}

			// todo: interact with the frame message...
		}

		if (!Messages::EndStream{}.ToNetworkMessage().Send(socket)) {
			return this->Terminate();
		}
	}

	Session::~Session() noexcept {
		// if we are destroyed prematurely, make sure we perform a proper termination
		if (GetState() != State::Terminated) {
			Terminate();
		}
	}
}
