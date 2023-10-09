#include "displaycapturer.h"

#include <iostream>
#include <string>
#include <memory>

#include "../managedhandle/managedhandle.h"


struct MonitorInfo {
    int index;
    RECT rect;
};

std::vector<MonitorInfo> monitors;  // Vector to store monitor information

// Callback function to enumerate monitors
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    // Create MonitorInfo struct and store information in the vector
    MonitorInfo info = {};
    info.index = static_cast<int>(monitors.size());
    info.rect = *lprcMonitor;
    monitors.push_back(info);

    return TRUE;  // Returning TRUE continues the enumeration
}

CLSID getFormatCLSID(DisplayCapturer::Format format) {
    using Format = DisplayCapturer::Format;

    CLSID result;
    switch (format) {
        case Format::BMP: (void)CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &result); break;
        case Format::PNG: (void)CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &result); break;
        case Format::JPG: (void)CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &result); break;
        case Format::GIF: (void)CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &result); break;
        case Format::TIF: (void)CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &result); break;
        default: result = getFormatCLSID(Format::PNG); break;
    }

    return result;
}

std::optional<std::vector<char>> CaptureAndEncodeMonitor(HMONITOR hMonitor, DisplayCapturer::Format format) {
    // Get monitor information
    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    if (GetMonitorInfo(hMonitor, &monitorInfo) == 0) {
        DWORD error = GetLastError(); //testing to see why there is an error
        printf("GetMonitorInfo failed with error: %d\n", error);
        return std::nullopt;
    }

    // Create a device context for the monitor
    ManagedHandle<HDC> monitorDC(CreateDC("DISPLAY", monitorInfo.szDevice, NULL, NULL), DeleteDC);


    // Calculate the width and height of the monitor
    int width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    int height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

    // Create a compatible memory device context and bitmap
    ManagedHandle<HDC> memoryDC(CreateCompatibleDC(monitorDC), DeleteDC);
    ManagedHandle<HBITMAP> memoryBitmap(CreateCompatibleBitmap(monitorDC, width, height), DeleteObject);

    // Copy current screen data into our memory device
    SelectObject(memoryDC, memoryBitmap);
    BitBlt(memoryDC, 0, 0, width, height, monitorDC, 0, 0, SRCCOPY);

    // Encode into the desired format
    Gdiplus::Bitmap bitmap(memoryBitmap, NULL);
    IStream* encodeStream = nullptr;

    if (!SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &encodeStream))) {
        DWORD error = GetLastError(); //testing to see why there is an error
        printf("CreateStreamOnHGlobal failed with error: %d\n", error);
        return std::nullopt;
    }

    CLSID encoderClsid = getFormatCLSID(format);
    if (bitmap.Save(encodeStream, &encoderClsid) != Gdiplus::Status::Ok) {
        DWORD error = GetLastError();
        printf("bitmap.Save failed with error: %d\n", error);
        encodeStream->Release();
        return std::nullopt;
    }

    // Get memory handle for the encoding stream
    HGLOBAL encodingStreamHandle = NULL;//testing to see why there is an error
    if (!SUCCEEDED(GetHGlobalFromStream(encodeStream, &encodingStreamHandle))) {
        DWORD error = GetLastError(); //testing to see why there is an error
        printf("GetHGlobalFromStream failed with error: %d\n", error);
        encodeStream->Release();
        return std::nullopt;
    }

    // Lock the stream data
    LPVOID encodedBytes = GlobalLock(encodingStreamHandle);
    if (encodedBytes == NULL) {
        DWORD error = GetLastError(); //testing to see why there is an error
        printf("GlobalLock failed with error: %d\n", error);
        encodeStream->Release();
        return std::nullopt;
    }

    // Copy encoded data into a vector
    SIZE_T numEncodedBytes = GlobalSize(encodingStreamHandle);
    std::vector<char> encodedImageData(numEncodedBytes);

    encodedImageData.resize(numEncodedBytes);
    std::memcpy(encodedImageData.data(), encodedBytes, numEncodedBytes);

    // Release lock and free stream
    GlobalUnlock(encodingStreamHandle);
    encodeStream->Release();

    return encodedImageData;
}

// Helper function to calculate the combined image width and height
void CalculateCombinedImageSize(int& combinedWidth, int& combinedHeight) {
    // Initialize with large values for minimum and maximum coordinates
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;

    // Iterate over monitors and find the bounding box
    for (const auto& monitor : monitors) {
        minX = std::min(minX, static_cast<int>(monitor.rect.left));
        minY = std::min(minY, static_cast<int>(monitor.rect.top));
        maxX = std::max(maxX, static_cast<int>(monitor.rect.right));
        maxY = std::max(maxY, static_cast<int>(monitor.rect.bottom));
    }
    // Calculate combined width and height
    combinedWidth = maxX - minX;
    combinedHeight = maxY - minY;
}

std::optional<std::vector<char>> DisplayCapturer::CaptureScreen(DisplayCapturer::Format format) {
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    // Enumerate monitors
    if (EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0)) {
        // Enumeration successful

        // Calculate the combined image size
        int combinedWidth, combinedHeight;
        int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;  // Define here
        CalculateCombinedImageSize(combinedWidth, combinedHeight);

        // Create a combined image
        Gdiplus::Bitmap combinedImage(combinedWidth, combinedHeight, PixelFormat32bppARGB);
        Gdiplus::Graphics combinedGraphics(&combinedImage);

        // Offset for drawing monitors on the combined image
        int offsetX = INT_MAX;
        int offsetY = INT_MAX;

        // Iterate over monitors and find the top-left corner of the combined image
        for (const auto& monitor : monitors) {
            offsetX = std::min(offsetX, static_cast<int>(monitor.rect.left));
            offsetY = std::min(offsetY, static_cast<int>(monitor.rect.top));
        }

        // Iterate over monitors and capture each
        for (const auto& monitor : monitors) {
            auto encodedImageData = CaptureAndEncodeMonitor(MonitorFromRect(&monitor.rect, MONITOR_DEFAULTTONULL), format);

            // Process or save the encoded image data as needed
            if (encodedImageData.has_value()) {
                const auto& imageData = encodedImageData.value();  // Extract the vector from std::optional

                // Copy data to a raw byte array
                std::vector<BYTE> rawBytes(imageData.begin(), imageData.end());

                // Create a memory stream from the raw data
                IStream* memoryStream = nullptr;
                if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &memoryStream))) {
                    memoryStream->Write(rawBytes.data(), static_cast<ULONG>(rawBytes.size()), nullptr);

                    // Load the individual monitor image
                    Gdiplus::Bitmap monitorImage(memoryStream, FALSE);

                    // Draw the monitor image onto the combined image
                    combinedGraphics.DrawImage(static_cast<Gdiplus::Image*>(&monitorImage), Gdiplus::Rect(monitor.rect.left - offsetX, monitor.rect.top - offsetY, monitorImage.GetWidth(), monitorImage.GetHeight()));

                    // Release the memory stream

                    memoryStream->Release();
                }
            }
        }

        // Save the combined image to a file
        CLSID encoderClsid = getFormatCLSID(format);
        combinedImage.Save(L"screenshot.png", &encoderClsid);
    }
    else {
        // Enumeration failed
        printf("EnumDisplayMonitors failed\n");
    }

    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return std::nullopt;/*
    for now this is just the default because my brain is melting and I tried to make it so the client did the work
    but for whatever reason that made the entire thing break so I jsut reverted to this state because it actually worked.
    */
}