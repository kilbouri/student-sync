#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>

#include "../../win32includes.h"

namespace StudentSync::Networking {
	/// <summary>
	/// This serves as a thin wrapper around a platform-specific socket.
	/// </summary>
	struct Socket {
		enum class Direction {
			Send,
			Receive,
			Both
		};

		enum class IOResult {
			Success,
			ConnectionClosed,
			Error
		};

		bool IsValid() const;
		bool Shutdown(Direction direction);
		bool Close();

		int WriteBytes(const byte* buffer, int nBytes);
		int ReadBytes(byte* buffer, int nBytes);

		IOResult WriteAllBytes(const byte* buffer, size_t nBytes);
		IOResult ReadAllBytes(byte* buffer, size_t nBytes);

		/// <summary>
		/// HACK: this provides direct access to the platform-specific
		/// socket.
		/// </summary>
		/// <returns>The underlying socket</returns>
		const SOCKET GetDescriptor() const;

	protected:
		Socket(SOCKET s);
		std::shared_ptr<SOCKET> underlyingSocket;
	};
}