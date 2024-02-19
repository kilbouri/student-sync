#include "gdiplusutil.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <ranges>
#include <numeric>
#include <algorithm>

#include "../smarthandle/smarthandle.hpp"

// todo: we may be able to optimize using this, idk
// https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api
namespace StudentSync::Common {

	struct MonitorInfo {
		RECT displayRect;
		RECT workAreaRect;
		TCHAR deviceName[CCHDEVICENAME];
	};

	std::vector<MonitorInfo> GetMonitors() {
		using MonitorInfos = std::vector<MonitorInfo>;
		MonitorInfos infos;

		MONITORENUMPROC callback = [](HMONITOR hMonitor, HDC _2, LPRECT _3, LPARAM infosPtr) {
			static_assert(sizeof(infosPtr) == sizeof(MonitorInfos*));
			MonitorInfos* rectsPtr = reinterpret_cast<MonitorInfos*>(infosPtr);

			// We have to set the size in order for GetMonitorInfo to know which we're asking for.
			// We ask for the extended info as we want the device name.
			MONITORINFOEX thisMonitor = {};
			thisMonitor.cbSize = sizeof(thisMonitor);

			if (!GetMonitorInfo(hMonitor, &thisMonitor)) {
				return FALSE; // stops enumeration and reports error
			}

			MonitorInfo monitorInfo = {
				.displayRect = thisMonitor.rcMonitor,
				.workAreaRect = thisMonitor.rcWork,
				.deviceName = { 0 }
			};

			std::memcpy(&monitorInfo.deviceName, thisMonitor.szDevice, sizeof(TCHAR) * CCHDEVICENAME);

			rectsPtr->push_back(monitorInfo);
			return TRUE; // continue enumerating
		};

		static_assert(sizeof(&infos) == sizeof(LPARAM));
		if (!EnumDisplayMonitors(nullptr, nullptr, callback, reinterpret_cast<LPARAM>(&infos))) {
			return std::vector<MonitorInfo>{}; // empty vector since we don't know the state of our rects vector
		}

		return infos;
	}

	RECT SmallestBoundingRect(RECT const& a, RECT const& b) {
		return RECT{
			.left = std::min(a.left, b.left),
			.top = std::min(a.top, b.top),
			.right = std::max(a.right, b.right),
			.bottom = std::max(a.bottom, b.bottom),
		};
	}

	std::shared_ptr<Gdiplus::Bitmap> GDIPlusUtil::GetBitmap(int width, int height, PixelFormat pixelFormat) {
		auto bitmapPixelFormat = static_cast<std::underlying_type_t<PixelFormat>>(pixelFormat);
		return std::make_shared<Gdiplus::Bitmap>(width, height, bitmapPixelFormat);
	}

	cpp::result<std::shared_ptr<Gdiplus::Bitmap>, GDIPlusUtil::CaptureScreenError> GDIPlusUtil::CaptureScreen(
		PixelFormat pixelFormat,
		Gdiplus::Color defaultBackgroundColor,
		std::shared_ptr<Gdiplus::Bitmap> existingBitmap
	) {
		std::vector<MonitorInfo> monitors = GetMonitors();
		if (monitors.size() == 0) {
			return cpp::fail(CaptureScreenError::NoMonitorsDetected);
		}

		RECT allMonitorsBoundingRect = std::transform_reduce(
			monitors.begin() + 1, monitors.end(),
			monitors[0].displayRect,
			&SmallestBoundingRect,
			[](MonitorInfo const& mon) { return mon.displayRect; }
		);

		long minWidthLong = allMonitorsBoundingRect.right - allMonitorsBoundingRect.left;
		long minHeightLong = allMonitorsBoundingRect.bottom - allMonitorsBoundingRect.top;

		if (minWidthLong > std::numeric_limits<int>::max() || minHeightLong > std::numeric_limits<int>::max()) {
			return cpp::fail(CaptureScreenError::ScreenTooLargeToCapture);
		}

		int minWidth = static_cast<int>(minWidthLong);
		int minHeight = static_cast<int>(minHeightLong);

		// If we were provided a bitmap, we need to make sure its big enough and has the right pixel format
		if (existingBitmap) {
			if (existingBitmap->GetWidth() < minWidth || existingBitmap->GetHeight() < minHeight) {
				return cpp::fail(CaptureScreenError::ProvidedBitmapTooSmall);
			}

			if (existingBitmap->GetPixelFormat() != static_cast<PixelFormat_t>(pixelFormat)) {
				return cpp::fail(CaptureScreenError::PixelFormatMismatch);
			}
		}

		// If we were not provided a bitmap, then we need to allocate one
		// todo: restore use of EXISTING if possible
		std::shared_ptr<Gdiplus::Bitmap> targetBitmap = GDIPlusUtil::GetBitmap(minWidth, minHeight, pixelFormat);

		HBITMAP targetBitmapHandle;
		if (targetBitmap->GetHBITMAP(defaultBackgroundColor, &targetBitmapHandle) != Gdiplus::Status::Ok) {
			return cpp::fail(CaptureScreenError::UnableToObtainBitmapHandle);
		}

		for (auto const& monitor : monitors) {
			// note: I'm not sure if monitor.name should be first or second argument.
			// based on parameter names, I think second. The docs do not specify which is
			// correct. If this code becomes problematic, try putting it as the first argument
			SmartHandle<HDC> monitorDC{
				CreateDC(nullptr, monitor.deviceName, nullptr, nullptr),
				[](HDC& dc) { DeleteDC(dc); }
			};

			if (*monitorDC == nullptr) {
				return cpp::fail(CaptureScreenError::FailedToCreateMonitorDC);
			}

			SmartHandle<HDC> memoryDC{
				CreateCompatibleDC(*monitorDC),
				[](HDC& dc) { DeleteDC(dc);  }
			};

			if (*memoryDC == nullptr) {
				return cpp::fail(CaptureScreenError::FailedToCreateComptaibleDC);
			}

			const int targetX = monitor.displayRect.left - allMonitorsBoundingRect.left;
			const int targetY = monitor.displayRect.top - allMonitorsBoundingRect.top;

			const int width = monitor.displayRect.right - monitor.displayRect.left;
			const int height = monitor.displayRect.bottom - monitor.displayRect.top;

			if (BitBlt(*memoryDC, targetX, targetY, width, height, *monitorDC, 0, 0, SRCCOPY) == 0) {
				int lastError = GetLastError();
				return cpp::fail(CaptureScreenError::DesktopCopyFailed);
			}
		}

		return targetBitmap;
	}

	cpp::result<std::vector<uint8_t>, GDIPlusUtil::GetPixelDataError> GDIPlusUtil::GetPixelData(std::shared_ptr<Gdiplus::Bitmap> bitmap, PixelFormat format_) {
		Gdiplus::PixelFormat format = static_cast<PixelFormat_t>(format_);

		UINT width = bitmap->GetWidth();
		UINT height = bitmap->GetHeight();

		static_assert(sizeof(UINT) >= sizeof(int));
		constexpr UINT intmax = static_cast<UINT>(std::numeric_limits<int>::max());

		if (width > intmax || height > intmax) {
			int lastError = GetLastError();
			return cpp::fail(GetPixelDataError::BitmapTooBig);
		}

		Gdiplus::BitmapData tempBuffer{};
		Gdiplus::Rect fullRect{ 0, 0, static_cast<int>(width), static_cast<int>(height) };
		if (bitmap->LockBits(&fullRect, Gdiplus::ImageLockModeRead, format, &tempBuffer) != Gdiplus::Status::Ok) {
			int lastError = GetLastError();
			return cpp::fail(GetPixelDataError::BitmapLockFailed);
		}

		size_t bytesPerLine = 1ULL * tempBuffer.Width * (Gdiplus::GetPixelFormatSize(format) / 8);
		uint8_t* tempBufferScan0 = reinterpret_cast<uint8_t*>(tempBuffer.Scan0);
		int absStride = std::abs(tempBuffer.Stride);

		std::vector<uint8_t> data;
		data.reserve(bytesPerLine * tempBuffer.Height);

		size_t yStart = (tempBuffer.Stride < 0) ? tempBuffer.Height - 1 : 0;
		int yStep = (tempBuffer.Stride < 0) ? -1 : 1;

		for (size_t y = yStart; y < tempBuffer.Height; y += yStep) {
			const uint8_t* start = tempBufferScan0 + (y * absStride);
			data.insert(data.end(), start, start + bytesPerLine);
		}

		if (bitmap->UnlockBits(&tempBuffer) != Gdiplus::Status::Ok) {
			int lastError = GetLastError();
			return cpp::fail(GetPixelDataError::BitmapUnlockFailed);
		}

		return data;
	}

	cpp::result<std::vector<uint8_t>, GDIPlusUtil::EncodeBitmapError> GDIPlusUtil::EncodeBitmap(std::shared_ptr<Gdiplus::Bitmap> bitmap, Encoding encoding) {
		auto getFormatCLSID = [](Encoding format) ->CLSID {
			CLSID result;
			switch (format) {
				case Encoding::BMP: (void)CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &result); break;
				case Encoding::JPG: (void)CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &result); break;
				case Encoding::GIF: (void)CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &result); break;
				case Encoding::TIF: (void)CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &result); break;

				default:
				case Encoding::PNG: (void)CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &result); break;
			}

			return result;
		};

			// create stream to store encoded image data
		IStream* encodeStream_ = nullptr;
		if (!SUCCEEDED(CreateStreamOnHGlobal(nullptr, true, &encodeStream_))) {
			return cpp::fail(EncodeBitmapError::FailedToCreateStream);
		}

		SmartHandle<IStream*> encodeStream{
			std::move(encodeStream_),
			[](IStream*& stream) { stream->Release(); }
		};

		// save into stream
		CLSID encoderClsid = getFormatCLSID(encoding);
		bitmap->Save(*encodeStream, &encoderClsid);

		// get pointer to the stream data (this locks the stream)
		HGLOBAL encodeStreamHG;
		if (!SUCCEEDED(GetHGlobalFromStream(*encodeStream, &encodeStreamHG))) {
			return cpp::fail(EncodeBitmapError::FailedToGetHGlobalForStream);
		}

		uint8_t* encodedData = nullptr;
		if ((encodedData = static_cast<uint8_t*>(GlobalLock(encodeStreamHG))) == nullptr) {
			return cpp::fail(EncodeBitmapError::FailedToLockStream);
		}

		size_t numEncodedBytes = GlobalSize(encodeStreamHG);

		std::vector<byte> imageData;
		imageData.reserve(numEncodedBytes);

		std::copy(
			encodedData,
			encodedData + numEncodedBytes,
			std::back_inserter(imageData)
		);

		GlobalUnlock(encodeStreamHG);

		return imageData;
	}
}
