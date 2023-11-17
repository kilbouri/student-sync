#pragma once

#include <functional>

// Provides a mechanism to manage unmanaged objects/handles with
// a custom deleter. Provides implicit conversion to the managed value.
template<typename TValue>
struct ManagedHandle {
public:
	using DeleterFuncVoid = std::function<void(TValue)>;

	ManagedHandle(TValue value, DeleterFuncVoid deleter) : value{ value }, deleter{ deleter } {}
	~ManagedHandle() { deleter(this->value); }

	operator TValue() { return this->value; }

private:
	TValue value;
	DeleterFuncVoid deleter;
};