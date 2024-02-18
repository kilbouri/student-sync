#pragma once

#include <vector>
#include <optional>
#include <result.hpp>

#include "../../ffmpegincludes.h"
#include "../../win32includes.h"

namespace StudentSync::Common {
	struct GDIPlusUtil {
		enum class Encoding : uint8_t {
			BMP,
			PNG,
			JPG,
			GIF,
			TIF
		};

		enum class CaptureScreenError {
			ProvidedBitmapTooSmall
		};

		/// <summary>
		/// Capture the screen. You may pass in an existing bitmap (eg. one returned from
		/// a prior call to this function) in order to reuse it. The provided bitmap will
		/// be overwritten with the current screen if it is big enough.
		/// </summary>
		/// <param name="existingBmp">An optional existing bitmap. `nullptr` may be used,
		/// in which case an appropriately sized bitmap will be created for you.</param>
		/// <returns>An error if capturing fails, otherwise either the bitmap provided,
		/// or a newly created bitmap, containing the current desktop image.</returns>
		cpp::result<std::shared_ptr<Gdiplus::Bitmap>, CaptureScreenError> CaptureScreen(
			std::shared_ptr<Gdiplus::Bitmap> existingBmp = nullptr
		);
	};
}