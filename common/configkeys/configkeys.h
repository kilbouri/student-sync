#pragma once
#include <string>

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
	static constexpr std::string_view name = prefix "_" #name;

// specializations of the above macro to effectively namespace the keys
#define MAKE_CLIENT_PREFERENCE_ENTRY(name) MAKE_PREFERENCES_ENTRY("Client", name)
#define MAKE_SERVER_PREFERENCE_ENTRY(name) MAKE_PREFERENCES_ENTRY("Server", name)

// lol here's the actual "implementation"... you probably don't need to touch it.
namespace ConfigKeys {
	struct Client {
		CLIENT_PREFS(MAKE_CLIENT_PREFERENCE_ENTRY)
	};

	struct Server {
		SERVER_PREFS(MAKE_SERVER_PREFERENCE_ENTRY)
	};
};

// This ain't your header, clean up after yo' stupid self
#undef MAKE_PREFERENCES_ENTRY
#undef MAKE_CLIENT_PREFERENCE_ENTRY
#undef MAKE_SERVER_PREFERENCE_ENTRY
#undef CLIENT_PREFS
#undef SERVER_PREFS