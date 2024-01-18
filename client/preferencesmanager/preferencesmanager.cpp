#include "preferencesmanager.hpp"

#include "../../common/appinfo/appinfo.hpp"

using namespace StudentSync::Common;
namespace StudentSync::Client {

	// local file is in %AppData%
	constexpr char LocalConfigFileName[] = "studentsync.client.local.conf";
	constexpr char GlobalConfigFileName[] = "studentsync.client.global.conf";

	struct ConfigKeys {
		static constexpr char DisplayName[] = "DisplayName";
		static constexpr char MaxFrameRate[] = "MaxFrameRate";
		static constexpr char MaxFrameWidth[] = "MaxFrameWidth";
		static constexpr char MaxFrameHeight[] = "MaxFrameHeight";
	};

	PreferencesManager::PreferencesManager()
		: preferences{ std::nullopt }
		, fileConfig{ AppInfo::AppName, AppInfo::AppVendor, LocalConfigFileName, GlobalConfigFileName, wxCONFIG_USE_LOCAL_FILE }
	{}

	PreferencesManager& PreferencesManager::GetInstance() {
		// Guaranteed to be destroyed, and instantiated on first use.
		static PreferencesManager instance;
		return instance;
	}

	const Preferences& PreferencesManager::GetPreferences() {
		if (preferences == std::nullopt) {
			preferences = Load();
		}

		return *preferences;
	}

	void PreferencesManager::SetPreferences(const Preferences& preferences) {
		fileConfig.Write(ConfigKeys::DisplayName, wxString{ preferences.displayName });
		fileConfig.Write(ConfigKeys::MaxFrameRate, preferences.maxFrameRate);
		fileConfig.Write(ConfigKeys::MaxFrameWidth, preferences.maxStreamResolution.width);
		fileConfig.Write(ConfigKeys::MaxFrameHeight, preferences.maxStreamResolution.height);

		this->preferences = preferences;
	}

	Preferences PreferencesManager::Load() {
		long screenWidth = fileConfig.Read(ConfigKeys::MaxFrameWidth, 0l);
		long screenHeight = fileConfig.Read(ConfigKeys::MaxFrameHeight, 0l);
		ScreenResolution maxStreamResolution = (screenWidth > 0 && screenHeight > 0) ? ScreenResolution(screenWidth, screenHeight) : ScreenResolution::GetCurrentDisplayResolution();

		Preferences prefs{
			.displayName = fileConfig.Read(ConfigKeys::DisplayName, wxString{ "Guest" }),
			.maxFrameRate = fileConfig.Read(ConfigKeys::MaxFrameRate, 30l),
			.maxStreamResolution = maxStreamResolution,
		};

		return prefs;
	}
}