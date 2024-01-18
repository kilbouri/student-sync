#pragma once

#include <vector>
#include <optional>

#include "../../win32includes.h"

namespace StudentSync::Common {
	struct DisplayCapturer {
		enum Format : uint8_t {
			BMP,
			PNG,
			JPG,
			GIF,
			TIF
		};

		static std::optional<std::vector<byte>> CaptureScreen(Format format);
	};
}