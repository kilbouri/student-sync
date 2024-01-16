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
		/// session's thread if required. If the session is already in the
		/// Terminated state, this method will only wake the thread, regardless
		/// of the state being changed. In that case the method will return false.
		/// </summary>
		/// <param name="state">The new state</param>
		/// <returns>true if the state change happened, otherwise false.</returns>
		bool SetState(State state);

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

		// The __ prefix here makes references to this variable uglier. Because
		// direct access is not a good idea unless you hold the session lock. So,
		// usually, you want GetState or SetState since they automatically obtain
		// a lock. Actually, if you hold a lock, you should use GetStateUnlocked.
		State __state;

		// The lock and notifier guard `socket` and `state`
		std::mutex lock;
		std::condition_variable notifier;
		std::unique_ptr<std::jthread> executor; // this is a unique_ptr so that we can late-initialize it

		/// <summary>
		/// Thread-safely sets the state to Terminated and closes the socket.
		/// </summary>
		void Terminate();

		/// <summary>
		/// Thread-safely reads the current state of the session.
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