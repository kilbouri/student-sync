#pragma once

#include <format>
#include <memory>
#include <string>
#include <vector>
#include <ranges>
#include <result.hpp>

#include "../../../ffmpegincludes.h"

namespace StudentSync::Common::FFmpeg::Encoders {
	class H264Encoder {
		template <typename T>
		using ffmpeg_struct_ptr = std::unique_ptr<T, void(*)(T*)>;

		int incomingBytesPerPixel;
		int fps;

		int64_t currentFrameIndex;

		//! The relative order of these two members MUST NOT be changed! 
		//! context creation in the initializer list DEPENDS ON codec!
		const AVCodec* codec;
		ffmpeg_struct_ptr<AVCodecContext> context;

		ffmpeg_struct_ptr<SwsContext> swsContext;
		ffmpeg_struct_ptr<AVFrame> frame;
		ffmpeg_struct_ptr<AVPacket> packet;

	public:
		H264Encoder(int width, int height, int fps, AVPixelFormat incomingFormat = AVPixelFormat::AV_PIX_FMT_RGB24);

		enum class SendFrameResult {
			Success,
			FrameNotWritable,
			ImageDataTooSmall,
			ImageDataTooLarge,
			OutputBufferFull,
			Flushed,
			InvalidState, // is a decoder, needs a flush, refcounted frames aren't set, etc.
			NoMemory,
			UnknownError
		};

		SendFrameResult SendFrame(std::vector<uint8_t> const& imageData, bool forceKeyframe);

		enum class ReceiveFrameError {
			InsufficientInput,
			FullyFlushed,
			InvalidState,
			Unknown
		};

		cpp::result<std::vector<uint8_t>, ReceiveFrameError> ReceivePacket();

		enum class FlushResult {
			Success,
			InsufficientInput,
			FullyFlushed,
			InvalidState,
			Unknown,
		};

		FlushResult Flush();

		// THIS IS A TEMPORARY BODGING MEASURE! DO NOT KEEP!
		AVCodecContext* GetCodecContext() { return context.get(); }
		AVPacket* ReceievePacketRaw() {
			switch (avcodec_receive_packet(context.get(), packet.get())) {
				case 0:
					// copy data into a vector
					return packet.get();
				default:
					return nullptr;
			}
		}
	};
}