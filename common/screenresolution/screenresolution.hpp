#pragma once

#include <string>

namespace StudentSync::Common {
	struct ScreenResolution {
		unsigned width;
		unsigned height;

		ScreenResolution(unsigned width, unsigned height);

		constexpr float GetAspectRatio() const;
		ScreenResolution Scale(float scale) const;

		/// <summary>
		/// Attempts to preserve aspect ratio while fitting this resolution into `other`.
		/// </summary>
		ScreenResolution ScaleToFitWithin(const ScreenResolution& other) const;

		std::string ToString() const;
		static ScreenResolution Parse(const std::string& str);
		static ScreenResolution GetCurrentDisplayResolution();
	};
}