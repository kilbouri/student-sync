#include "h264Decoder.hpp"

#include <string>
#include <format>

template <typename T>
static inline constexpr T* throwIfNull(T* ptr, std::string_view error) {
	if (!ptr) {
		throw error;
	}

	return ptr;
}

static inline size_t getBitsPerPixel(AVPixelFormat format) {
	const AVPixFmtDescriptor* descriptor = av_pix_fmt_desc_get(format);
	if (!descriptor) {
		throw std::format("Unknown pixel format {}", static_cast<std::underlying_type_t<AVPixelFormat>>(format));
	}

	return static_cast<size_t>(av_get_bits_per_pixel(descriptor));
}

namespace StudentSync::Common::FFmpeg::Encoders {
	H264Decoder::H264Decoder(AVPixelFormat outputFormat)
		: codec{
			throwIfNull(avcodec_find_decoder(AVCodecID::AV_CODEC_ID_H264), "Failed to find h264 codec")
		}
		, context{
			throwIfNull(avcodec_alloc_context3(codec), "Failed to allocate AVCodecContext for h264 codec"),
			[](AVCodecContext* context) { avcodec_free_context(&context); }
		}
		, swsContext{ nullptr, [](SwsContext* ptr) {} } // we can't init this yet
		, frame{
			throwIfNull(av_frame_alloc(), "Failed to allocate AVFrame"),
			[](AVFrame* frame) { av_frame_free(&frame); }
		}
		, packet{
			throwIfNull(av_packet_alloc(), "Failed to allocate AVPacket"),
			[](AVPacket* packet) { av_packet_free(&packet); }
		}
		, incomingBytesPerPixel{ getBitsPerPixel(AVPixelFormat::AV_PIX_FMT_YUV420P) / 8 }
		, outgoingBytesPerPixel{ getBitsPerPixel(outputFormat) / 8 }
		, outputFormat{ outputFormat }
	{
		if (avcodec_open2(context.get(), codec, nullptr) < 0) {
			throw "Failed to open AVCodecContext";
		}
	}

	H264Decoder::SendPacketResult H264Decoder::SendPacket(std::vector<uint8_t>& data) {
		packet->data = data.data();
		packet->size = static_cast<int>(data.size());

		switch (avcodec_send_packet(context.get(), packet.get())) {
			case 0: return SendPacketResult::Success;
			case AVERROR(EAGAIN): return SendPacketResult::OutputBufferFull;
			case AVERROR_EOF: return SendPacketResult::Flushed;
			case AVERROR(EINVAL): return SendPacketResult::InvalidState;
			case AVERROR(ENOMEM): return SendPacketResult::NoMemory;
			default: return SendPacketResult::Skipped;
		}
	}

	cpp::result<std::vector<uint8_t>, H264Decoder::ReadFrameError> H264Decoder::ReadFrame() {
		switch (avcodec_receive_frame(context.get(), frame.get())) {
			case 0: {
				if (!swsContext) {
					swsContext = ffmpeg_struct_ptr<SwsContext>{
						sws_getContext(
							frame->width, frame->height,
							AVPixelFormat::AV_PIX_FMT_YUV420P,
							frame->width, frame->height,
							outputFormat,
							SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
						),
						[](SwsContext* ptr) { sws_freeContext(ptr); }
					};
				}

				// Create vector to hold resulting data
				std::vector<uint8_t> result;
				result.resize(outgoingBytesPerPixel * frame->width * frame->height);

				uint8_t* dataPtr = result.data();
				int outLineSize = static_cast<int>(outgoingBytesPerPixel * frame->width);
				sws_scale(swsContext.get(), frame->data, frame->linesize, 0, frame->height, &dataPtr, &outLineSize);

				return result;
			}
			case AVERROR(EAGAIN): return cpp::fail(ReadFrameError::InsufficientInput);
			case AVERROR_EOF: return cpp::fail(ReadFrameError::FullyFlushed);
			case AVERROR(EINVAL): return cpp::fail(ReadFrameError::InvalidState);
			default: return cpp::fail(ReadFrameError::Unknown);
		}
	}
}