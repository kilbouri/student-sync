#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "../../networking/message/message.hpp"
#include "../../networking/socket/tcpsocket.hpp"

namespace StudentSync::Server {
	struct Session {
		struct EventDispatcher {
			/// <summary>
			/// Called after the Session is created, but before it begins executing.
			/// </summary>
			virtual void SessionStarted(Session const& session) = 0;

			/// <summary>
			/// Called after the Session has ended, but before it may be cleaned up.
			/// The state will always be Terminated and the socket will always be closed.
			/// </summary>
			virtual void SessionEnded(Session const& session) = 0;

			/// <summary>
			/// Called immediately after a client has registered.
			/// </summary>
			virtual void ClientRegistered(Session const& session, Networking::Message::Hello const& message) = 0;

			/// <summary>
			/// Called upon receipt of a video frame.
			/// </summary>
			virtual void ClientFrameReceived(Session const& session, Networking::Message::StreamFrame const& message) = 0;
		};

		enum class State {
			Idle,
			Streaming,
			Terminated
		};

		const unsigned long identifier;

		/// <summary>
		/// Creates a new server session. The session begins executing
		/// immediately on a new thread.
		/// </summary>
		/// <param name="socket">The TCP Socket the client is connected on. Must be moved in.</param>
		Session(unsigned long identifier, Networking::TCPSocket&& socket, std::shared_ptr<EventDispatcher> dispatcher);

		/// <summary>
		/// Thread-safely sets the state of the session to either Idle or Streaming,
		/// depending on its current state. Streaming -> Idle and vice versa.
		/// When the session is terminated, no change takes place and false is always returned.
		/// </summary>
		/// <returns>whether or not the session is now in the Streaming state</returns>
		bool ToggleStreaming();

		/// <summary>
		/// Thread-safely sets the state to Terminated and closes the socket.
		/// Does not automatically join the thread. The caller should call Join()
		/// at their earliest convenience afterwards.
		/// </summary>
		void Terminate();

		/// <summary>
		/// Blocks until the thread the session is executing on has completed.
		/// It is an exception to call this method while the state is anything
		/// other than Terminated. This method may block indefinitely if Terminate()
		/// has not been called previously.
		/// </summary>
		void Join();

		/// <summary>
		/// Get the socket info of the Session's client socket.
		/// </summary>
		Networking::TCPSocket::SocketInfo GetPeerInfo() const;

		// Since the executor captures `this`, the object
		// MUST NEVER be moved or copied.
		Session(const Session&) = delete;				// Copy constructor
		Session& operator=(const Session&) = delete;	// Copy assignment
		Session(Session&&) = delete;					// Move constructor
		Session& operator=(Session&&) = delete;			// Move assignment
		~Session() noexcept;
	private:
		Networking::TCPSocket socket;

		// The __ prefix here makes references to this variable uglier. Because
		// direct access is not a good idea unless you hold the session lock. So,
		// usually, you want GetState or SetState since they automatically obtain
		// a lock.
		// We could wrap this in an atomic, but then we would have to accept that
		// the state may be Terminated while the socket is not closed or vice versa.
		State __state;

		// The lock and notifier guard `socket` and `state`
		std::mutex lock;
		std::condition_variable notifier;
		std::unique_ptr<std::jthread> executor; // this is a unique_ptr so that we can late-initialize it

		std::shared_ptr<EventDispatcher> dispatcher;

		/// <summary>
		/// Thread-safe helper to read the current state of the session.
		/// </summary>
		/// <returns>A copy of the current state</returns>
		State GetState();

		/// <summary>
		/// The entrypoint for the Session. Runs on a new thread.
		/// </summary>
		void ThreadEntry();

		/// <summary>
		/// Event loop for the thread while the session is actively streaming.
		/// Runs until the state is set to anything other than Streaming. Informs
		/// the client to stop sending messages before returning. Does not absorb
		/// any extra data sent by the client after the stop message is sent.
		/// </summary>
		void ThreadStreaming();
	};
}