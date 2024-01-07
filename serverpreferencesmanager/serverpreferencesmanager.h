#pragma once

#include <wx/wx.h>
#include <wx/fileconf.h>
#include <string>
#include <optional>
#include "../common/screenresolution/screenresolution.h"

struct ServerPreferences {
	int maxConcurrentClients;
	int maxFrameRate;
	ScreenResolution maxStreamResolution;
};

// This is a singleton because we don't want to have to re-read the configuration
// every time we need it. It also prevents external modification of the peristent
// config from creating unexpected application behaviour.
class ServerPreferencesManager {
public:
	static ServerPreferencesManager& GetInstance();

	/// <summary>
	/// Retrieves a reference to the current preferences.
	/// </summary>
	const ServerPreferences& GetPreferences();

	/// <summary>
	/// Saves a new set of preferences.
	/// </summary>
	void SetPreferences(const ServerPreferences& preferences);


	// Note: Scott Meyers mentions in his Effective Modern
	//       C++ book, that deleted functions should generally
	//       be public as it results in better error messages
	//       due to the compilers behavior to check accessibility
	//       before deleted status
	ServerPreferencesManager(ServerPreferencesManager const&) = delete;
	void operator=(ServerPreferencesManager const&) = delete;

private:
	std::optional<ServerPreferences> preferences;

	// The file config will save the stored values when it is destroyed
	// (eg. during application shutdown)
	wxFileConfig fileConfig;

	// Private method to handle loading preferences from the file
	ServerPreferences Load();

	// hide the default constructor
	ServerPreferencesManager();
};