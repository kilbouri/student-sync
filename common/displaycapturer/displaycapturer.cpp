#include "displaycapturer.h"

#include <iostream>
#include <string>
#include <memory>

#include "../managedhandle/managedhandle.h"

CLSID getFormatCLSID(DisplayCapturer::Format format);

std::optional<std::vector<char>> DisplayCapturer::CaptureScreen(DisplayCapturer::Format format) {
	int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// grab the virtual screen device context
	ManagedHandle<HDC> screenDC(GetDC(NULL), [](HDC value) { ReleaseDC(NULL, value); });

	// create a compatible memory device context and bitmap
	ManagedHandle<HDC> memoryDC(CreateCompatibleDC(screenDC), DeleteDC);
	ManagedHandle<HBITMAP> memoryBitmap(CreateCompatibleBitmap(screenDC, width, height), DeleteObject);

	// copy current screen data into our memory device
	SelectObject(memoryDC, memoryBitmap);
	BitBlt(memoryDC, screenx, screeny, width, height, screenDC, 0, 0, SRCCOPY);

	// note: at this point, the screen has been captured into an in-memory device context, which is backed by
	// memoryBitmap. If you really wanted to, you could return and work with this bitmap directly.
	// For now, we encode the bitmap into a normal image format before we return.

	Gdiplus::Bitmap bitmap(memoryBitmap, NULL);

	// create an encoding stream
	IStream* encodeStream = nullptr;
	if (!SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &encodeStream))) {
		return std::nullopt;
	}

	// encode into the stream
	CLSID encoderClsid = getFormatCLSID(format);
	if (bitmap.Save(encodeStream, &encoderClsid) != Gdiplus::Status::Ok) {
		encodeStream->Release();
		return std::nullopt;
	}

	// get memory handle for the encoding stream
	HGLOBAL encodingStreamHandle = NULL;
	if (!SUCCEEDED(GetHGlobalFromStream(encodeStream, &encodingStreamHandle))) {
		encodeStream->Release();
		return std::nullopt;
	}

	// lock the stream data
	LPVOID encodedBytes = GlobalLock(encodingStreamHandle);
	if (encodedBytes == NULL) {
		encodeStream->Release();
		return std::nullopt;
	}

	// copy encoded data into a vector
	SIZE_T numEncodedBytes = GlobalSize(encodingStreamHandle);
	std::vector<char> encodedImageData(numEncodedBytes);
	
	encodedImageData.resize(numEncodedBytes); // this sets size = numEncodedBytes
	std::memcpy(encodedImageData.data(), encodedBytes, numEncodedBytes);

	// release lock and free stream
	GlobalUnlock(encodedBytes);
	encodeStream->Release();
	
	return encodedImageData;
}

CLSID getFormatCLSID(DisplayCapturer::Format format) {
	using Format = DisplayCapturer::Format;

	CLSID result;
	switch (format) {
		// heck you Microsoft for not providing a CLSIDFromString macro that doesn't require an output pointer
		// CLSID string source: https://superkogito.github.io/blog/2020/07/26/capture_screen_using_gdiplus.html
		case Format::BMP: (void)CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::PNG: (void)CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::JPG: (void)CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::GIF: (void)CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::TIF: (void)CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		default: result = getFormatCLSID(Format::PNG); break;
	}

	return result;
}