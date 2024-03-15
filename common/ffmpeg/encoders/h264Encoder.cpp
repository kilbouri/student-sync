#include "h264Encoder.hpp"

template <typename T>
static inline constexpr T* throwIfNull(T* ptr, std::string_view error) {
	if (!ptr) {
		throw error;
	}

	return ptr;
}

namespace StudentSync::Common::FFmpeg::Encoders {
	H264Encoder::H264Encoder(int width, int height, int fps, AVPixelFormat incomingFormat)
		: codec{
			throwIfNull(avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264), "Failed to find h264 codec")
		}
		, context{
			throwIfNull(avcodec_alloc_context3(codec), "Failed to allocate AVCodecContext for h264 codec"),
			[](AVCodecContext* context) { avcodec_free_context(&context); }
		}
		, swsContext{
			throwIfNull(sws_getContext(
				width, height, incomingFormat,						// input
				width, height, AVPixelFormat::AV_PIX_FMT_YUV420P,	// output
				0, nullptr, nullptr, nullptr),						// idk what these do
			"Failed to get SwsContext"),
			[](SwsContext* context) { sws_freeContext(context);  }
		}
		, frame{
			throwIfNull(av_frame_alloc(), "Failed to allocate AVFrame"),
			[](AVFrame* frame) { av_frame_free(&frame); }
		}
		, packet{
			throwIfNull(av_packet_alloc(), "Failed to allocate AVPacket"),
			[](AVPacket* packet) { av_packet_free(&packet); }
		}
		, incomingBytesPerPixel{ getBitsPerPixel(incomingFormat) / 8 }
		, fps{ fps }
		, currentFrameIndex{ 0 }
	{
		// See context options here: https://medium.com/@restlessladder/real-time-h-265-video-stream-encoding-with-c-and-libavcodec-77eafc20fa9e#da0a

		// this is the only supported pixel format of h264/5
		context->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;

		// incoming frame dimensions
		context->width = width;
		context->height = height;

		context->time_base = AVRational{ 1, fps };
		context->framerate = AVRational{ fps, 1 };

		// Setting `tune` to `zerolatency` removes delay between an input frame and output packet
		av_opt_set(context->priv_data, "tune", "zerolatency", 0);

		// We can finally open the context!
		if (avcodec_open2(context.get(), codec, nullptr) < 0) {
			throw "Failed to open AVCodecContext";
		}

		// prepare frame
		frame->format = context->pix_fmt;
		frame->width = context->width;
		frame->height = context->height;
		if (av_frame_get_buffer(frame.get(), 0) < 0) {
			throw std::format("Failed to allocate AVFrame buffer ({}x{}, YUV240P)", width, height);
		}
	}

	H264Encoder::SendFrameResult H264Encoder::SendFrame(std::vector<uint8_t> const& image, bool forceKeyframe) {
		// We need to ensure the frame is writable
		if (av_frame_make_writable(frame.get()) < 0) {
			return SendFrameResult::FrameNotWritable;
		}

		int expectedByteCount = incomingBytesPerPixel * context->width * context->height;
		if (image.size() < expectedByteCount) {
			return SendFrameResult::ImageDataTooSmall;
		}
		else if (image.size() > expectedByteCount) {
			return SendFrameResult::ImageDataTooLarge;
		}

		// The following both converts from the incoming pixel format
		// to YUV420P, and populates the frame's buffer.
		uint8_t const* const sourceSlice[1] = { image.data() };
		int sourceStride[1] = { incomingBytesPerPixel * context->width };

		[[maybe_unused]]
		int outputSliceHeight = sws_scale(swsContext.get(),
			sourceSlice, sourceStride, 0, context->height,	// incoming data info
			frame->data, frame->linesize					// outgoing data info
		);

		// Prepare some final frame information for the encoder
		frame->pts = currentFrameIndex;
		if (forceKeyframe) {
			frame->key_frame = 1;
			frame->pict_type = AVPictureType::AV_PICTURE_TYPE_I;
		}

		// We may now send the frame to the encoder
		switch (avcodec_send_frame(context.get(), frame.get())) {
			case 0:
				// we know the denominator of framerate is 1, so we don't need to worry about it
				currentFrameIndex++;
				return SendFrameResult::Success;

			case AVERROR(EAGAIN): return SendFrameResult::OutputBufferFull;
			case AVERROR(EOF): return SendFrameResult::Flushed;
			case AVERROR(EINVAL): return SendFrameResult::InvalidState;
			case AVERROR(ENOMEM): return SendFrameResult::NoMemory;
			default: return SendFrameResult::UnknownError;
		}
	}

	cpp::result<std::vector<uint8_t>, H264Encoder::ReceivePacketError> H264Encoder::ReceivePacket() {
		std::vector<uint8_t> packetData;

		switch (avcodec_receive_packet(context.get(), packet.get())) {
			case 0:
				// copy data into a vector
				packetData.reserve(packet->size);
				std::copy(packet->data, packet->data + packet->size, std::back_inserter(packetData));

				av_packet_unref(packet.get());
				return packetData;

			case AVERROR(EAGAIN): return cpp::fail(ReceivePacketError::InsufficientInput);
			case AVERROR(EOF): return cpp::fail(ReceivePacketError::FullyFlushed);
			case AVERROR(EINVAL): return cpp::fail(ReceivePacketError::InvalidState);
			default: return cpp::fail(ReceivePacketError::Unknown);
		}
	}

	H264Encoder::FlushResult H264Encoder::Flush() {
		switch (avcodec_send_frame(context.get(), nullptr)) {
			case 0: return FlushResult::Success;
			case AVERROR(EAGAIN): return FlushResult::InsufficientInput; // I doubt this can happen
			case AVERROR(EOF): return FlushResult::FullyFlushed;
			case AVERROR(EINVAL): return FlushResult::InvalidState;
			default: return FlushResult::Unknown;
		}
	}
}