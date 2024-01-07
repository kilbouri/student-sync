#include "serverpreferencesmanager.h"
#include "../common/appinfo/appinfo.h"

// local file is in %AppData%
constexpr char LocalConfigFileName[] = "studentsync.server.local.conf";
constexpr char GlobalConfigFileName[] = "studentsync.server.global.conf";

struct ConfigKeys {
	static constexpr char MaxConcurrentClients[] = "MaxConcurrentClients";
	static constexpr char MaxFrameRate[] = "MaxFrameRate";
	static constexpr char MaxFrameWidth[] = "MaxFrameWidth";
	static constexpr char MaxFrameHeight[] = "MaxFrameHeight";
};

ServerPreferencesManager::ServerPreferencesManager()
	: preferences{ std::nullopt }
	, fileConfig{ AppInfo::AppName, AppInfo::AppVendor, LocalConfigFileName, GlobalConfigFileName, wxCONFIG_USE_LOCAL_FILE }
{}

ServerPreferencesManager& ServerPreferencesManager::GetInstance() {
	// Guaranteed to be destroyed, and instantiated on first use.
	static ServerPreferencesManager instance;
	return instance;
}

const ServerPreferences& ServerPreferencesManager::GetPreferences() {
	if (preferences == std::nullopt) {
		preferences = Load();
	}

	return *preferences;
}

void ServerPreferencesManager::SetPreferences(const ServerPreferences& preferences) {
	fileConfig.Write(ConfigKeys::MaxConcurrentClients, preferences.maxConcurrentClients);
	fileConfig.Write(ConfigKeys::MaxFrameRate, preferences.maxFrameRate);
	fileConfig.Write(ConfigKeys::MaxFrameWidth, preferences.maxStreamResolution.width);
	fileConfig.Write(ConfigKeys::MaxFrameHeight, preferences.maxStreamResolution.height);

	this->preferences = preferences;
}

ServerPreferences ServerPreferencesManager::Load() {
	long screenWidth = fileConfig.Read(ConfigKeys::MaxFrameWidth, 0l);
	long screenHeight = fileConfig.Read(ConfigKeys::MaxFrameHeight, 0l);
	ScreenResolution maxStreamResolution = (screenWidth > 0 && screenHeight > 0) ? ScreenResolution(screenWidth, screenHeight) : ScreenResolution::GetCurrentDisplayResolution();

	ServerPreferences prefs{
		.maxConcurrentClients = fileConfig.Read(ConfigKeys::MaxConcurrentClients, std::numeric_limits<int>::max() - 1),
		.maxFrameRate = fileConfig.Read(ConfigKeys::MaxFrameRate, 30l),
		.maxStreamResolution = maxStreamResolution,
	};

	return prefs;
}