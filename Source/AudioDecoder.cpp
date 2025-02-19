/*
	DiscordCoreAPI, A bot library for Discord, written in C++, and featuring explicit multithreading through the usage of custom, asynchronous C++ CoRoutines.

	Copyright 2021, 2022 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// AudioDecoder.cpp - Source file for the audio decoder class.
/// Jul 29, 2021
/// https://discordcoreapi.com
/// \file AudioDecoder.cpp

#include <discordcoreapi/Utilities.hpp>
#include <discordcoreapi/AudioDecoder.hpp>
#include <libavcodec/avcodec.h>

namespace DiscordCoreInternal {

	void OpusDecoderWrapper::OpusDecoderDeleter::operator()(OpusDecoder* other) noexcept {
		if (other) {
			opus_decoder_destroy(other);
			other = nullptr;
		}
	}

	OpusDecoderWrapper::OpusDecoderWrapper() {
		int32_t error{};
		this->data.resize(23040);
		this->ptr.reset(opus_decoder_create(48000, 2, &error));
		if (error != OPUS_OK) {
			throw DiscordCoreAPI::DCAException{ "Failed to create the Opus decoder, Reason: " + std::string{ opus_strerror(error) } };
		}
	}

	std::basic_string_view<opus_int16> OpusDecoderWrapper::decodeData(std::basic_string_view<std::byte> dataToDecode) {
		const int64_t sampleCount = opus_decode(this->ptr.get(), reinterpret_cast<const uint8_t*>(dataToDecode.data()),
			static_cast<opus_int32>(dataToDecode.length() & 0x7FFFFFFF), data.data(), 5760, 0);
		if (sampleCount > 0) {
			return std::basic_string_view<opus_int16>{ this->data.data(), static_cast<size_t>(sampleCount * 2ull) };
		} else {
			throw DiscordCoreAPI::DCAException{ "Failed to decode a user's voice payload, Reason: " + std::string{ opus_strerror(sampleCount) } };
		}
	}

	void AVFrameWrapper::AVFrameDeleter::operator()(AVFrame* other) {
		if (other) {
			av_frame_unref(other);
			av_frame_free(&other);
		}
	}

	AVFrameWrapper& AVFrameWrapper::operator=(AVFrame* other) {
		this->ptr.reset(nullptr);
		this->ptr.reset(other);
		return *this;
	}

	AVFrameWrapper::AVFrameWrapper(AVFrame* other) {
		*this = other;
	}

	AVFrame* AVFrameWrapper::operator->() {
		return this->ptr.get();
	}

	AVFrameWrapper::operator AVFrame*() {
		return this->ptr.get();
	}

	void AVCodecContextWrapper::AVCodecContextDeleter::operator()(AVCodecContext* other) {
		if (other) {
			avcodec_free_context(&other);
		}
	}

	AVCodecContextWrapper& AVCodecContextWrapper::operator=(AVCodecContext* other) {
		this->ptr.reset(nullptr);
		this->ptr.reset(other);
		return *this;
	}

	AVCodecContextWrapper::AVCodecContextWrapper(AVCodecContext* other) {
		*this = other;
	}

	AVCodecContext* AVCodecContextWrapper::operator->() {
		return this->ptr.get();
	}

	AVCodecContextWrapper::operator AVCodecContext*() {
		return this->ptr.get();
	}

	void AVFormatContextWrapper::AVFormatContextDeleter::operator()(AVFormatContextWrapper01* other) {
		if (other->didItInitialize) {
			avformat_free_context(other->theContext);
		}
	}

	AVFormatContextWrapper& AVFormatContextWrapper::operator=(AVFormatContext* other) {
		this->ptr->theContext = other;
		return *this;
	}

	AVFormatContextWrapper::AVFormatContextWrapper(AVFormatContext* other) {
		*this = other;
	}

	bool* AVFormatContextWrapper::getBoolPtr() {
		return &this->ptr.get()->didItInitialize;
	}

	AVFormatContext* AVFormatContextWrapper::operator->() {
		return this->ptr.get()->theContext;
	}

	AVFormatContext** AVFormatContextWrapper::operator*() {
		return &this->ptr.get()->theContext;
	}

	AVFormatContextWrapper::operator AVFormatContext*() {
		return this->ptr.get()->theContext;
	}

	void SwrContextWrapper::SwrContextDeleter::operator()(SwrContext* other) {
		if (other) {
			swr_free(&other);
		}
	}

	SwrContextWrapper& SwrContextWrapper::operator=(SwrContext* other) {
		this->ptr.reset(nullptr);
		this->ptr.reset(other);
		return *this;
	}

	SwrContextWrapper::SwrContextWrapper(SwrContext* other) {
		*this = other;
	}

	SwrContextWrapper::operator SwrContext*() {
		return this->ptr.get();
	}

	void AVIOContextWrapper::AVIOContextDeleter::operator()(AVIOContext* other) {
		if (other) {
			if (other->buffer) {
				av_freep(&other->buffer);
			}
			avio_context_free(&other);
		}
	}

	AVIOContextWrapper& AVIOContextWrapper::operator=(AVIOContext* other) {
		this->ptr.reset(nullptr);
		this->ptr.reset(other);
		return *this;
	}

	AVIOContextWrapper::AVIOContextWrapper(AVIOContext* other) {
		*this = other;
	}

	AVIOContext* AVIOContextWrapper::operator->() {
		return this->ptr.get();
	}

	AVIOContextWrapper::operator AVIOContext*() {
		return this->ptr.get();
	}

	void AVPacketWrapper::AVPacketDeleter::operator()(AVPacket* other) {
		if (other) {
			av_packet_free(&other);
		}
	}

	AVPacketWrapper& AVPacketWrapper::operator=(AVPacket* other) {
		this->ptr.reset(nullptr);
		this->ptr.reset(other);
		return *this;
	}

	AVPacketWrapper::AVPacketWrapper(AVPacket* other) {
		*this = other;
	}

	AVPacket* AVPacketWrapper::operator->() {
		return this->ptr.get();
	}

	AVPacketWrapper::operator AVPacket*() {
		return this->ptr.get();
	}

	AudioDecoder::AudioDecoder(const BuildAudioDecoderData& dataPackage) {
		this->configManager = dataPackage.configManager;
		this->bufferMaxSize = dataPackage.bufferMaxSize;
		this->totalFileSize = dataPackage.totalFileSize;
	}

	bool AudioDecoder::getFrame(DiscordCoreAPI::AudioFrameData& dataPackage) {
		if (!this->areWeQuitting.load()) {
			if (this->outDataBuffer.tryReceive(dataPackage)) {
				if (dataPackage.currentSize != -5) {
					return true;
				}
			}
		}
		return false;
	}

	void AudioDecoder::submitDataForDecoding(std::string dataToDecode) {
		this->inputDataBuffer.send(std::move(dataToDecode));
	}

	bool AudioDecoder::haveWeFailed() {
		return this->haveWeFailedBool.load();
	}

	void AudioDecoder::startMe() {
		this->taskThread = std::make_unique<std::jthread>([=, this](std::stop_token token) {
			this->run(token);
		});
	};

	int32_t AudioDecoder::ReadBufferData(void* opaque, uint8_t* buf, int32_t) {
		AudioDecoder* stream = static_cast<AudioDecoder*>(opaque);
		stream->bytesRead = 0;
		stream->currentBuffer = std::string{};
		DiscordCoreAPI::AudioFrameData frameData{};
		if (stream->areWeQuitting.load()) {
			frameData.currentSize = -5;
			stream->outDataBuffer.send(std::move(frameData));
			return AVERROR_EOF;
		}
		if (DiscordCoreAPI::waitForTimeToPass(stream->inputDataBuffer, stream->currentBuffer, stream->refreshTimeForBuffer.load())) {
			frameData.currentSize = -5;
			stream->outDataBuffer.send(std::move(frameData));
			stream->areWeQuitting.store(true);
			return AVERROR_EOF;
		}
		if (stream->currentBuffer.size() > 0) {
			stream->bytesRead = stream->currentBuffer.size();
		}
		for (int32_t x = 0; x < stream->bytesRead; ++x) {
			buf[x] = stream->currentBuffer[x];
		}
		if (stream->ioContext->buf_ptr - stream->ioContext->buffer >= stream->totalFileSize) {
			frameData.currentSize = stream->bytesRead;
			stream->outDataBuffer.send(std::move(frameData));
			stream->areWeQuitting.store(true);
			if (stream->bytesRead == 0) {
				return AVERROR_EOF;
			}
			return static_cast<int32_t>(stream->bytesRead);
		}
		if (stream->bytesRead == 0) {
			return AVERROR_EOF;
		}
		return static_cast<int32_t>(stream->bytesRead);
	}

	void AudioDecoder::run(std::stop_token token) {
		if (!this->haveWeBooted) {
			auto buffer = static_cast<uint8_t*>(av_malloc(this->bufferMaxSize));
			if (!buffer) {
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Failed to allocate filestreambuffer."
						 << DiscordCoreAPI::reset() << endl
						 << endl;
				}
				return;
			}

			this->ioContext = avio_alloc_context(buffer, static_cast<int32_t>(this->bufferMaxSize - 1), 0, this, &AudioDecoder::ReadBufferData, 0, 0);

			if (!this->ioContext) {
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Failed to allocate AVIOContext."
						 << DiscordCoreAPI::reset() << endl
						 << endl;
				}
				return;
			}

			this->formatContext = avformat_alloc_context();

			if (!this->formatContext) {
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Could not allocate the format context."
						 << DiscordCoreAPI::reset() << endl
						 << endl;
				}
				return;
			}

			this->formatContext->pb = this->ioContext;
			this->formatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
			if (avformat_open_input(*this->formatContext, "memory", nullptr, nullptr) < 0) {
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Failed to open the AVFormatContext."
						 << DiscordCoreAPI::reset() << endl
						 << endl;
				}
				return;
			}
			*this->formatContext.getBoolPtr() = true;
			AVMediaType type = AVMediaType::AVMEDIA_TYPE_AUDIO;
			int32_t ret = av_find_best_stream(this->formatContext, type, -1, -1, NULL, 0);
			if (ret < 0) {
				std::string newString = "AudioDecoder::run() Error: Could not find ";
				newString += av_get_media_type_string(type);
				newString += " stream in input memory stream.";
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << newString << DiscordCoreAPI::reset() << endl << endl;
				}
				return;
			} else {
				this->audioStreamIndex = ret;
				this->audioStream = this->formatContext->streams[this->audioStreamIndex];
				if (!this->audioStream) {
					this->haveWeFailedBool.store(true);
					if (this->configManager->doWePrintFFMPEGErrorMessages()) {
						cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Could not find an audio stream."
							 << DiscordCoreAPI::reset() << endl
							 << endl;
					}
					return;
				}

				if (avformat_find_stream_info(this->formatContext, NULL) < 0) {
					this->haveWeFailedBool.store(true);
					if (this->configManager->doWePrintFFMPEGErrorMessages()) {
						cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Could not find stream information."
							 << DiscordCoreAPI::reset() << endl
							 << endl;
					}
					return;
				}

				this->codec = const_cast<AVCodec*>(avcodec_find_decoder(this->audioStream->codecpar->codec_id));
				if (!this->codec) {
					std::string newString = "AudioDecoder::run() Error: Failed to find ";
					newString += av_get_media_type_string(type);
					newString += " decoder.";
					this->haveWeFailedBool.store(true);
					if (this->configManager->doWePrintFFMPEGErrorMessages()) {
						cout << DiscordCoreAPI::shiftToBrightRed() << newString << DiscordCoreAPI::reset() << endl << endl;
					}
					return;
				}

				this->audioDecodeContext = avcodec_alloc_context3(this->codec);
				if (!this->audioDecodeContext) {
					std::string newString = "AudioDecoder::run() Error: Failed to allocate the ";
					newString += av_get_media_type_string(type);
					newString += " AVCodecContext.";
					this->haveWeFailedBool.store(true);
					if (this->configManager->doWePrintFFMPEGErrorMessages()) {
						cout << DiscordCoreAPI::shiftToBrightRed() << newString << DiscordCoreAPI::reset() << endl << endl;
					}
					return;
				}

				if (avcodec_parameters_to_context(this->audioDecodeContext, this->audioStream->codecpar) < 0) {
					std::string newString = "AudioDecoder::run() Error: Failed to copy ";
					newString += av_get_media_type_string(type);
					newString += " codec parameters to decoder context.";
					this->haveWeFailedBool.store(true);
					if (this->configManager->doWePrintFFMPEGErrorMessages()) {
						cout << DiscordCoreAPI::shiftToBrightRed() << newString << DiscordCoreAPI::reset() << endl << endl;
					}
					return;
				}

				if (avcodec_open2(this->audioDecodeContext, this->codec, NULL) < 0) {
					std::string newString = "AudioDecoder::run() Error: Failed to open ";
					newString += av_get_media_type_string(type);
					newString += " AVCodecContext.";
					this->haveWeFailedBool.store(true);
					if (this->configManager->doWePrintFFMPEGErrorMessages()) {
						cout << DiscordCoreAPI::shiftToBrightRed() << newString << DiscordCoreAPI::reset() << endl << endl;
					}
					return;
				}

				this->swrContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AVSampleFormat::AV_SAMPLE_FMT_S16, 48000, AV_CH_LAYOUT_STEREO,
					this->audioDecodeContext->sample_fmt, this->audioDecodeContext->sample_rate, 0, nullptr);
				swr_init(this->swrContext);
				if (this->configManager->doWePrintFFMPEGSuccessMessages()) {
					av_dump_format(this->formatContext, 0, "memory", 0);
				}
			}

			this->haveWeBooted = true;
		}

		if (this->currentBuffer.size() > 0) {
			this->packet = av_packet_alloc();
			if (!this->packet) {
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Could not allocate packet" << DiscordCoreAPI::reset()
						 << endl
						 << endl;
				}
				return;
			}

			this->frame = av_frame_alloc();
			if (!this->frame) {
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Could not allocate frame" << DiscordCoreAPI::reset()
						 << endl
						 << endl;
				}
				return;
			}

			this->newFrame = av_frame_alloc();
			if (!this->newFrame) {
				this->haveWeFailedBool.store(true);
				if (this->configManager->doWePrintFFMPEGErrorMessages()) {
					cout << DiscordCoreAPI::shiftToBrightRed() << "AudioDecoder::run() Error: Could not allocate new-frame" << DiscordCoreAPI::reset()
						 << endl
						 << endl;
				}
				return;
			}

			while (!token.stop_requested() && !this->areWeQuitting.load() && av_read_frame(this->formatContext, this->packet) == 0) {
				if (this->packet->stream_index == this->audioStreamIndex) {
					int32_t returnValue = avcodec_send_packet(this->audioDecodeContext, this->packet);
					if (returnValue < 0) {
						char charString[32];
						av_strerror(returnValue, charString, 32);
						std::string newString = "AudioDecoder::run() Error: Issue submitting a packet for decoding (" + std::to_string(returnValue) +
							"), " + charString + ".";
						this->haveWeFailedBool.store(true);
						if (this->configManager->doWePrintFFMPEGErrorMessages()) {
							cout << DiscordCoreAPI::shiftToBrightRed() << newString << DiscordCoreAPI::reset() << endl << endl;
						}
						return;
					} else {
						returnValue = avcodec_receive_frame(this->audioDecodeContext, this->frame);
						if (returnValue < 0) {
							std::string newString = "AudioDecoder::run() Error: Issue during decoding (" + std::to_string(returnValue) + ")";
							this->haveWeFailedBool.store(true);
							if (this->configManager->doWePrintFFMPEGErrorMessages()) {
								cout << DiscordCoreAPI::shiftToBrightRed() << newString << DiscordCoreAPI::reset() << endl << endl;
							}
							return;
						}

						if (!swr_is_initialized(this->swrContext)) {
							swr_init(this->swrContext);
						}
						this->newFrame->channel_layout = AV_CH_LAYOUT_STEREO;
						this->newFrame->sample_rate = 48000;
						this->newFrame->format = AVSampleFormat::AV_SAMPLE_FMT_S16;
						this->newFrame->nb_samples = frame->nb_samples;
						this->newFrame->pts = frame->pts;
						swr_convert_frame(this->swrContext, this->newFrame, this->frame);
						int32_t unpadded_linesize =
							this->newFrame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(this->newFrame->format)) * 2;
						DiscordCoreAPI::AudioFrameData rawFrame{};
						rawFrame.type = DiscordCoreAPI ::AudioFrameType::RawPCM;
						rawFrame.data.resize(unpadded_linesize);
						for (int32_t x = 0; x < unpadded_linesize; ++x) {
							rawFrame.data[x] = static_cast<std::byte>(this->newFrame->extended_data[0][x]);
						}
						rawFrame.currentSize = static_cast<int64_t>(newFrame->nb_samples) * 4ll;
						this->outDataBuffer.send(std::move(rawFrame));
						int64_t sampleCount = swr_get_delay(this->swrContext, this->newFrame->sample_rate);
						if (sampleCount > 0) {
							if (!swr_is_initialized(this->swrContext)) {
								swr_init(this->swrContext);
							}
							swr_convert_frame(this->swrContext, this->newFrame, nullptr);
							DiscordCoreAPI::AudioFrameData rawFrame02{};
							rawFrame02.type = DiscordCoreAPI ::AudioFrameType::RawPCM;
							rawFrame02.data.resize(*this->newFrame->linesize);
							for (int32_t x = 0; x < *this->newFrame->linesize; ++x) {
								rawFrame02.data[x] = static_cast<std::byte>(this->newFrame->extended_data[0][x]);
							}
							rawFrame02.currentSize = static_cast<int64_t>(newFrame->nb_samples) * 4ll;
							this->outDataBuffer.send(std::move(rawFrame02));
						}
					}
				} else {
					break;
				}
				this->frame = av_frame_alloc();
				this->newFrame = av_frame_alloc();
				this->packet = av_packet_alloc();
				if (token.stop_requested() || this->areWeQuitting.load()) {
					break;
				}
			}
			if (this->configManager->doWePrintFFMPEGSuccessMessages()) {
				cout << DiscordCoreAPI::shiftToBrightGreen() << "Completed decoding!" << endl << DiscordCoreAPI::reset() << endl << endl;
			}
		}
		return;
	}

	void AudioDecoder::cancelMe() {
		this->refreshTimeForBuffer.store(10);
		this->inputDataBuffer.clearContents();
		this->inputDataBuffer.send(std::string{});
		this->inputDataBuffer.send(std::string{});
		this->inputDataBuffer.send(std::string{});
		this->inputDataBuffer.send(std::string{});
		this->inputDataBuffer.send(std::string{});
		this->areWeQuitting.store(true);
	}

	AudioDecoder::~AudioDecoder() {
		this->cancelMe();
	}

};
