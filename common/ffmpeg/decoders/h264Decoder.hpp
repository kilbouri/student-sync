#pragma once

#include <memory>
#include <result.hpp>

#include "../../../ffmpegincludes.h"

namespace StudentSync::Common::FFmpeg::Encoders {
	class H264Decoder {
		template <typename T>
		using ffmpeg_struct_ptr = std::unique_ptr<T, void(*)(T*)>;

		//! The relative order of these two members MUST NOT be changed! 
		//! context creation in the initializer list DEPENDS ON codec!
		const AVCodec* codec;
		ffmpeg_struct_ptr<AVCodecContext> context;

		ffmpeg_struct_ptr<SwsContext> swsContext;
		ffmpeg_struct_ptr<AVFrame> frame;
		ffmpeg_struct_ptr<AVPacket> packet;

		size_t incomingBytesPerPixel;
		size_t outgoingBytesPerPixel;
		AVPixelFormat outputFormat;

	public:
		H264Decoder(AVPixelFormat outputFormat = AVPixelFormat::AV_PIX_FMT_RGB24);

		enum class SendPacketResult {
			Success,
			OutputBufferFull, // should ReceiveFrame then try again
			Flushed, // decoder flushed and cannot receive more packets
			InvalidState, // codec is an encoder, needs a flush, etc.
			NoMemory, // legitimately exceptional errors
			Skipped, // ie. because the frame is not a keyframe and no keyframes have been received yet
		};

		SendPacketResult SendPacket(std::vector<uint8_t>& data);

		enum class ReadFrameError {
			InsufficientInput,
			FullyFlushed,
			InvalidState,
			Unknown
		};

		cpp::result<std::vector<uint8_t>, ReadFrameError> ReadFrame();
	};
}