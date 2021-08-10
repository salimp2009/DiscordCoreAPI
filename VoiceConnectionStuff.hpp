// VoiceConnectionStuff.hpp - Header for all of the "voice connection" stuff.
// Jul 15, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#ifndef _VOICE_CONNECTION_STUFF_
#define _VOICE_CONNECTION_STUFF_

#include "../pch.h"
#include "WebSocketStuff.hpp"
#include "DemuxingStuff.hpp"
#include "DecodingStuff.hpp"
#include "C:/Users/Chris/source/repos/MBot-MusicHouse-Cpp/AudioOutputStuff.hpp"

namespace DiscordCoreAPI {

	struct AudioDataChunk {
		IBuffer audioData;
		string playerId;
		unsigned __int64 audioBitrate;
		__int64 totalByteSize = 1;
		__int64 remainingBytes;
		string filePath;
		string fileName;
	};

	class ThreadPoolTimerNew {
	public:
		ThreadPoolTimerNew() {};
		ThreadPoolTimer threadPoolTimer{ nullptr };
	};

	class VoiceConnection : public agent {
	public:
		bool doWeQuit = false;
		bool doWeWait = true;
		__int32 timestamp;
		__int16 sequenceIndex;
		DiscordCoreInternal::VoiceConnectionData voiceConnectionData;
		shared_ptr<DiscordCoreInternal::VoiceChannelWebSocketAgent> voicechannelWebSocketAgent{ nullptr };
		shared_ptr<unbounded_buffer<AudioDataChunk>> bufferMessageBlock;
		shared_ptr<unbounded_buffer<bool>> readyBuffer;
		unbounded_buffer<bool> playPauseBuffer;
		VoiceConnection(DiscordCoreInternal::VoiceConnectionData voiceConnectionDataNew, shared_ptr<unbounded_buffer<AudioDataChunk>> bufferMessageBlockNew)
			: agent(*DiscordCoreInternal::ThreadManager::getThreadContext().get()->scheduler) {
			if (voiceConnectionDataNew.channelId != "") {
				this->voiceConnectionData = voiceConnectionDataNew;
				this->bufferMessageBlock = bufferMessageBlockNew;
				this->readyBuffer = make_shared<unbounded_buffer<bool>>();
				this->voicechannelWebSocketAgent = make_shared<DiscordCoreInternal::VoiceChannelWebSocketAgent>(DiscordCoreInternal::ThreadManager::getThreadContext().get(), this->voiceConnectionData, this->readyBuffer);
				send(this->voicechannelWebSocketAgent->voiceConnectionDataBuffer, this->voiceConnectionData);
				this->voicechannelWebSocketAgent->start();
				receive(*this->readyBuffer);
				this->voiceConnectionData = this->voicechannelWebSocketAgent->voiceConnectionData;
			}
			else {
				throw exception("Sorry, but you need to select a proper voice channel to connect to!");
			}
		}

		void deleteFile(string filePath, string fileName) {
			auto folder = winrt::Windows::Storage::KnownFolders::GetFolderAsync(winrt::Windows::Storage::KnownFolderId::MusicLibrary).get();
			auto folder2 = folder.GetFolderFromPathAsync(to_hstring(filePath)).get();
			winrt::Windows::Storage::StorageFile storageFile = folder2.GetFileAsync(to_hstring(fileName)).get();
			storageFile.DeleteAsync().get();
		}

		void play(bool doWeBlock = false) {
			send(this->playPauseBuffer, true);
			this->doWeWait = false;
			this->start();
			if (doWeBlock) {
				wait(this);
			}
		}

		void startPlaying() {
			send(this->playPauseBuffer, true);
			this->doWeWait = false;
		}

		void pausePlaying() {
			this->doWeWait = true;
		}

		void clearBuffer() {

		}

		~VoiceConnection() {
			this->terminate();
		}

	protected:
		friend class Guild;

		void sendSingleAudioQuantum(IBuffer bufferToSend) {
			if (sodium_init() == -1) {
				cout << "LibSodium failed to initialize!" << endl << endl;
				this->terminate();
			}
			constexpr int headerSize = 12;
			constexpr int nonceSize = 4;
			const unsigned int frameSize = bufferToSend.Length() / 2;
			this->timestamp += 10;

			const uint8_t header[headerSize] = {
				0x80,
				0x78,
				(uint8_t)((this->sequenceIndex >> (8 * 1)) & 0xff),
				(uint8_t)((this->sequenceIndex >> (8 * 0)) & 0xff),
				(uint8_t)((this->timestamp >> (8 * 3)) & 0xff),
				(uint8_t)((this->timestamp >> (8 * 2)) & 0xff),
				(uint8_t)((this->timestamp >> (8 * 1)) & 0xff),
				(uint8_t)((this->timestamp >> (8 * 0)) & 0xff),
				(uint8_t)((this->voiceConnectionData.audioSSRC >> (8 * 3)) & 0xff),
				(uint8_t)((this->voiceConnectionData.audioSSRC >> (8 * 2)) & 0xff),
				(uint8_t)((this->voiceConnectionData.audioSSRC >> (8 * 1)) & 0xff),
				(uint8_t)((this->voiceConnectionData.audioSSRC >> (8 * 0)) & 0xff),
			};

			uint8_t nonce[nonceSize] =
			{ (uint8_t)((this->sequenceIndex >> (8 * 1)) & 0xff),
				(uint8_t)((this->sequenceIndex >> (8 * 0)) & 0xff) };

			const size_t numOfBytes = sizeof(header) + nonceSize + bufferToSend.Length() + crypto_secretbox_MACBYTES;
			uint8_t* audioDataPacket;
			audioDataPacket = new uint8_t[numOfBytes];
			std::memcpy(audioDataPacket, header, sizeof(header));

			if (crypto_secretbox_easy(audioDataPacket + sizeof(header),
				bufferToSend.data(), bufferToSend.Length(), nonce, (unsigned char*)this->voiceConnectionData.keys.data()) != 0) {
				throw exception("ENCRYPTION FAILED!");
			};

			memcpy_s(audioDataPacket + (numOfBytes - nonceSize), nonceSize, nonce, nonceSize);
			vector<uint8_t> audioDataPacketNew;
			audioDataPacketNew.resize(numOfBytes);
			for (unsigned int x = 0; x < numOfBytes; x += 1) {
				audioDataPacketNew[x] = audioDataPacket[x];
			}

			this->voicechannelWebSocketAgent->sendVoiceData(audioDataPacketNew);
			this->sequenceIndex += 1;
		}

		void sendSpeakingMessage(bool isSpeaking) {
			this->voiceConnectionData.audioSSRC = this->voicechannelWebSocketAgent->voiceConnectionData.audioSSRC;
			string newString = DiscordCoreInternal::getIsSpeakingPayload(isSpeaking, this->voiceConnectionData.audioSSRC, 0);
			this->voicechannelWebSocketAgent->sendMessage(newString);
		}

		void cleanupAndSwitchPlayerId(int* counter, __int64* totalByteSize, __int64* bytesRemaining, __int64* startingTime, vector<ThreadPoolTimerNew>* sendTimers, string* playerId, AudioDataChunk audioData) {
			*counter = 0;
			*totalByteSize = audioData.totalByteSize;
			*bytesRemaining = audioData.totalByteSize;
			*playerId = audioData.playerId;
			*startingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			for (auto value : *sendTimers) {
				value.threadPoolTimer.Cancel();
			}
		}

		void saveFile(hstring filePath, hstring fileName, IBuffer readBuffer) {
			auto folder = winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(filePath).get();
			winrt::Windows::Storage::StorageFile storageFile = folder.CreateFileAsync(fileName, CreationCollisionOption::ReplaceExisting).get();
			winrt::Windows::Storage::FileIO::WriteBufferAsync(storageFile, readBuffer).get();
		}

		IBuffer loadFile(hstring filePath, hstring fileName) {
			auto folder = winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(filePath).get();
			winrt::Windows::Storage::StorageFile storageFile = folder.GetFileAsync(fileName).get();
			auto returnBuffer = winrt::Windows::Storage::FileIO::ReadBufferAsync(storageFile).get();
			return returnBuffer;
		}

		void run() {
			AudioDataChunk audioData;
			vector<ThreadPoolTimerNew> sendTimers;
			string playerId;
			int counter = 0;
			__int64 totalByteSize = 0;
			__int64 bytesRemaining = -1;
			int bufferIndex = 0;
			vector<IBuffer> bufferVector(2);
			__int64 startingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			while (doWeQuit == false && bytesRemaining > 0) {
				if (this->doWeWait) {
					receive(this->playPauseBuffer);
				}

				try {
					if (bytesRemaining == -1) {
						sendSpeakingMessage(true);
					}
					if (bytesRemaining == 0) {
						sendSpeakingMessage(false);
					}
					audioData = receive(bufferMessageBlock.get(), 20000);
					if (counter == 0) {
						bytesRemaining = audioData.totalByteSize;
					}
					totalByteSize = audioData.totalByteSize;
					cout << "TOTAL BYTES: " << totalByteSize << endl;
					bytesRemaining -= audioData.audioData.Length();
					cout << "AUDIO DATA LENGTH: " << audioData.audioData.Length() << endl;
					if (audioData.playerId != playerId) {
						cleanupAndSwitchPlayerId(&counter, &totalByteSize, &bytesRemaining, &startingTime, &sendTimers, &playerId, audioData);
						this->timestamp = (int)startingTime;
					}
					if (counter == 0 && bufferIndex < 2) {
						bufferVector[bufferIndex] = audioData.audioData;
						bufferIndex += 1;
					}
					if (audioData.audioData.Length() > 0) {
						vector<uint8_t> dataVector;
						for (unsigned int x = 0; x < audioData.audioData.Length(); x += 1) {
							dataVector.push_back(audioData.audioData.data()[x]);
						}
						WebAFile buffer = demuxWebA(dataVector);
						InMemoryRandomAccessStream randomStream;
						DataWriter writer(randomStream.GetOutputStreamAt(0));
						
						
						DataReader reader(randomStream.GetInputStreamAt(0));
						
						auto outputBufferNew = decodeOpusData(buffer, audioData.filePath, audioData.fileName);
						hstring newFilePath = to_hstring(audioData.filePath);
						hstring newFileName = to_hstring(audioData.fileName) + L" DECODED.opus";
						writer.WriteBytes(outputBufferNew);
						writer.StoreAsync().get();
						reader.LoadAsync((uint32_t)outputBufferNew.size());
						saveFile(newFilePath, newFileName, reader.ReadBuffer((uint32_t)outputBufferNew.size()));
					}
					counter += 1;
				}
				catch (exception& e) {
					cout << "Out of time: " << e.what() << endl;
					break;
				}
			}
			cout << "END OF VOICECONNECTION::PLAY: " << endl;
			done();
		}

		void terminate() {
			done();
			this->doWeQuit = true;
		}

	};

}
#endif