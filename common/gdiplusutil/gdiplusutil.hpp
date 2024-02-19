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

		/// <summary>
		/// An enumeration of GDI+ Pixel Formats. Enum values are
		/// of exactly the same value as the corresponding format
		/// in GDI+.
		/// </summary>
		enum class PixelFormat : Gdiplus::PixelFormat {
			Indexed_1bpp = PixelFormat1bppIndexed,
			Indexed_4bpp = PixelFormat4bppIndexed,
			Indexed_8bpp = PixelFormat8bppIndexed,
			ARGB1555_16bpp = PixelFormat16bppARGB1555,
			GrayScale_16bpp = PixelFormat16bppGrayScale,
			RGB555_16bpp = PixelFormat16bppRGB555,
			RGB565_16bpp = PixelFormat16bppRGB565,
			RGB_24bpp = PixelFormat24bppRGB,
			ARGB_32bpp = PixelFormat32bppARGB,
			PARGB_32bpp = PixelFormat32bppPARGB,
			RGB_32bpp = PixelFormat32bppRGB,
			RGB_48bpp = PixelFormat48bppRGB,
			ARGB_64bpp = PixelFormat64bppARGB,
			PARGB_64bpp = PixelFormat64bppPARGB,
		};

		using PixelFormat_t = std::underlying_type_t<PixelFormat>;

		/// <summary>
		/// Creates a new bitmap with the specified dimensions and pixel format
		/// and returns a shared pointer to the bitmap object.
		/// </summary>
		/// <param name="width">The width in pixels of the new bitmap</param>
		/// <param name="height">The height in pixels of the new bitmap</param>
		/// <param name="pixelFormat">The pxiel format for the new bitmap</param>
		/// <returns>A shared pointer that manages the newly created bitmap object</returns>
		static std::shared_ptr<Gdiplus::Bitmap> GetBitmap(int width, int height, PixelFormat pixelFormat);

		enum class CaptureScreenError {
			NoMonitorsDetected, // no monitors were detected, or an error ocurred while enumerating them
			ScreenTooLargeToCapture, // the number of pixels in either axis exceeds INT_MAX
			ProvidedBitmapTooSmall, // a bitmap was provided, but not big enough
			PixelFormatMismatch, // a bitmap was provided, but its pixel format does not match the requested pixel format
			UnableToObtainBitmapHandle, // failed to obtain an HBITMAP from the provided/created bitmap
			FailedToCreateMonitorDC, // failed to create a DC for the monitor
			FailedToCreateComptaibleDC, // failed to create a DC compatible with monitor to hold the bitmap
			FailedToSelectMonitor, // SelectObject failed to select the HBITMAP into the drawing context
			DesktopCopyFailed, // An error occurred during at least one BitBlt from the desktop to the bitmap
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
		static cpp::result<std::shared_ptr<Gdiplus::Bitmap>, CaptureScreenError> CaptureScreen(
			PixelFormat pixelFormat,
			Gdiplus::Color defaultBackgroundColor = Gdiplus::Color::Red,
			std::shared_ptr<Gdiplus::Bitmap> existingBmp = nullptr
		);

		enum class GetPixelDataError {
			BitmapTooBig, // The provided bitmap's width or height is greater than INT_MAX
			BitmapLockFailed, // Failed to lock the bitmap for reading
			BitmapUnlockFailed // Failed to unlock the bitmap after reading
		};

		static cpp::result<std::vector<uint8_t>, GetPixelDataError> GetPixelData(std::shared_ptr<Gdiplus::Bitmap> bitmap, PixelFormat format);

		enum class EncodeBitmapError {
			FailedToCreateStream,
			FailedToGetHGlobalForStream,
			FailedToLockStream
		};

		static cpp::result<std::vector<uint8_t>, EncodeBitmapError> EncodeBitmap(std::shared_ptr<Gdiplus::Bitmap> bitmap, Encoding encoding);
	};
}