#include "socket.hpp"
#include "../../win32includes.h"

namespace StudentSync::Networking {
	Socket::Socket(SOCKET s) :
		underlyingSocket{ new SOCKET(s), [](SOCKET* s) { closesocket(*s); delete s; } }
	{}

	bool Socket::Shutdown(Direction direction) {
		if (!IsValid()) {
			return false;
		}

		int how;
		switch (direction) {
			case Direction::Both: how = SD_BOTH; break;
			case Direction::Send: how = SD_SEND; break;
			case Direction::Receive: how = SD_RECEIVE; break;
			default: return false;
		}

		return shutdown(*underlyingSocket, how) != SOCKET_ERROR;
	}

	bool Socket::Close() {
		// closing an invalid socket is vacuous.
		if (!IsValid()) {
			return true;
		}

		int rc = closesocket(*underlyingSocket);
		if (rc != SOCKET_ERROR) {
			*underlyingSocket = INVALID_SOCKET;
		}

		return rc != SOCKET_ERROR;
	}

	int Socket::WriteBytes(const uint8_t* buffer, int nBytes) {
		return send(*underlyingSocket, reinterpret_cast<const char*>(buffer), nBytes, 0);
	}

	int Socket::ReadBytes(uint8_t* buffer, int nBytes) {
		return recv(*underlyingSocket, reinterpret_cast<char*>(buffer), nBytes, 0);
	}

	Socket::IOResult Socket::WriteAllBytes(const uint8_t* buffer, size_t nBytes) {
		size_t bytesSent = 0;

		do {
			// We can write any number of bytes in range [remaining, INT_MAX] at once
			size_t remaining = nBytes - bytesSent;
			int writableBytes = static_cast<int>(std::min<size_t>(remaining, std::numeric_limits<int>::max()));

			int writeResult = WriteBytes(buffer + bytesSent, writableBytes);
			if (writeResult == SOCKET_ERROR) {
				switch (GetLastError()) {
					case WSAECONNRESET:
					case WSAESHUTDOWN:
					case WSAECONNABORTED:
						this->Close();
						return IOResult::ConnectionClosed;

					default:
						return IOResult::Error;
				}
			}

			bytesSent += writeResult;
		} while (bytesSent < nBytes && IsValid());

		return IOResult::Success;
	}

	Socket::IOResult Socket::ReadAllBytes(uint8_t* buffer, size_t nBytes) {
		size_t bytesRead = 0;

		do {
			// We can read any number of bytes in range [remaining, INT_MAX] at once
			size_t remaining = nBytes - bytesRead;
			int readableBytes = static_cast<int>(std::min<size_t>(remaining, std::numeric_limits<int>::max()));

			int readResult = ReadBytes(buffer + bytesRead, readableBytes);
			int lastError = WSAGetLastError();
			if (readResult == 0 || (readResult == SOCKET_ERROR && lastError == WSAECONNRESET)) {
				this->Close();
				return IOResult::ConnectionClosed;
			}
			else if (readResult == SOCKET_ERROR) {
				return IOResult::Error;
			}

			bytesRead += readResult;
		} while (bytesRead < nBytes && IsValid());

		return IOResult::Success;
	}

	const SOCKET Socket::GetDescriptor() const {
		return *underlyingSocket;
	}

	bool Socket::IsValid() const {
		return underlyingSocket && *underlyingSocket != INVALID_SOCKET;
	}
}