
#include "window.hpp"

#include <format>
#include <wx/mstream.h>
#include <fstream>
#include <algorithm>
#include <iterator>

#include "../../common/timer/timer.hpp"
#include "../../common/screenresolution/screenresolution.hpp"
#include "../../common/ffmpeg/encoders/h264Encoder.hpp"
#include "../../common/gdiplusutil/gdiplusutil.hpp"
#include "../preferenceseditor/preferenceseditor.hpp"

using namespace StudentSync::Common;

namespace StudentSync::Client {
	void RecordClip(int seconds, int fps, std::string path) {
		using FFmpeg::Encoders::H264Encoder;
		constexpr auto format = GDIPlusUtil::PixelFormat::RGB_24bpp;

		auto desktop = GDIPlusUtil::CaptureScreen(format);
		if (!desktop) {
			wxLogFatalError("Failed to capture screen");
			return;
		}

		const AVOutputFormat* outFormat = av_guess_format("mp4", nullptr, nullptr);
		if (!outFormat) {
			wxLogFatalError("Failed to guess output format for *.mp4");
			return;
		}

		AVFormatContext* formatContext = nullptr;
		if (avformat_alloc_output_context2(&formatContext, outFormat, nullptr, nullptr) < 0) {
			wxLogFatalError("Failed to create format context");
			return;
		}

		AVStream* stream = avformat_new_stream(formatContext, nullptr);
		if (!stream) {
			wxLogFatalError("Error creating new video stream");
			return;
		}

		const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!codec) {
			wxLogFatalError("Failed to find H264 codec");
			return;
		}

		AVPixelFormat ffmpegFormat = (std::endian::native == std::endian::little)
			? AVPixelFormat::AV_PIX_FMT_BGR24
			: AVPixelFormat::AV_PIX_FMT_RGB24;

		H264Encoder encoder{
			static_cast<int>((*desktop)->GetWidth()),
			static_cast<int>((*desktop)->GetHeight()),
			fps, ffmpegFormat
		};

		stream->codecpar->codec_id = codec->id;
		stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		stream->codecpar->format = AV_PIX_FMT_YUV420P; // Adjust according to your input video format
		stream->codecpar->width = static_cast<int>((*desktop)->GetWidth()); // Adjust according to your input video width
		stream->codecpar->height = static_cast<int>((*desktop)->GetHeight()); // Adjust according to your input video height

		if (avcodec_parameters_to_context(encoder.GetCodecContext(), stream->codecpar) < 0) {
			wxLogFatalError("Failed to copy codec parameters to codec context");
			return;
		}

		if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
			if (avio_open(&formatContext->pb, path.c_str(), AVIO_FLAG_WRITE) < 0) {
				wxLogFatalError("Error opening output file");
				return;
			}
		}

		if (avformat_write_header(formatContext, nullptr) < 0) {
			wxLogFatalError("Error writing header");
			return;
		}

		const auto writeAllPackets = [&]() {
			AVPacket* packet;
			while ((packet = encoder.ReceievePacketRaw()) != nullptr) {
				packet->stream_index = stream->index;
				av_packet_rescale_ts(packet, encoder.GetCodecContext()->time_base, stream->time_base);

				if (av_interleaved_write_frame(formatContext, packet) < 0) {
					wxLogFatalError("Error writing packet");
					return;
				}

				av_packet_unref(packet);
			}
		};

		for (int i = 0; i < fps * seconds; ++i) {
			auto startTime = std::chrono::steady_clock::now();

			desktop = GDIPlusUtil::CaptureScreen(format, Gdiplus::Color::Black, desktop.value());

			auto pixelData = GDIPlusUtil::GetPixelData(*desktop, format);
			if (!pixelData) {
				wxLogFatalError("Failed to obtain pixel data from last screen capture. Error code: {}", static_cast<std::underlying_type_t<GDIPlusUtil::CaptureScreenError>>(pixelData.error()));
				return;
			}

			auto sendFrameResult = encoder.SendFrame(*pixelData, true);
			if (sendFrameResult != H264Encoder::SendFrameResult::Success) {
				wxLogFatalError(std::format("Failed to send frame to encoder. Error code: {}", static_cast<std::underlying_type_t<H264Encoder::SendFrameResult>>(sendFrameResult)).c_str());
				return;
			}

			writeAllPackets();

			auto endTime = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
			auto remaining = std::chrono::milliseconds(1000 / fps) - elapsed;

			if (remaining > std::chrono::milliseconds(0)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(remaining));
			}
		}

		encoder.Flush();
		writeAllPackets();

		av_write_trailer(formatContext);
		avformat_free_context(formatContext);

		wxLogInfo("Test capture completed");
	}

	void Window::OnTestFFmpegEncode(wxCommandEvent& event) {
		wxFileDialog saveLocationPrompt{
			this, "Save Video",
			wxEmptyString, wxEmptyString,
			"mp4 files (*.mp4)|*.mp4", wxFD_SAVE | wxFD_OVERWRITE_PROMPT
		};

		if (saveLocationPrompt.ShowModal() != wxID_OK) {
			return;
		}

		// this is fucking stupid
		std::string path{ saveLocationPrompt.GetPath().c_str() };
		std::thread{ &RecordClip, 10, 30, path }.detach();
	}

	void Window::OnShowPreferences(wxCommandEvent& event) {
		PreferencesEditor editor{ PreferencesManager::GetInstance().GetPreferences(), this };
		editor.ShowModal();
	}

	void Window::OnAbout(wxCommandEvent& event) {
		using namespace Networking;

		TCPSocket::SocketInfo localConnection = client->GetClientInfo();
		TCPSocket::SocketInfo remoteConnection = client->GetRemoteInfo();

		std::string message = "";
		message += std::format("Local connection: {}:{}\n", localConnection.Address, localConnection.Port);
		message += std::format("Remote connection: {}:{}\n", remoteConnection.Address, remoteConnection.Port);
		message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

		wxMessageBox(message);
	}

	void Window::OnExit(wxCommandEvent& event) {
		Close(true);
	}

	void Window::OnClose(wxCloseEvent& event) {
		client->Stop();

		if (clientThread && clientThread->joinable()) {
			clientThread->join();
		}

		Destroy();
	}

	void Window::OnClientPushLog(wxThreadEvent& event) {
		wxString message = event.GetPayload<wxString>();
		this->GetStatusBar()->SetLabel(message);
	}

	void Window::OnRegistrationFailed(wxThreadEvent& event) {
		wxLogFatalError("Failed to register with remote server.");
		this->Close();
	}
}