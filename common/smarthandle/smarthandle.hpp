#pragma once

/// <summary>
/// A SmartHandle can wrap any object to ensure that a
/// construction-time cleanup function is called when
/// the object goes out of scope.
/// It is the consumer's responsibility to ensure that
/// no copies are made if the wrapped type is not safely
/// copiable.
/// A reference to the wrapped object may be obtained by
/// dereferencing the SmartHandle.
/// </summary>
template <typename T>
struct SmartHandle {
	using Deleter = void(*)(T&);

	SmartHandle(T&& obj, Deleter&& deleter)
		: instance{ std::move(obj) }
		, deleter{ std::move(deleter) }
	{}

	~SmartHandle() {
		deleter(&instance);
	}

	T& operator*() {
		return instance;
	}
private:
	T instance;
	Deleter deleter;
};
