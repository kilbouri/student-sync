#pragma once

#include <string>
#include <optional>
#include <wx/wx.h>
#include <wx/fileconf.h>

#include "../../common/screenresolution/screenresolution.hpp"

namespace StudentSync::Server {
	struct Preferences {
		int maxConcurrentClients;
		int maxFrameRate;
		Common::ScreenResolution maxStreamResolution;
	};

	// This is a singleton because we don't want to have to re-read the configuration
	// every time we need it. It also prevents external modification of the peristent
	// config from creating unexpected application behaviour.
	class PreferencesManager {
	public:
		static PreferencesManager& GetInstance();

		/// <summary>
		/// Retrieves a reference to the current preferences.
		/// </summary>
		const Preferences& GetPreferences();

		/// <summary>
		/// Saves a new set of preferences.
		/// </summary>
		void SetPreferences(const Preferences& preferences);


		// Note: Scott Meyers mentions in his Effective Modern
		//       C++ book, that deleted functions should generally
		//       be public as it results in better error messages
		//       due to the compilers behavior to check accessibility
		//       before deleted status
		PreferencesManager(PreferencesManager const&) = delete;
		void operator=(PreferencesManager const&) = delete;

	private:
		std::optional<Preferences> preferences;

		// The file config will save the stored values when it is destroyed
		// (eg. during application shutdown)
		wxFileConfig fileConfig;

		// Private method to handle loading preferences from the file
		Preferences Load();

		// hide the default constructor
		PreferencesManager();
	};
}