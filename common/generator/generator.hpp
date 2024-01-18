#pragma once

#include <coroutine>
#include <iostream>
#include <optional>

namespace StudentSync::Common {
	template<std::movable T>
	struct Generator {
		struct promise_type;
		using Handle = std::coroutine_handle<promise_type>;

		struct promise_type {
			Generator<T> get_return_object() {
				return Generator{ Handle::from_promise(*this) };
			}

			static std::suspend_always initial_suspend() noexcept {
				return {};
			}

			static std::suspend_always final_suspend() noexcept {
				return {};
			}

			void return_void() noexcept {}

			std::suspend_always yield_value(T value) noexcept {
				current_value = std::move(value);
				return {};
			}

			// Disallow co_await in generator coroutines.
			void await_transform() = delete;

			static void unhandled_exception() { }

			std::optional<T> current_value;
		};

		explicit Generator(const Handle coroutine) : m_coroutine{ coroutine } {}

		Generator() = default;
		~Generator() {
			if (m_coroutine) {
				m_coroutine.destroy();
			}
		}

		Generator(const Generator&) = delete;
		Generator& operator=(const Generator&) = delete;

		Generator(Generator&& other) noexcept : m_coroutine{ other.m_coroutine } {
			other.m_coroutine = {};
		}

		Generator& operator=(Generator&& other) noexcept {
			if (this != &other) {
				if (m_coroutine) {
					m_coroutine.destroy();
				}

				m_coroutine = other.m_coroutine;
				other.m_coroutine = {};
			}

			return *this;
		}

		// Range-based for loop support.
		struct Iter {
			void operator++() {
				m_coroutine.resume();
			}

			const T& operator*() const {
				return *m_coroutine.promise().current_value;
			}

			bool operator==(std::default_sentinel_t) const {
				return !m_coroutine || m_coroutine.done();
			}

			explicit Iter(const Handle coroutine) : m_coroutine{ coroutine } {}

		private:
			Handle m_coroutine;
		};

		Iter begin() {
			if (m_coroutine) {
				m_coroutine.resume();
			}

			return Iter{ m_coroutine };
		}

		std::default_sentinel_t end() { return {}; }

	private:
		Handle m_coroutine;
	};
}