#include "gdiplusutil.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <ranges>
#include <numeric>

// todo: this is incredibly unperformant and would cause a huge strain on the client device. We can absolutely do better:
// https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api
namespace StudentSync::Common {
	//struct MonitorInfo {
	//	RECT rect;
	//	TCHAR name[CCHDEVICENAME];
	//};

	//std::optional<std::vector<MonitorInfo>> GetMonitors() {
	//	// this needs to have static lifetime, since we can't capture it into the callback below
	//	static std::vector<MonitorInfo> result = std::vector<MonitorInfo>();
	//	result.clear();

	//	MONITORENUMPROC callback = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	//		MONITORINFOEX monitorInfo = {};

	//		// the size must be set to either sizeof(MONITORINFO) or sizeof(MONITORINFOEX).
	//		// since we need the device name, we need the "extended" struct which is MONITORINFOEX.
	//		monitorInfo.cbSize = sizeof(MONITORINFOEX);
	//		GetMonitorInfo(hMonitor, &monitorInfo);

	//		MonitorInfo thisMonitor = {
	//			.rect = *lprcMonitor,
	//			.name = {0},
	//		};

	//		std::memcpy(thisMonitor.name, monitorInfo.szDevice, CCHDEVICENAME);

	//		result.push_back(thisMonitor);
	//		return TRUE; // continues the enumeration
	//	};

	//	if (!EnumDisplayMonitors(nullptr, nullptr, callback, 0)) {
	//		return std::nullopt;
	//	}

	//	return result;
	//}

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

	//constexpr RECT BoundingBoxOf(RECT a, RECT b) {
	//	return RECT{
	//		.left = std::min(a.left, b.left),
	//		.top = std::min(a.top, b.top),
	//		.right = std::max(a.right, b.right),
	//		.bottom = std::max(a.bottom, b.bottom)
	//	};
	//}

	//constexpr int WidthOfRect(RECT r) {
	//	return r.right - r.left;
	//}

	//constexpr int HeightOfRect(RECT r) {
	//	return r.bottom - r.top;
	//}

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

struct Rectangle {

};

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
			return FALSE; // My guess is this stops the enumeration, but I don't know for sure.
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



namespace StudentSync::Common {
	using namespace Gdiplus;

	cpp::result<std::shared_ptr<Bitmap>, GDIPlusUtil::CaptureScreenError> GDIPlusUtil::CaptureScreen(
		std::shared_ptr<Bitmap> existingBitmap
	) {
		return existingBitmap;
	}
}