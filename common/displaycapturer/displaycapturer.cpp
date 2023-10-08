#include "displaycapturer.h"

#include <iostream>
#include <string>
#include <memory>

BITMAPINFOHEADER createBitmapHeader(int width, int height);
CLSID getFormatCLSID(DisplayCapturer::Format format);

std::vector<char> DisplayCapturer::CaptureScreen(DisplayCapturer::Format format) {

	int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	HDC screenDC = GetDC(NULL);
	HDC screenCompatibleDC = CreateCompatibleDC(screenDC);

	HBITMAP screenBitmap = CreateCompatibleBitmap(screenDC, width, height);
	BITMAPINFOHEADER screenBitmapHeader = createBitmapHeader(width, height);

	SelectObject(screenCompatibleDC, screenBitmap);

	// figure out how big our image is gonna be
	int bitsPerRow = width * screenBitmapHeader.biBitCount;
	int dwordsPerRow = (bitsPerRow + 31) / 32; // must be 32-bit aligned for GetDIBits below
	int bytesPerRow = dwordsPerRow * 4;
	int totalImageBytes = bytesPerRow * height;

	// allocate a locked global object to contain the image
	HANDLE bitmapBufferHandle = GlobalAlloc(GHND, totalImageBytes);
	if (bitmapBufferHandle == 0) {
		std::cout << "Failed to allocate bitmap buffer";
		throw "Failed to allocate bitmap buffer";
	}

	char* bitmapBuffer = (char*)GlobalLock(bitmapBufferHandle);
	if (bitmapBuffer == 0) {
		std::cout << "Failed to acquire bitmap buffer";
		throw "Failed to acquire bitmap buffer";
	}

	// copy screen into memory device, then image data into buffer
	BitBlt(screenCompatibleDC, screenx, screeny, width, height, screenDC, 0, 0, SRCCOPY);
	GetDIBits(screenCompatibleDC, screenBitmap, 0, height, bitmapBuffer, (BITMAPINFO*)&screenBitmapHeader, DIB_RGB_COLORS);

	// clean up capture handles
	DeleteDC(screenCompatibleDC);
	ReleaseDC(NULL, screenDC);

	// create a stream to store the encoded image
	IStream* encodingStream = nullptr;
	HRESULT streamCreateResult = CreateStreamOnHGlobal(NULL, TRUE, &encodingStream);
	if (!SUCCEEDED(streamCreateResult)) {
		std::cout << "Failed to create image encoding stream";
		throw "Failed to create image encoding stream";
	}

	// save the bitmap into the stream in the appropriate format
	CLSID formatClsid = getFormatCLSID(format);
	Gdiplus::Status encodeStatus = Gdiplus::Bitmap(screenBitmap, NULL).Save(encodingStream, &formatClsid, NULL); 
	if (encodeStatus != Gdiplus::Status::Ok) {
		std::cout << "Failed to save image in format " << format << "\n";
		throw "Failed to save image in format " + format;
	}

	// get memory handle for the encoding stream
	HGLOBAL encodingStreamHandle = NULL;
	HRESULT fetchEncodingStreamHandleResult = GetHGlobalFromStream(encodingStream, &encodingStreamHandle);
	if (!SUCCEEDED(fetchEncodingStreamHandleResult)) {
		std::cout << "Failed to fetch encoding stream handle";
		throw "Failed to fetch encoding stream handle";
	}

	int encodedBytes = GlobalSize(encodingStreamHandle);
	std::vector<char> encodedImageData(encodedBytes);

	// lock
	LPVOID encodedDataPointer = GlobalLock(encodingStreamHandle);
	if (encodedDataPointer == NULL) {
		int lastError = GetLastError();
		std::cout << "Failed to lock encoded data with error " << lastError;
		throw "Failed to lock encoded data with error " + lastError;
	}

	// copy
	encodedImageData.resize(encodedBytes); // this will do nothing other than set size=encodedBytes
	std::memcpy(encodedImageData.data(), encodedDataPointer, encodedBytes);

	// unlock
	GlobalUnlock(encodingStreamHandle);
	encodedDataPointer = nullptr;

	// release the last few handles/allocated memory
	encodingStream->Release();
	DeleteObject(screenBitmap);
	
	return encodedImageData;
}

BITMAPINFOHEADER createBitmapHeader(int width, int height) {
	return BITMAPINFOHEADER{
		.biSize = sizeof(BITMAPINFOHEADER),
		.biWidth = width,
		.biHeight = height,
		.biPlanes = 1,
		.biBitCount = 32,
		.biCompression = BI_RGB,
		.biSizeImage = 0,
		.biXPelsPerMeter = 0,
		.biYPelsPerMeter = 0,
		.biClrUsed = 0,
		.biClrImportant = 0,
	};
}

CLSID getFormatCLSID(DisplayCapturer::Format format) {
	using Format = DisplayCapturer::Format;

	CLSID result;
	switch (format) {
		// heck you Microsoft for not providing a CLSIDFromString macro that doesn't require an output pointer
		// CLSID string source: https://superkogito.github.io/blog/2020/07/26/capture_screen_using_gdiplus.html
		case Format::BMP: CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::PNG: CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::JPG: CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::GIF: CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &result); break;
		case Format::TIF: CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &result); break;
	}

	return result;
}