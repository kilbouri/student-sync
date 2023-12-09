#pragma once

#include <string>

#include <wx/config.h>
#include <wx/fileconf.h>

#include "../constants/constants.h";

// list of keys which contain server preferences in the global config file
#define SERVER_PREFS(x) \
	x(DefaultPort)

// list of keys which contain client preferences in the global config file
#define CLIENT_PREFS(x) \
	x(Username) \
	x(ServerIp) \
	x(ServerPort)

// general macro to generate what are effectively string enum values
#define MAKE_PREFERENCES_ENTRY(prefix, name) \
	static constexpr char name[] = prefix "_" #name;

// specializations of the above macro to effectively namespace the keys
#define MAKE_CLIENT_PREFERENCE_ENTRY(name) MAKE_PREFERENCES_ENTRY("Client", name)
#define MAKE_SERVER_PREFERENCE_ENTRY(name) MAKE_PREFERENCES_ENTRY("Server", name)

namespace Config {
	inline static wxConfigBase* GetLocalConfig() {
		static wxFileConfig defaultConfig{
			Constants::AppName,
			Constants::VendorName,
			"local.conf",
			wxEmptyString,
			wxCONFIG_USE_LOCAL_FILE
		};
		return &defaultConfig;
	}

	inline static wxConfigBase* GetGlobalConfig() {
		static wxFileConfig defaultConfig{
			Constants::AppName,
			Constants::VendorName,
			wxEmptyString,
			"global.conf",
			wxCONFIG_USE_GLOBAL_FILE
		};

		return &defaultConfig;
	}

	namespace Keys {
		struct Client {
			CLIENT_PREFS(MAKE_CLIENT_PREFERENCE_ENTRY)
		};

		struct Server {
			SERVER_PREFS(MAKE_SERVER_PREFERENCE_ENTRY)
		};
	}
}

// This ain't your header, clean up after yo' stupid self
#undef MAKE_PREFERENCES_ENTRY
#undef MAKE_CLIENT_PREFERENCE_ENTRY
#undef MAKE_SERVER_PREFERENCE_ENTRY
#undef CLIENT_PREFS
#undef SERVER_PREFS