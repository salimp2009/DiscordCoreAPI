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
/// VoiceConnection.hpp - Header for the voice connection class.
/// Jul 15, 2021
/// https://discordcoreapi.com
/// \file VoiceConnection.hpp

#pragma once

#ifndef VOICE_CONNECTION
	#define VOICE_CONNECTION

	#include <discordcoreapi/FoundationEntities.hpp>
	#include <discordcoreapi/AudioEncoder.hpp>
	#include <discordcoreapi/CoRoutine.hpp>
	#include <discordcoreapi/WebSocketEntities.hpp>
	#include <sodium.h>

namespace DiscordCoreAPI {

	struct DiscordCoreAPI_Dll VoiceSocketReadyData {
		VoiceSocketReadyData(simdjson::ondemand::value);
		std::string mode{};
		std::string ip{};
		uint64_t port{};
		uint32_t ssrc{};
	};

	struct DiscordCoreAPI_Dll OpusDecoderWrapper {
		struct DiscordCoreAPI_Dll OpusDecoderDeleter {
			void operator()(OpusDecoder*) noexcept;
		};

		OpusDecoderWrapper();

		std::basic_string_view<opus_int16> decodeData(const std::basic_string_view<uint8_t> dataToDecode);

	  protected:
		std::unique_ptr<OpusDecoder, OpusDecoderDeleter> ptr{ nullptr, OpusDecoderDeleter{} };
		std::vector<opus_int16> data{};
	};

	struct MovingAverager {
		MovingAverager(size_t periodCountNew) noexcept;

		void addValue(int64_t value);

		int64_t collectAverage();

	  protected:
		std::deque<int64_t> values{};
		const size_t periodCount{};
		std::mutex accessMutex{};
	};

	struct DiscordCoreAPI_Dll VoiceUser {
		VoiceUser() noexcept = default;

		VoiceUser(MovingAverager* sleepableTimeNew, std::atomic_int8_t* voiceUserCount) noexcept;

		VoiceUser& operator=(VoiceUser&&) noexcept;

		VoiceUser(VoiceUser&&) noexcept;

		VoiceUser& operator=(const VoiceUser&) noexcept = delete;

		VoiceUser(const VoiceUser&) noexcept = delete;

		void insertPayload(std::basic_string<uint8_t>&&);

		std::basic_string<uint8_t> extractPayload();

		OpusDecoderWrapper& getDecoder();

		void setEndingStatus(bool);

		void setUserId(Snowflake);

		bool getEndingStatus();

		Snowflake getUserId();

	  protected:
		UnboundedMessageBlock<std::basic_string<uint8_t>> payloads{};
		std::atomic_bool wereWeEnding{ false };
		std::atomic_int8_t* voiceUserCount{};
		MovingAverager* sleepableTime{};
		OpusDecoderWrapper decoder{};
		Snowflake userId{};
	};

	using TimeDuration = std::chrono::duration<long long, Nanoseconds>;
	using TimePoint = std::chrono::time_point<HRClock, Nanoseconds>;

	struct DiscordCoreAPI_Dll RTPPacketEncrypter {
		RTPPacketEncrypter() noexcept = default;

		RTPPacketEncrypter(uint32_t ssrcNew, const std::vector<uint8_t>& keysNew) noexcept;

		std::basic_string_view<uint8_t> encryptPacket(const AudioFrameData& audioData) noexcept;

	  protected:
		std::vector<uint8_t> keys{};
		std::vector<uint8_t> data{};
		uint8_t version{ 0x80 };
		uint8_t flags{ 0x78 };
		uint32_t timeStamp{};
		uint16_t sequence{};
		uint32_t ssrc{};
	};

	/// For the various connection states of the VoiceConnection class. \brief For the various connection states of the VoiceConnection class.
	enum class VoiceConnectionState : int8_t {
		Collecting_Init_Data = 0,///< Collecting initialization data.
		Initializing_WebSocket = 1,///< Initializing the WebSocket.
		Collecting_Hello = 2,///< Collecting the client hello.
		Sending_Identify = 3,///< Sending the identify payload.
		Collecting_Ready = 4,///< Collecting the client ready.
		Initializing_DatagramSocket = 5,///< Initializing the datagram udp socket.
		Sending_Select_Protocol = 6,///< Sending the select-protocol payload.
		Collecting_Session_Description = 7///< Collecting the session-description payload.
	};

	/// For the various active states of the VoiceConnection class. \brief For the various active states of the VoiceConnection class.
	enum class VoiceActiveState : int8_t {
		Connecting = -1,///< Connecting - it hasn't started or it's reconnecting.
		Playing = 1,///< Playing.
		Stopped = 2,///< Stopped.
		Paused = 3,///< Paused.
		Exiting = 4///< Exiting.
	};

	class DiscordCoreAPI_Dll VoiceConnectionBridge : public DiscordCoreInternal::DatagramSocketClient {
	  public:
		VoiceConnectionBridge(DiscordCoreClient* voiceConnectionPtrNew, StreamType streamType, Snowflake guildIdNew);

		void parseOutGoingVoiceData() noexcept;

		void handleAudioBuffer() noexcept;

	  protected:
		DiscordCoreClient* clientPtr{ nullptr };
		Snowflake guildId{};
	};

	/**
	 * \addtogroup voice_connection
	 * @{
	 */
	/// VoiceConnection class - represents the connection to a given voice Channel. \brief VoiceConnection class - represents the connection to a given voice Channel.
	class DiscordCoreAPI_Dll VoiceConnection : public DiscordCoreInternal::WebSocketCore, public DiscordCoreInternal::DatagramSocketClient {
	  public:
		friend class DiscordCoreInternal::BaseSocketAgent;
		friend class DiscordCoreInternal::SoundCloudAPI;
		friend class DiscordCoreInternal::YouTubeAPI;
		friend class VoiceConnectionBridge;
		friend class DiscordCoreClient;
		friend class GuildData;
		friend class SongAPI;

		/// The constructor.
		VoiceConnection(DiscordCoreClient* clientPtrNew, DiscordCoreInternal::WebSocketSSLShard* baseShardNew,
			std::atomic_bool* doWeQuitNew) noexcept;

		/// Collects the currently connected-to voice Channel's id. \brief Collects the currently connected-to voice Channel's id.
		/// \returns DiscordCoreAPI::Snowflake A Snowflake containing the Channel's id.
		Snowflake getChannelId() noexcept;

		~VoiceConnection() noexcept;

	  protected:
		std::atomic<VoiceConnectionState> connectionState{ VoiceConnectionState::Collecting_Init_Data };
		UnboundedMessageBlock<DiscordCoreInternal::VoiceConnectionData> voiceConnectionDataBuffer{};
		std::atomic<VoiceActiveState> activeState{ VoiceActiveState::Connecting };
		DiscordCoreInternal::VoiceConnectionData voiceConnectionData{};
		DiscordCoreInternal::WebSocketSSLShard* baseShard{ nullptr };
		std::unique_ptr<VoiceConnectionBridge> streamSocket{};
		std::unique_ptr<std::jthread> taskThread01{ nullptr };
		std::unique_ptr<std::jthread> taskThread02{ nullptr };
		std::unordered_map<uint64_t, VoiceUser> voiceUsers{};
		DiscordCoreClient* discordCoreClient{ nullptr };
		VoiceConnectInitData voiceConnectInitData{};
		std::vector<opus_int16> downSampledVector{};
		std::vector<uint8_t> decryptedDataString{};
		std::vector<opus_int32> upSampledVector{};
		std::atomic_bool areWePlaying{ false };
		std::atomic_bool* doWeQuit{ nullptr };
		RTPPacketEncrypter packetEncrypter{};
		std::vector<uint8_t> encryptionKey{};
		simdjson::ondemand::parser parser{};
		std::atomic_int8_t voiceUserCount{};
		MovingAverager sleepableTime{ 12 };
		std::string audioEncryptionMode{};
		Snowflake currentGuildMemberId{};
		std::mutex voiceUserMutex{};
		AudioFrameData audioData{};
		std::string externalIp{};
		AudioEncoder encoder{};
		std::string voiceIp{};
		std::string baseUrl{};
		uint32_t audioSSRC{};
		uint64_t port{};

		void parseIncomingVoiceData(const std::basic_string_view<uint8_t> rawDataBufferNew) noexcept;

		void sendVoiceData(const std::basic_string_view<uint8_t> responseData) noexcept;

		void connect(const DiscordCoreAPI::VoiceConnectInitData& initData) noexcept;

		UnboundedMessageBlock<AudioFrameData>& getAudioBuffer() noexcept;

		void checkForAndSendHeartBeat(const bool isItImmediage) noexcept;

		bool onMessageReceived(const std::string_view data) noexcept;

		void sendSingleFrame(AudioFrameData& frameData) noexcept;

		void sendSpeakingMessage(const bool isSpeaking) noexcept;

		void runBridge(std::stop_token) noexcept;

		void runVoice(std::stop_token) noexcept;

		bool areWeCurrentlyPlaying() noexcept;

		void handleAudioBuffer() noexcept;

		void connectInternal() noexcept;

		void clearAudioData() noexcept;

		bool areWeConnected() noexcept;

		bool voiceConnect() noexcept;

		void sendSilence() noexcept;

		void pauseToggle() noexcept;

		void checkForConnections();

		void disconnect() noexcept;

		void reconnect() noexcept;

		void onClosed() noexcept;

		void mixAudio() noexcept;

		bool stop() noexcept;

		bool play() noexcept;
	};
	/**@}*/

};// namespace DiscordCoreAPI
#endif