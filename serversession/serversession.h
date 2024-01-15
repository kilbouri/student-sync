#pragma once

#include <mutex>
#include <condition_variable>

#include "../common/socket/socket.h"

namespace StudentSync::Server {
	struct Session {
		enum class State {
			Idle,
			Streaming,
			Terminated
		};

		/// <summary>
		/// Creates a new server session. The session begins executing
		/// immediately on a new thread.
		/// </summary>
		/// <param name="socket">The TCP Socket the client is connected on. Must be moved in.</param>
		Session(TCPSocket&& socket);

		/// <summary>
		/// Thread-safely sets the state of the session, and wakes up the
		/// session's thread if required.
		/// </summary>
		/// <param name="state">The new state</param>
		void SetState(State state);

		/// <summary>
		/// Blocks until the thread the session is executing on has completed.
		/// It is an exception to call this method while the state is anything
		/// other than Terminated.
		/// </summary>
		void Join();

		// Since the executor captures `this`, the object
		// MUST NEVER be moved or copied.
		Session(const Session&) = delete;				// Copy constructor
		Session& operator=(const Session&) = delete;	// Copy assignment
		Session(Session&&) = delete;					// Move constructor
		Session& operator=(Session&&) = delete;			// Move assignment

	private:
		TCPSocket socket;
		State state;
		State lastState;

		// The lock and notifier guard `socket` and `state`
		std::mutex lock;
		std::condition_variable notifier;
		std::unique_ptr<std::jthread> executor;

		/// <summary>
		/// The entrypoint for the Session. Runs on a new thread.
		/// </summary>
		void ThreadEntry();
	};
}