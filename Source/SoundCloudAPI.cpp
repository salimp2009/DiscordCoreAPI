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
/// SoundCloudAPI.cpp - Source file for the SoundCloud api related stuff.
/// Aug 25, 2021
/// https://discordcoreapi.com
/// \file SoundCloudAPI.cpp

#include <discordcoreapi/SoundCloudAPI.hpp>
#include <discordcoreapi/DiscordCoreClient.hpp>
#include <discordcoreapi/AudioEncoder.hpp>
#include <discordcoreapi/VoiceConnection.hpp>

namespace DiscordCoreInternal {

	std::vector<DiscordCoreAPI::Song> SoundCloudRequestBuilder::collectSearchResults(const std::string& songQuery) {
		try {
			HttpsWorkloadData dataPackage{ HttpsWorkloadType::SoundCloudGetSearchResults };
			dataPackage.baseUrl = this->baseUrl02;
			dataPackage.relativePath = "/search?q=" + DiscordCoreAPI::urlEncode(songQuery.c_str()) +
				"&facet=model&client_id=" + SoundCloudRequestBuilder::clientId +
				"&limit=20&offSet=0&linked_partitioning=1&app_version=" + this->appVersion + "&app_locale=en";
			dataPackage.workloadClass = HttpsWorkloadClass::Get;
			HttpsResponseData returnData = this->httpsClient->submitWorkloadAndGetResult(dataPackage);
			std::vector<DiscordCoreAPI::Song> results{};
			simdjson::ondemand::parser parser{};
			returnData.responseData.reserve(returnData.responseData.size() + simdjson::SIMDJSON_PADDING);
			auto document = parser.iterate(returnData.responseData.data(), returnData.responseData.length(), returnData.responseData.capacity());
			simdjson::ondemand::array arrayValue{};
			if (document.get_value().value().get_object().value()["collection"].get(arrayValue) == simdjson::error_code::SUCCESS) {
				for (auto value: arrayValue) {
					DiscordCoreAPI::Song newSong{ value.value() };
					if (newSong.songTitle == "") {
						continue;
					}
					newSong.type = DiscordCoreAPI::SongType::SoundCloud;
					newSong.firstDownloadUrl +=
						"?client_id=" + SoundCloudRequestBuilder::clientId + "&track_authorization=" + newSong.trackAuthorization;
					if (newSong.thumbnailUrl.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 ") != std::string::npos &&
						newSong.thumbnailUrl.find("https") != std::string::npos) {
						std::string newString = newSong.thumbnailUrl.substr(0, newSong.thumbnailUrl.find_last_of("-t") + 1);
						newString += "t500x500.jpg";
						newSong.thumbnailUrl = newString;
					}
					results.emplace_back(newSong);
				}
			}
			return results;
		} catch (...) {
			if (this->configManager->doWePrintHttpsErrorMessages()) {
				DiscordCoreAPI::reportException("SoundCloudRequestBuilder::collectSearchResults()");
			}
		}
		return {};
	}

	DiscordCoreAPI::Song SoundCloudRequestBuilder::constructDownloadInfo(DiscordCoreAPI::Song& newSong) {
		try {
			HttpsWorkloadData dataPackage01{ HttpsWorkloadType::SoundCloudGetSearchResults };
			dataPackage01.baseUrl = newSong.firstDownloadUrl;
			dataPackage01.workloadClass = HttpsWorkloadClass::Get;
			HttpsResponseData results = this->httpsClient->submitWorkloadAndGetResult(dataPackage01);
			simdjson::ondemand::parser parser{};
			results.responseData.reserve(results.responseData.size() + simdjson::SIMDJSON_PADDING);
			simdjson::ondemand::document document{};
			if (parser.iterate(results.responseData.data(), results.responseData.size(), results.responseData.capacity()).get(document) ==
				simdjson::error_code::SUCCESS) {
				simdjson::ondemand::value object{};
				if (document.get(object) == simdjson::error_code::SUCCESS) {
					std::string_view theUrl{};
					if (object["url"].get(theUrl) == simdjson::error_code::SUCCESS) {
						newSong.secondDownloadUrl = static_cast<std::string>(theUrl);
					}
				}
			}
			if (newSong.secondDownloadUrl.find("/playlist") != std::string::npos) {
				HttpsWorkloadData dataPackage{ HttpsWorkloadType::SoundCloudGetSearchResults };
				dataPackage.baseUrl = newSong.secondDownloadUrl;
				dataPackage.workloadClass = HttpsWorkloadClass::Get;
				HttpsResponseData results = this->httpsClient->submitWorkloadAndGetResult(dataPackage);
				std::string newString{ results.responseData };
				newSong.finalDownloadUrls.clear();
				while (newString.find("#EXTINF:") != std::string::npos) {
					std::string newString01 = "#EXTINF:";
					std::string newString02 = newString.substr(newString.find("#EXTINF:") + newString01.size());
					auto commandFind = newString02.find(",");
					std::string newString00 = newString02.substr(0, commandFind);
					std::string newString03 = newString02.substr(commandFind + 2, newString02.find("#EXTINF:") - (newString00.size() + 3));
					newString = newString02.substr(commandFind);
					if (newString03.find("#EXT-X-ENDLIST") != std::string::npos) {
						newString03 = newString03.substr(0, newString03.find("#EXT-X-ENDLIST"));
					}
					std::string newString04 = newString03.substr(newString03.find_first_of("1234567890"));
					int32_t firstNumber01 = stoi(newString04.substr(0, newString04.find("/")));
					std::string newString05 = newString04.substr(newString04.find("/") + 1);
					int32_t secondNumber = stoi(newString05.substr(0, newString05.find("/")));
					DiscordCoreAPI::DownloadUrl downloadUrl{};
					downloadUrl.urlPath = newString03;
					downloadUrl.contentSize = secondNumber - firstNumber01;
					newSong.finalDownloadUrls.emplace_back(downloadUrl);
				}
				for (auto& value: newSong.finalDownloadUrls) {
					newSong.contentLength += value.contentSize;
				}
			} else {
				HttpsWorkloadData dataPackage02{ HttpsWorkloadType::SoundCloudGetSearchResults };
				dataPackage02.baseUrl = newSong.secondDownloadUrl;
				dataPackage02.workloadClass = HttpsWorkloadClass::Get;
				auto headersNew = this->httpsClient->submitWorkloadAndGetResult(dataPackage02);
				int64_t valueBitRate{};
				int64_t valueLength{};
				if (headersNew.responseHeaders.find("x-amz-meta-bitrate") != headersNew.responseHeaders.end()) {
					valueBitRate = stoll(headersNew.responseHeaders.find("x-amz-meta-bitrate")->second);
				}
				if (headersNew.responseHeaders.find("x-amz-meta-duration") != headersNew.responseHeaders.end()) {
					valueLength = stoll(headersNew.responseHeaders.find("x-amz-meta-duration")->second);
				}
				DiscordCoreAPI::DownloadUrl downloadUrl{};
				downloadUrl.contentSize = static_cast<int32_t>(((valueBitRate * valueLength) / 8) - 193);
				downloadUrl.urlPath = newSong.secondDownloadUrl;
				newSong.finalDownloadUrls.emplace_back(downloadUrl);
			}
			return newSong;
		} catch (...) {
			if (this->configManager->doWePrintHttpsErrorMessages()) {
				DiscordCoreAPI::reportException("SoundCloudRequestBuilder::constructDownloadInfo()");
			}
		}
		return {};
	}

	DiscordCoreAPI::Song SoundCloudRequestBuilder::collectFinalSong(DiscordCoreAPI::Song& newSong) {
		newSong = constructDownloadInfo(newSong);
		return newSong;
	}

	std::string SoundCloudRequestBuilder::collectClientId() {
		HttpsWorkloadData dataPackage02{ HttpsWorkloadType::SoundCloudGetSearchResults };
		dataPackage02.baseUrl = this->baseUrl;
		dataPackage02.relativePath = "/search?q=testValue";
		dataPackage02.workloadClass = HttpsWorkloadClass::Get;
		HttpsResponseData returnData = this->httpsClient->submitWorkloadAndGetResult(dataPackage02);
		std::vector<std::string> assetPaths{};
		std::string newString01 = "crossorigin src=";
		std::string newerString{};
		newerString = returnData.responseData;
		std::string clientIdNew{};
		if (newerString.find(newString01) != std::string::npos) {
			std::string newString = newerString.substr(newerString.find(newString01) + newString01.size());
			std::string newString02 = newString.substr(1, newString.find(".js") + 2);
			assetPaths.emplace_back(newString02);
			while (newString.find("crossorigin src=") != std::string::npos) {
				std::string newString03 = newString.substr(1, newString.find(".js") + 2);
				newString = newString.substr(newString.find("crossorigin src=") + newString01.size());
				assetPaths.emplace_back(newString03);
			}
			HttpsWorkloadData dataPackage03{ HttpsWorkloadType::SoundCloudGetSearchResults };
			dataPackage03.baseUrl = assetPaths[5];
			dataPackage03.workloadClass = HttpsWorkloadClass::Get;
			HttpsResponseData returnData02 = this->httpsClient->submitWorkloadAndGetResult(dataPackage03);
			std::string newerString02{};
			newerString02.insert(newerString02.begin(), returnData02.responseData.begin(), returnData02.responseData.end());

			std::string newString03 =
				newerString02.substr(newerString02.find("JSON.stringify({client_id:\"") + std::string{ "JSON.stringify({client_id:\"" }.size());


			if (newString03.find("\",nonce:e.nonce}))))") != std::string::npos) {
				clientIdNew = newString03.substr(0, newString03.find("\",nonce:e.nonce}))))"));
			}
			if (returnData02.responseCode not_eq 200 && this->configManager->doWePrintHttpsErrorMessages()) {
				cout << DiscordCoreAPI::shiftToBrightRed() << "SoundCloudAPI::searchForSong Error: " << returnData.responseCode
					 << newerString02.c_str() << DiscordCoreAPI::reset() << endl
					 << endl;
			}
		}
		return clientIdNew;
	}

	SoundCloudAPI::SoundCloudAPI(DiscordCoreAPI::ConfigManager* configManagerNew, HttpsClient* httpsClient,
		const DiscordCoreAPI::Snowflake guildIdNew) {
		this->configManager = configManagerNew;
		this->httpsClient = httpsClient;
		this->guildId = static_cast<DiscordCoreAPI::Snowflake>(guildIdNew);
		if (SoundCloudRequestBuilder::clientId == "") {
			SoundCloudRequestBuilder::clientId = this->collectClientId();
		}
	}

	void SoundCloudAPI::weFailedToDownloadOrDecode(const DiscordCoreAPI::Song& newSong, std::stop_token token, int32_t currentReconnectTries) {
		++currentReconnectTries;
		DiscordCoreAPI::GuildMemberData guildMember =
			DiscordCoreAPI::GuildMembers::getCachedGuildMember({ .guildMemberId = newSong.addedByUserId, .guildId = this->guildId });
		DiscordCoreAPI::Song newerSong = newSong;
		if (currentReconnectTries > 9) {
			DiscordCoreAPI::AudioFrameData frameData{};
			while (DiscordCoreAPI::DiscordCoreClient::getSongAPI(this->guildId)->audioDataBuffer.tryReceive(frameData)) {
			};
			DiscordCoreAPI::SongCompletionEventData eventData{};
			auto returnValue = DiscordCoreAPI::DiscordCoreClient::getSongAPI(this->guildId);
			if (returnValue) {
				eventData.previousSong = returnValue->getCurrentSong(this->guildId);
			}
			eventData.wasItAFail = true;
			eventData.guildMember = guildMember;
			eventData.guild = DiscordCoreAPI::Guilds::getGuildAsync({ .guildId = this->guildId }).get();
			DiscordCoreAPI::DiscordCoreClient::getSongAPI(this->guildId)->onSongCompletionEvent(eventData);
		} else {
			newerSong = this->collectFinalSong(newerSong);
			SoundCloudAPI::downloadAndStreamAudio(newerSong, token, currentReconnectTries);
		}
	}

	void SoundCloudAPI::downloadAndStreamAudio(const DiscordCoreAPI::Song& newSong, std::stop_token token, int32_t currentReconnectTries) {
		try {
			int32_t counter{};
			BuildAudioDecoderData dataPackage{};
			dataPackage.totalFileSize = static_cast<uint64_t>(newSong.contentLength);
			dataPackage.bufferMaxSize = this->maxBufferSize;
			dataPackage.configManager = this->configManager;
			std::unique_ptr<AudioDecoder> audioDecoder = std::make_unique<AudioDecoder>(dataPackage);
			bool didWeGetZero{ true };
			std::vector<std::basic_string<std::byte>> submittedFrames{};
			while (counter < newSong.finalDownloadUrls.size()) {
				if (counter == newSong.finalDownloadUrls.size() && didWeGetZero) {
					audioDecoder.reset(nullptr);
					SoundCloudAPI::weFailedToDownloadOrDecode(newSong, token, currentReconnectTries);
					return;
				}
				std::this_thread::sleep_for(1ms);
				if (audioDecoder->haveWeFailed()) {
					audioDecoder.reset(nullptr);
					SoundCloudAPI::weFailedToDownloadOrDecode(newSong, token, currentReconnectTries);
					return;
				}
				if (token.stop_requested()) {
					audioDecoder.reset(nullptr);
					return;
				}
				HttpsWorkloadData dataPackage03{ HttpsWorkloadType::SoundCloudGetSearchResults };
				std::string baseUrl =
					newSong.finalDownloadUrls[counter].urlPath.substr(0, std::string{ "https://cf-hls-opus-media.sndcdn.com/media/" }.size());
				std::string relativeUrl =
					newSong.finalDownloadUrls[counter].urlPath.substr(std::string{ "https://cf-hls-opus-media.sndcdn.com/media/" }.size());
				dataPackage03.baseUrl = baseUrl;
				dataPackage03.relativePath = relativeUrl;
				dataPackage03.workloadClass = HttpsWorkloadClass::Get;
				auto result = this->httpsClient->submitWorkloadAndGetResult(dataPackage03);
				if (result.responseData.size() != 0) {
					didWeGetZero = false;
				}
				uint64_t amountToSubmitRemaining{ result.contentLength };
				uint64_t amountSubmitted{};
				while (amountToSubmitRemaining > 0 && result.responseData.size() > 0) {
					std::this_thread::sleep_for(1ms);
					std::string newerVector{};
					if (amountToSubmitRemaining >= this->maxBufferSize && result.responseData.size() >= this->maxBufferSize) {
						auto sizeToSubmit = this->maxBufferSize;
						newerVector.insert(newerVector.end(), result.responseData.begin(), result.responseData.begin() + sizeToSubmit);
						result.responseData.erase(result.responseData.begin(), result.responseData.begin() + sizeToSubmit);
						amountSubmitted += sizeToSubmit;
						amountToSubmitRemaining -= sizeToSubmit;
					} else if (result.responseData.size() < this->maxBufferSize || result.responseData.size() < amountToSubmitRemaining) {
						auto sizeToSubmit = result.responseData.size();
						newerVector.insert(newerVector.end(), result.responseData.begin(), result.responseData.begin() + sizeToSubmit);
						result.responseData.erase(result.responseData.begin(), result.responseData.begin() + sizeToSubmit);
						amountSubmitted += sizeToSubmit;
						amountToSubmitRemaining -= sizeToSubmit;
					} else {
						auto sizeToSubmit = amountToSubmitRemaining;
						newerVector.insert(newerVector.end(), result.responseData.begin(), result.responseData.begin() + sizeToSubmit);
						result.responseData.erase(result.responseData.begin(), result.responseData.begin() + sizeToSubmit);
						amountSubmitted += sizeToSubmit;
						amountToSubmitRemaining -= sizeToSubmit;
					}
					audioDecoder->submitDataForDecoding(newerVector);
				}
				if (counter == 0) {
					audioDecoder->startMe();
				}
				std::vector<DiscordCoreAPI::AudioFrameData> frames{};
				bool doWeBreak{};
				while (!doWeBreak) {
					DiscordCoreAPI::AudioFrameData rawFrame{};
					doWeBreak = !audioDecoder->getFrame(rawFrame);
					if (rawFrame.currentSize == -5) {
						doWeBreak = true;
						break;
					}
					if (rawFrame.data.size() != 0) {
						submittedFrames.emplace_back(rawFrame.data);
						frames.emplace_back(std::move(rawFrame));
					}
				}
				if (token.stop_requested()) {
					audioDecoder.reset(nullptr);
					return;
				} else {
					for (auto& value: frames) {
						value.guildMemberId = static_cast<DiscordCoreAPI::Song>(newSong).addedByUserId.operator size_t();
						DiscordCoreAPI::DiscordCoreClient::getSongAPI(this->guildId)->audioDataBuffer.send(std::move(value));
					}
				}
				if (token.stop_requested()) {
					audioDecoder.reset(nullptr);
					return;
				}
				++counter;
				std::this_thread::sleep_for(1ms);
			}
			DiscordCoreAPI::AudioFrameData frameData01{};
			while (audioDecoder->getFrame(frameData01)) {
			};
			audioDecoder.reset(nullptr);
			DiscordCoreAPI::AudioFrameData frameData{};
			frameData.type = DiscordCoreAPI::AudioFrameType::Skip;
			frameData.currentSize = 0;
			DiscordCoreAPI::DiscordCoreClient::getSongAPI(this->guildId)->audioDataBuffer.send(std::move(frameData));
		} catch (...) {
			if (this->configManager->doWePrintHttpsErrorMessages()) {
				DiscordCoreAPI::reportException("SoundCloudAPI::downloadAndStreamAudio()");
			}
			++currentReconnectTries;
			DiscordCoreAPI::DiscordCoreClient::getVoiceConnection(this->guildId)->clearAudioData();
			this->weFailedToDownloadOrDecode(newSong, token, currentReconnectTries);
		}
	};

	std::vector<DiscordCoreAPI::Song> SoundCloudAPI::searchForSong(const std::string& searchQuery) {
		return this->collectSearchResults(searchQuery);
	}

	DiscordCoreAPI::Song SoundCloudAPI::collectFinalSong(DiscordCoreAPI::Song& newSong) {
		return SoundCloudRequestBuilder::collectFinalSong(newSong);
	}

	std::string SoundCloudRequestBuilder::clientId{};
};