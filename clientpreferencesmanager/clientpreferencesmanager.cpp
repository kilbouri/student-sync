#include "clientpreferencesmanager.h"
#include "../common/appinfo/appinfo.h"

constexpr char LocalConfigFileName[] = "client.local.conf";
constexpr char GlobalConfigFileName[] = "client.global.conf";

struct ConfigKeys {
	static constexpr char DisplayName[] = "DisplayName";
	static constexpr char MaxFrameRate[] = "MaxFrameRate";
	static constexpr char MaxFrameWidth[] = "MaxFrameWidth";
	static constexpr char MaxFrameHeight[] = "MaxFrameHeight";
};

ClientPreferencesManager::ClientPreferencesManager()
	: preferences{}
	, fileConfig{ AppInfo::AppName, AppInfo::AppVendor, LocalConfigFileName, GlobalConfigFileName, wxCONFIG_USE_LOCAL_FILE }
{
	Load();
}

ClientPreferencesManager& ClientPreferencesManager::GetInstance() {
	// Guaranteed to be destroyed, and instantiated on first use.
	static ClientPreferencesManager instance;
	return instance;
}

const ClientPreferences& ClientPreferencesManager::GetPreferences()
{
	return preferences;
}

void ClientPreferencesManager::Save(const ClientPreferences& preferences) {
	fileConfig.Write(ConfigKeys::DisplayName, wxString{ preferences.displayName });
	fileConfig.Write(ConfigKeys::MaxFrameRate, preferences.maxFrameRate);
	fileConfig.Write(ConfigKeys::MaxFrameWidth, preferences.maxFrameWidth);
	fileConfig.Write(ConfigKeys::MaxFrameHeight, preferences.maxFrameHeight);

	this->preferences = preferences;
}

void ClientPreferencesManager::Load() {
	ClientPreferences prefs{
		.displayName = fileConfig.Read(ConfigKeys::DisplayName, wxString{ "Guest" }),
		.maxFrameRate = fileConfig.Read(ConfigKeys::MaxFrameRate, 30l),
		.maxFrameWidth = fileConfig.Read(ConfigKeys::MaxFrameWidth, 1920l),
		.maxFrameHeight = fileConfig.Read(ConfigKeys::MaxFrameHeight, 1080l),
	};

	this->preferences = prefs;
}