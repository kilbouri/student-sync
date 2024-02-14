#include "session.hpp"

#include <functional>

using namespace StudentSync::Networking;
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

	bool Session::ToggleStreaming() {
		std::unique_lock<std::mutex> guard{ lock };

		if (__state == State::Terminated) {
			return false;
		}

		const bool wasIdle = __state == State::Idle;
		if (wasIdle) {
			__state = State::Streaming;
		}
		else {
			__state = State::Idle;
		}

		guard.unlock();
		notifier.notify_all();
		return wasIdle;
	}

	void Session::Join() {
		std::unique_lock<std::mutex> guard{ lock };
		if (__state != State::Terminated) {
			throw "Session::Join must only be called while the Session is in the Terminated state!";
		}

		guard.unlock();
		executor->join();
	}

	TCPSocket::SocketInfo Session::GetPeerInfo() const {
		return socket
			.GetPeerSocketInfo()
			.value_or(TCPSocket::SocketInfo{ .Address = "unknown", .Port = 0 });
	}

	void Session::Terminate() {
		std::unique_lock<std::mutex> guard{ lock };
		if (__state == State::Terminated) {
			return;
		}

		__state = State::Terminated;
		socket.Close();

		guard.unlock();
		dispatcher->SessionEnded(*this);
		notifier.notify_all();
	}

	Session::State Session::GetState() {
		std::scoped_lock<std::mutex> guard{ lock };
		return __state;
	}

	void Session::ThreadEntry() {
		auto hello = Message::TryReceive<Message::Hello>(socket);
		if (!hello) {
			return this->Terminate();
		}

		dispatcher->ClientRegistered(*this, *hello);

		if (!Message::Ok{}.ToTLVMessage().Send(socket)) {
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
		if (!Message::GetStreamParams{}.ToTLVMessage().Send(socket)) {
			return this->Terminate();
		}

		// absorb all remaining StreamFrames, and store the next non-streamframe message
		std::optional<TLVMessage> nextMessage;
		while (true) {
			nextMessage = TLVMessage::TryReceive(socket);
			if (nextMessage && nextMessage->tag != TLVMessage::Tag::StreamFrame) {
				break;
			}
		}

		// all StreamFrames absorbed, we can now expect the next message must be a StreamParams
		auto clientParams = Message::StreamParams::FromTLVMessage(*nextMessage);
		if (!clientParams) {
			return this->Terminate();
		}

		// todo: make this take into account local preferences
		Message::InitializeStream initMessage{
			.frameRate = std::min(60l, clientParams->frameRate),
			.resolution = clientParams->resolution
		};
		if (!initMessage.ToTLVMessage().Send(socket)) {
			return this->Terminate();
		}

		// We store a copy of the state so that we don't have to contend the lock.
		// We, then, are assuming that our state will not change within a single
		// iteration.
		State state = State::Idle;
		while ((state = GetState()) == State::Streaming) {
			auto frame = Message::TryReceive<Message::StreamFrame>(socket);
			if (!frame) {
				break;
			}

			dispatcher->ClientFrameReceived(*this, *frame);
		}

		if (!Message::EndStream{}.ToTLVMessage().Send(socket)) {
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
