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
/// AudioEncoder.hpp - Header for the audio encoder class.
/// Aug 22, 2021
/// https://discordcoreapi.com
/// \file AudioEncoder.hpp

#pragma once

#include <discordcoreapi/FoundationEntities.hpp>
#include <opus/opus.h>

namespace DiscordCoreInternal {

	struct DiscordCoreAPI_Dll EncoderReturnData {
		std::basic_string_view<std::byte> data{};
		size_t sampleCount{};
	};

	struct DiscordCoreAPI_Dll OpusEncoderWrapper {
		struct DiscordCoreAPI_Dll OpusEncoderDeleter {
			void operator()(OpusEncoder*) noexcept;
		};

		OpusEncoderWrapper();

		EncoderReturnData encodeData(std::basic_string_view<std::byte> inputFrame);

	  protected:
		std::unique_ptr<OpusEncoder, OpusEncoderDeleter> ptr{ nullptr, OpusEncoderDeleter{} };
		std::basic_string<std::byte> encodedData{};
		const int32_t maxBufferSize{ 1276 };
		const int32_t sampleRate{ 48000 };
		const int32_t nChannels{ 2 };
	};

}// namespace DiscordCoreAPI
