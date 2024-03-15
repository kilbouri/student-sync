#pragma once

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libavutil/pixdesc.h>
	#include <libavdevice/avdevice.h>
	#include <libswscale/swscale.h>
}

#include <format>

// util function to compute num bytes per pixel
static inline int getBitsPerPixel(AVPixelFormat format) {
	const AVPixFmtDescriptor* descriptor = av_pix_fmt_desc_get(format);
	if (!descriptor) {
		throw std::format("Unknown pixel format {}", static_cast<std::underlying_type_t<AVPixelFormat>>(format));
	}

	return av_get_bits_per_pixel(descriptor);
}