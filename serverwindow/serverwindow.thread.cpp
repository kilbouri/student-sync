// This file defines the background thread for ServerWindow. It is #included in serverwindow.cpp
#include "serverwindow.h"

namespace StudentSync::Server {
	#define PUSH_LOG_MESSAGE(message)								    \
	{																    \
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_PUSH_LOG);	\
		event->SetPayload(wxString(message));							\
		wxQueueEvent(this, event);										\
	}

	// Despite the name of this function, it is defining the entry for the server thread.
	void* ServerWindow::Entry() {
		// yea I know we're supposed to check TestDestroy() periodically...
		// but in this case, we simply assume that if the thread is
		// expected to terminate, Server::Stop() will be called on the
		// server this thread is running.

		// set server callbacks
		PUSH_LOG_MESSAGE("Server starting...");
		server->Run();
		return 0;
	}
}