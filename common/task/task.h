#pragma once

#include <coroutine>
#include <iostream>
#include <optional>

template <std::movable TResult>
struct Task {
	struct promise_type;
	using Handle = std::coroutine_handle<promise_type>;

	struct promise_type {
		std::optional<TResult> value{ std::nullopt };
		bool has_value() {
			return value != std::nullopt;
		}

		promise_type() {}
		~promise_type() {}

		Task get_return_object() {
			return Task{
				Handle::from_promise(*this)
			};
		}

		std::suspend_always initial_suspend() noexcept {
			return {};
		}

		void return_value(TResult value) noexcept {
			this->value = value;
		}

		std::suspend_always final_suspend() noexcept {
			return {};
		}

		void unhandled_exception() { throw; }
	};

	Handle handle;
	Task(Handle handle) : handle{ handle } {}

	// no copying
	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

	Task(Task&& s) : handle(s.handle) {
		s.handle = nullptr;
	}

	Task& operator=(Task&& s) {
		handle = s.handle;
		s.handle = nullptr;
		return *this;
	}
#pragma region co_await
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

	~Task() {
		if (handle) {
			handle.destroy();
		}
	}
};