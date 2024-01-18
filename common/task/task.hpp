#pragma once

#include <coroutine>
#include <iostream>
#include <optional>
#include <future>

namespace StudentSync::Common {
	/// <summary>
	/// A coroutine type for lightweight, cooperative, concurrency. Tasks do not
	/// begin executing until they are explicitly resumed or awaited.
	/// </summary>
	template <typename TResult> requires (std::is_void_v<TResult> || std::movable<TResult>)
		struct Task {
		struct promise_type;
		using Handle = std::coroutine_handle<promise_type>;

		struct promise_type {
			std::optional<TResult> value;

			promise_type() {}
			~promise_type() {}

			Task get_return_object() {
				return Task{ Handle::from_promise(*this) };
			}

			std::suspend_always initial_suspend() noexcept {
				return {};
			}

			std::suspend_always final_suspend() noexcept {
				return {};
			}

			void return_value(TResult value) noexcept {
				this->value = value;
			}

			void unhandled_exception() { }
		};

		Handle handle;
		Task(Handle handle) : handle{ handle } {}

		// no copying
		Task(const Task&) = delete;
		Task& operator=(const Task&) = delete;

		Task(Task&& s) noexcept : handle(s.handle) {
			s.handle = nullptr;
		}

		Task& operator=(Task&& s) noexcept {
			handle = s.handle;
			s.handle = nullptr;
			return *this;
		}

		#pragma region enable using co_await within
		bool await_ready() {
			return handle.done();
		}

		void await_suspend(std::coroutine_handle<> awaiterCoroutine) {
			handle.resume(); // resume ourself
			awaiterCoroutine.resume(); // resume the awaiter
		}

		TResult await_resume() {
			return handle.promise().value.value();
		}
		#pragma endregion

		#pragma region control helpers
		void Resume() {
			handle.resume();
		}

		bool Done() {
			return handle.done();
		}
		#pragma endregion

		~Task() {
			if (handle) {
				handle.destroy();
			}
		}
	};

	/// <summary>
	/// A coroutine type for lightweight, cooperative, concurrency. Tasks do not
	/// begin executing until they are explicitly resumed or awaited.
	/// </summary>
	template <>
	struct Task<void> {
		struct promise_type;
		using Handle = std::coroutine_handle<promise_type>;

		struct promise_type {
			promise_type() {}
			~promise_type() {}

			Task<void> get_return_object() {
				Task<void> retval{ Handle::from_promise(*this) };
				return retval;
			}

			std::suspend_always initial_suspend() noexcept {
				return {};
			}

			std::suspend_always final_suspend() noexcept {
				return {};
			}

			void return_void() noexcept {}
			void unhandled_exception() { throw; }
		};

		Handle handle;
		Task(Handle handle) : handle{ handle } {}

		// no copying
		Task(const Task&) = delete;
		Task& operator=(const Task&) = delete;

		Task(Task&& s) noexcept : handle(s.handle) {
			s.handle = nullptr;
		}

		Task& operator=(Task&& s) noexcept {
			handle = s.handle;
			s.handle = nullptr;
			return *this;
		}
		#pragma region enable using co_await within
		bool await_ready() {
			return handle.done();
		}

		void await_suspend(std::coroutine_handle<> awaiterCoroutine) {
			handle.resume(); // resume ourself
			awaiterCoroutine.resume(); // resume the awaiter
		}

		void await_resume() {}
		#pragma endregion

		#pragma region control helpers
		void Resume() {
			if (!await_ready()) {
				handle.resume();
			}
		}

		bool Done() {
			return handle.done();
		}
		#pragma endregion

		~Task() {
			if (handle) {
				handle.destroy();
			}
		}
	};
}