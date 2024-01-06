#include <wx/wx.h>
#include <wx/fileconf.h>
#include <string>

struct ClientPreferences {
	std::string displayName;
	long maxFrameRate;

	/// todo: make this a "screen resolution" type
	long maxFrameWidth;
	long maxFrameHeight;
};

// This is a singleton because we don't want to have to re-read the configuration
// every time we need it. It also prevents external modification of the peristent
// config from creating unexpected application behaviour.
class ClientPreferencesManager {
public:
	static ClientPreferencesManager& GetInstance();

	/// <summary>
	/// Retrieves a reference to the current preferences.
	/// </summary>
	const ClientPreferences& GetPreferences();

	/// <summary>
	/// Saves a new set of preferences.
	/// </summary>
	void Save(const ClientPreferences& preferences);

	/// <summary>
	/// Loads the preferences from disk.
	/// </summary>
	void Load();

	// Note: Scott Meyers mentions in his Effective Modern
	//       C++ book, that deleted functions should generally
	//       be public as it results in better error messages
	//       due to the compilers behavior to check accessibility
	//       before deleted status
	ClientPreferencesManager(ClientPreferencesManager const&) = delete;
	void operator=(ClientPreferencesManager const&) = delete;

private:
	ClientPreferences preferences;

	// The file config will save the stored values when it is destroyed 
	// (eg. during application shutdown)
	wxFileConfig fileConfig;

	// hide the default constructor
	ClientPreferencesManager();
};