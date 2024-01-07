#pragma once

#include <string>

// A note on the algorithm I want to use for negotiating screen resolution in streams:
// Client sends their resolution to the server. The server then chooses the resolution based on the following:
// 0. If both parties request the same resolution, that is the resolution for the stream.
// 1. If the client requests a lower resolution than the server, that is the resolution for the stream.
// 2. If the server requests a lower resolution than the client, then the server must, preserving aspect ratio,
//    downscale the client's requested resolution to fit within the server's requested resolution. The rationale
//    here is that the way the server displays the stream can trivially cope with an aspect ratio differing from
//    the viewport, but the client would need to stretch/shrink the captured image to match the server's resolution.
//    Thus, the client's aspect ratio should be used for the stream no matter what resolution is chosen.

struct ScreenResolution {
public:
	unsigned width;
	unsigned height;

	ScreenResolution(unsigned width, unsigned height);

	constexpr float GetAspectRatio() const;
	ScreenResolution Scale(float scale) const;

	/// <summary>
	/// Attempts to preserve aspect ratio while fitting this resolution into `other`.
	/// </summary>
	ScreenResolution ScaleToFitWithin(const ScreenResolution& other) const;

	std::string ToString() const;
	static ScreenResolution Parse(const std::string& str);
	static ScreenResolution GetCurrentDisplayResolution();
};