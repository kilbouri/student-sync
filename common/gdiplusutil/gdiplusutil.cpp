#include "gdiplusutil.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <ranges>
#include <numeric>
#include <algorithm>

#include "../smarthandle/smarthandle.hpp"

// todo: this is incredibly unperformant and would cause a huge strain on the client device. We can absolutely do better:
// https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api
namespace StudentSync::Common {
	//CLSID getFormatCLSID(DisplayCapturer::Encoding format) {
	//	using Format = DisplayCapturer::Encoding;

	//	CLSID result;
	//	switch (format) {
	//		case Encoding::BMP: (void)CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &result); break;
	//		case Encoding::PNG: (void)CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &result); break;
	//		case Encoding::JPG: (void)CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &result); break;
	//		case Encoding::GIF: (void)CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &result); break;
	//		case Encoding::TIF: (void)CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &result); break;
	//		default: result = getFormatCLSID(Encoding::PNG); break;
	//	}

	//	return result;
	//}
	// 
	//std::optional<std::vector<byte>> DisplayCapturer::CaptureScreen(DisplayCapturer::Encoding format) {
	//	std::vector<MonitorInfo> monitors;
	//	auto maybeMonitors = GetMonitors();
	//	if (!maybeMonitors || (monitors = std::move(*maybeMonitors)).size() == 0) {
	//		return std::nullopt;
	//	}

	//	// note that the bounding rect MAY have negative coordinates. This is fine, just make sure you don't assume Top/Left == 0.
	//	auto monitorRects = monitors | std::views::transform(&MonitorInfo::rect);
	//	auto boundingRect = std::reduce(monitorRects.begin(), monitorRects.end(), monitorRects.front(), BoundingBoxOf);

	//	int totalWidth = WidthOfRect(boundingRect);
	//	int totalHeight = HeightOfRect(boundingRect);

	//	HBITMAP combinedBitmap;
	//	Gdiplus::Status createStatus = Gdiplus::Bitmap(totalWidth, totalHeight, PixelFormat32bppARGB).GetHBITMAP(Gdiplus::Color::Transparent, &combinedBitmap);
	//	if (createStatus != Gdiplus::Status::Ok) {
	//		return std::nullopt;
	//	}

	//	// Deleter for the HDCs we will create below
	//	const auto hdcDeleter = [](HDC* dc) {
	//		if (dc) {
	//			DeleteDC(*dc);
	//			delete dc;
	//		}
	//	};

	//	// Capture each monitor into the combined image
	//	for (const auto& monitor : monitors) {

	//		// note: I'm not sure if monitor.name should be first or second argument.
	//		// based on parameter names, I think second. The docs do not specify which is
	//		// correct. If this code becomes problematic, try putting it as the first argument
	//		auto monitorDC = std::unique_ptr<HDC, decltype(hdcDeleter)>(new HDC(CreateDC(nullptr, monitor.name, nullptr, nullptr)), hdcDeleter);
	//		auto memoryDC = std::unique_ptr<HDC, decltype(hdcDeleter)>(new HDC(CreateCompatibleDC(*monitorDC)), hdcDeleter);

	//		SelectObject(*memoryDC, combinedBitmap);

	//		// we have to offset here because the bounding rect may have negative coordinates.
	//		// this occurs when the primary display is to the right of one or more other displays.
	//		int targetX = monitor.rect.left - boundingRect.left;
	//		int targetY = monitor.rect.top - boundingRect.top;

	//		int width = WidthOfRect(monitor.rect);
	//		int height = HeightOfRect(monitor.rect);

	//		// If this fails, it is what it is. Hopefully others will succeed.
	//		BitBlt(*memoryDC, targetX, targetY, width, height, *monitorDC, 0, 0, SRCCOPY);
	//	}

	//	// create stream to store encoded image data
	//	IStream* encodeStream = nullptr;
	//	if (!SUCCEEDED(CreateStreamOnHGlobal(nullptr, true, &encodeStream))) {
	//		DeleteObject(combinedBitmap);
	//		return std::nullopt;
	//	}

	//	// save into stream
	//	CLSID encoderClsid = getFormatCLSID(format);
	//	Gdiplus::Bitmap(combinedBitmap, nullptr).Save(encodeStream, &encoderClsid);

	//	// get pointer to the stream data (this locks the stream)
	//	HGLOBAL encodeStreamHG;
	//	void* encodedData = nullptr;
	//	if (!SUCCEEDED(GetHGlobalFromStream(encodeStream, &encodeStreamHG)) || (encodedData = GlobalLock(encodeStreamHG)) == nullptr) {
	//		encodeStream->Release();
	//		DeleteObject(combinedBitmap);
	//		return std::nullopt;
	//	}

	//	size_t numEncodedBytes = GlobalSize(encodeStreamHG);
	//	std::vector<byte> imageData(numEncodedBytes);
	//	std::memcpy(imageData.data(), encodedData, numEncodedBytes);

	//	// unlock and free stream
	//	GlobalUnlock(encodeStreamHG);
	//	encodeStream->Release();

	//	DeleteObject(combinedBitmap);

	//	return imageData;
	//}
}

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

		MONITORINFOEX monitorInfo = {};
		if (!GetMonitorInfo(hMonitor, &monitorInfo)) {
			return FALSE; // stops enumeration and reports error
		}

		MonitorInfo thisMonitor = {
			.displayRect = monitorInfo.rcMonitor,
			.workAreaRect = monitorInfo.rcWork,
			.deviceName = { 0 }
		};

		std::memcpy(&thisMonitor.deviceName, monitorInfo.szDevice, sizeof(TCHAR) * CCHDEVICENAME);
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

namespace StudentSync::Common {
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
			SmallestBoundingRect,
			&MonitorInfo::displayRect
		);

		long minWidth = allMonitorsBoundingRect.right - allMonitorsBoundingRect.left;
		long minHeight = allMonitorsBoundingRect.bottom - allMonitorsBoundingRect.top;

		if (minWidth > std::numeric_limits<int>::max() || minHeight > std::numeric_limits<int>::max()) {
			return cpp::fail(CaptureScreenError::ScreenTooLargeToCapture);
		}

		// If we were provided a bitmap, we need to make sure its big enough and has the right pixel format
		if (existingBitmap) {
			if (existingBitmap->GetWidth() < minWidth || existingBitmap->GetHeight() < minHeight) {
				return cpp::fail(CaptureScreenError::ProvidedBitmapTooSmall);
			}

			if (existingBitmap->GetPixelFormat() != static_cast<std::underlying_type_t<PixelFormat>>(pixelFormat)) {
				return cpp::fail(CaptureScreenError::PixelFormatMismatch);
			}
		}

		// If we were not provided a bitmap, then we need to allocate one
		std::shared_ptr<Gdiplus::Bitmap> targetBitmap = (existingBitmap) ? existingBitmap : GDIPlusUtil::GetBitmap(
			static_cast<int>(minWidth), static_cast<int>(minHeight),
			pixelFormat
		);


		HBITMAP targetBitmapHandle;
		if (targetBitmap->GetHBITMAP(defaultBackgroundColor, &targetBitmapHandle) != Gdiplus::Status::Ok) {
			return cpp::fail(CaptureScreenError::UnableToObtainBitmapHandle);
		}

		SmartHandle<HDC> bitmapHDC{
			CreateCompatibleDC(nullptr),
			[](HDC& dc) { DeleteDC(dc); }
		};

		// we ignore the return value as we will be deleting the DC when we are done
		SelectObject(*bitmapHDC, targetBitmapHandle);

		#if 1 // I'm not sure which of these capture implementations work properly, so one is dead and the other isn't
		for (auto const& monitor : monitors) {
			const int monWidth = monitor.displayRect.right - monitor.displayRect.left;
			const int monHeight = monitor.displayRect.bottom - monitor.displayRect.top;

			const int targetX = monitor.displayRect.left - allMonitorsBoundingRect.left;
			const int targetY = monitor.displayRect.top - allMonitorsBoundingRect.top;

			// note: I'm not sure if monitor.name should be first or second argument.
			// based on parameter names, I think second. The docs do not specify which is
			// correct. If this code becomes problematic, try putting it as the first argument
			SmartHandle<HDC> monitorHDC{
				CreateDC(nullptr, monitor.deviceName, nullptr, nullptr),
				[](HDC& dc) {DeleteDC(dc); }
			};

			if (!BitBlt(*bitmapHDC, targetX, targetY, monWidth, monHeight, *monitorHDC, 0, 0, SRCCOPY)) {
				return cpp::fail(CaptureScreenError::DesktopCopyFailed);
			}
		}
		#else
		SmartHandle<HDC> monitorHDC{
			GetDC(nullptr),
			[](HDC& dc) { DeleteDC(dc); }
		};

		const int displayWidth = allMonitorsBoundingRect.right - allMonitorsBoundingRect.left;
		const int displayHeight = allMonitorsBoundingRect.bottom - allMonitorsBoundingRect.top;

		if (!BitBlt(*bitmapHDC, 0, 0, displayWidth, displayHeight, *monitorHDC, allMonitorsBoundingRect.left, allMonitorsBoundingRect.right, SRCCOPY)) {
			return cpp::fail(CaptureScreenError::DesktopCopyFailed);
		}
		#endif

		return targetBitmap;
	}
}