#include "screenresolution.hpp"

#include "../../win32includes.h"

namespace StudentSync::Common {
	ScreenResolution::ScreenResolution(unsigned width, unsigned height) : width{ width }, height{ height } {}

	constexpr float ScreenResolution::GetAspectRatio() const {
		return static_cast<float>(width) / height;
	}

	ScreenResolution ScreenResolution::Scale(float scale) const {
		// negative pixels makes no sense lol... and are unstorable, for that matter
		if (scale < 0) {
			scale *= -1;
		}

		return ScreenResolution{
			static_cast<unsigned>(scale * width),
			static_cast<unsigned>(scale * height)
		};
	}

	ScreenResolution ScreenResolution::ScaleToFitWithin(const ScreenResolution& other) const {
		// todo: this can end up with rounding errors (ie. 1080x1920 into 1920x1080 yields 607.5x1080),
		// meaning this may not preserve aspect ratio!
		float scaleWidth = static_cast<float>(other.width) / this->width;
		float scaleHeight = static_cast<float>(other.height) / this->height;

		float finalScale = std::min(scaleWidth, scaleHeight);
		return this->Scale(finalScale);
	}

	std::string ScreenResolution::ToString() const {
		return std::to_string(width) + "x" + std::to_string(height);
	}

	ScreenResolution ScreenResolution::Parse(const std::string& str) {
		size_t delimPosition = str.rfind('x');
		if (delimPosition == std::string::npos) {
			throw "unable to parse '" + str + "' as ScreenResolution";
		}

		std::string widthStr = str.substr(0, delimPosition);
		std::string heightStr = str.substr(delimPosition + 1);

		unsigned width = std::stoul(widthStr);
		unsigned height = std::stoul(heightStr);

		return ScreenResolution{ width, height };
	}

	ScreenResolution ScreenResolution::GetCurrentDisplayResolution() {
		int cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);

		return ScreenResolution(cx, cy);
	}
}