#include "serverwindow.h"

#include "../common/message/message.h"
#include "../serverpreferencesmanager/serverpreferencesmanager.h"

using Session = ServerWindow::Session;

Session::Session(wxFrame* eventTarget, Server::Connection connection)
	: connection{ connection }
	, eventTarget{ eventTarget }
	, shouldStream{ false }
	, notifier{ std::make_shared<std::condition_variable>() }
	, sessionLock{ std::make_shared<std::mutex>() }
{
	if (eventTarget == nullptr) {
		throw "(wxFrame*)nullptr given to Session";
	}
}

void Session::RunAsync() {
	const auto registration = this->Register();
	if (!registration) {
		throw "Client failed to register";
	}

	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_REGISTERED);
	event->SetPayload(*registration);
	wxQueueEvent(eventTarget, event);

	executor = std::jthread(std::bind(&Session::ThreadJob, this));
}

void Session::StartStream() {
	// obtain lock on the session
	std::unique_lock guard{ *sessionLock };
	shouldStream = true;
	guard.release();

	// wake the thread if sleeping
	notifier->notify_all();
}

void Session::EndStream() {
	std::unique_lock guard{ *sessionLock };
	shouldStream = false;
	guard.release();

	notifier->notify_all();
}

void Session::Terminate() {
	// Obtain a lock on the session, then kill its connection and release the lock
	std::unique_lock guard{ *sessionLock };
	connection.Terminate();
	guard.release();

	// wake the thread if it sleeping, then wait for it to join
	notifier->notify_all();
	executor.join();
}

std::optional<ServerWindow::ClientInfo> Session::Register() {
	if (!this->connection.socket.IsValid()) {
		return std::nullopt;
	}

	const auto hello = TryReceiveMessage<HelloMessage>(connection.socket);
	if (!hello) {
		return std::nullopt;
	}

	if (!OkMessage{}.ToNetworkMessage().Send(connection.socket)) {
		return std::nullopt;
	}

	return ServerWindow::ClientInfo{
		.identifier = connection.identifier,
		.username = hello->username
	};
}

void Session::ThreadJob() {
	//! any cv-waiting in this thread MUST have a time limit (recommended not to exceed 10 seconds)
	//! in order to allow the connection to be cleaned up soon after the client disconnects
	using namespace std::chrono_literals;

	bool currentlyStreaming = false;

	while (connection.socket.IsValid()) {
		std::unique_lock guard{ *sessionLock };
		notifier->wait_for(guard, 5s);

		if (!currentlyStreaming && shouldStream) {
			if (!ThreadStartStream()) {
				connection.Terminate();
				continue;
			}
		}

		while (shouldStream) {
			currentlyStreaming = true;
			guard.release();

			if (!ThreadReceiveFrame()) {
				connection.Terminate();
				continue;
			}

			guard.lock();
		}

		if (currentlyStreaming && !shouldStream) {
			if (!ThreadEndStream()) {
				connection.Terminate();
				continue;
			}
		}
	}

	/*wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_DISCONNECT);
	event->SetPayload(connection.identifier);
	wxQueueEvent(eventTarget, event);*/
}

bool Session::ThreadStartStream() {
	if (!GetStreamParamsMessage{}.ToNetworkMessage().Send(connection.socket)) {
		return false;
	}

	auto reply = TryReceiveMessage<StreamParamsMessage>(connection.socket);
	if (!reply) {
		return false;
	}

	const auto& prefs = ServerPreferencesManager::GetInstance().GetPreferences();
	InitializeStreamMessage initMessage{
		.frameRate = std::min(prefs.maxFrameRate, reply->frameRate),
		.resolution = reply->resolution.ScaleToFitWithin(prefs.maxStreamResolution)
	};

	return initMessage.ToNetworkMessage().Send(connection.socket);
}

bool Session::ThreadReceiveFrame() {
	auto frameMessage = TryReceiveMessage<StreamFrameMessage>(connection.socket);
	if (!frameMessage) {
		return false;
	}

	wxMemoryInputStream imageDataStream(frameMessage->imageData.data(), frameMessage->imageData.size());

	wxImage image{ imageDataStream };
	wxBitmap bmp{ image };

	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED);
	event->SetPayload(bmp);
	wxQueueEvent(eventTarget, event);

	return true;
}

bool Session::ThreadEndStream() {
	if (!EndStreamMessage{}.ToNetworkMessage().Send(connection.socket)) {
		return false;
	}

	std::optional<NetworkMessage> message;
	while ((message = NetworkMessage::TryReceive(connection.socket)) && message->tag != NetworkMessage::Tag::Ok) {
		// *poof*
	}
}