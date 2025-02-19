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
/// StickerEntities.cpp - Source file for the sticker related classes and structs.
/// May 13, 2021
/// https://discordcoreapi.com
/// \file StickerEntities.cpp

#include <discordcoreapi/StickerEntities.hpp>
#include <discordcoreapi/Https.hpp>

namespace DiscordCoreAPI {

	Sticker::Sticker(simdjson::ondemand::value jsonObjectData) {
		this->asset = getString(jsonObjectData, "asset");

		this->description = getString(jsonObjectData, "description");

		this->formatType = static_cast<StickerFormatType>(getUint8(jsonObjectData, "format_type"));

		uint8_t newFlags{};
		newFlags = setBool(newFlags, StickerFlags::Available, getBool(jsonObjectData, "available"));

		this->flags = static_cast<StickerFlags>(newFlags);

		this->guildId = getId(jsonObjectData, "guild_id");

		this->packId = getString(jsonObjectData, "pack_id");

		this->type = static_cast<StickerType>(getUint8(jsonObjectData, "type"));

		this->sortValue = getUint32(jsonObjectData, "sort_value");

		this->name = getString(jsonObjectData, "name");

		this->id = getId(jsonObjectData, "id");

		simdjson::ondemand::value object{};
		if (getObject(object, "user", jsonObjectData)) {
			this->user = UserData{ object };
		}
	}

	StickerVector::StickerVector(simdjson::ondemand::value jsonObjectData) {
		if (jsonObjectData.type() != simdjson::ondemand::json_type::null) {
			simdjson::ondemand::array arrayValue{};
			if (getArray(arrayValue, jsonObjectData)) {
				for (simdjson::simdjson_result<simdjson::ondemand::value> value: arrayValue) {
					Sticker newData{ value.value() };
					this->stickers.emplace_back(std::move(newData));
				}
			}
		}
	}

	StickerVector::operator std::vector<Sticker>() {
		return this->stickers;
	}

	void Stickers::initialize(DiscordCoreInternal::HttpsClient* client) {
		Stickers::httpsClient = client;
	}

	CoRoutine<Sticker> Stickers::getStickerAsync(GetStickerData dataPackage) {
		DiscordCoreInternal::HttpsWorkloadData workload{ DiscordCoreInternal::HttpsWorkloadType::Get_Sticker };
		co_await NewThreadAwaitable<Sticker>();
		workload.workloadClass = DiscordCoreInternal::HttpsWorkloadClass::Get;
		workload.relativePath = "/stickers/" + dataPackage.stickerId;
		workload.callStack = "Stickers::getStickerAsync()";
		Sticker returnValue{};
		co_return Stickers::httpsClient->submitWorkloadAndGetResult<Sticker>(workload, &returnValue);
	}

	CoRoutine<std::vector<StickerPackData>> Stickers::getNitroStickerPacksAsync() {
		DiscordCoreInternal::HttpsWorkloadData workload{ DiscordCoreInternal::HttpsWorkloadType::Get_Nitro_Sticker_Packs };
		co_await NewThreadAwaitable<std::vector<StickerPackData>>();
		workload.workloadClass = DiscordCoreInternal::HttpsWorkloadClass::Get;
		workload.relativePath = "/sticker-packs";
		workload.callStack = "Stickers::getNitroStickerPacksAsync()";
		StickerPackDataVector returnValue{};
		co_return Stickers::httpsClient->submitWorkloadAndGetResult<StickerPackDataVector>(workload, &returnValue);
	}

	CoRoutine<std::vector<Sticker>> Stickers::getGuildStickersAsync(GetGuildStickersData dataPackage) {
		DiscordCoreInternal::HttpsWorkloadData workload{ DiscordCoreInternal::HttpsWorkloadType::Get_Guild_Stickers };
		co_await NewThreadAwaitable<std::vector<Sticker>>();
		workload.workloadClass = DiscordCoreInternal::HttpsWorkloadClass::Get;
		workload.relativePath = "/guilds/" + dataPackage.guildId + "/stickers";
		workload.callStack = "Stickers::getGuildStickersAsync()";
		StickerVector returnValue{};
		co_return Stickers::httpsClient->submitWorkloadAndGetResult<StickerVector>(workload, &returnValue);
	}

	CoRoutine<Sticker> Stickers::createGuildStickerAsync(CreateGuildStickerData dataPackage) {
		DiscordCoreInternal::HttpsWorkloadData workload{ DiscordCoreInternal::HttpsWorkloadType::Post_Guild_Sticker };
		co_await NewThreadAwaitable<Sticker>();
		workload.workloadClass = DiscordCoreInternal::HttpsWorkloadClass::Post;
		workload.relativePath = "/guilds/" + dataPackage.guildId + "/stickers";
		Jsonifier data{};
		data["description"] = dataPackage.description;
		data["name"] = dataPackage.name;
		data["tags"] = dataPackage.tags;
		data["file"] = dataPackage.file;
		data.refreshString(JsonifierSerializeType::Json);
		workload.content = data.operator std::string();
		workload.callStack = "Stickers::createGuildStickerAsync()";
		if (dataPackage.reason != "") {
			workload.headersToInsert["X-Audit-Log-Reason"] = dataPackage.reason;
		}
		Sticker returnValue{};
		co_return Stickers::httpsClient->submitWorkloadAndGetResult<Sticker>(workload, &returnValue);
	}

	CoRoutine<Sticker> Stickers::modifyGuildStickerAsync(ModifyGuildStickerData dataPackage) {
		DiscordCoreInternal::HttpsWorkloadData workload{ DiscordCoreInternal::HttpsWorkloadType::Patch_Guild_Sticker };
		co_await NewThreadAwaitable<Sticker>();
		workload.workloadClass = DiscordCoreInternal::HttpsWorkloadClass::Patch;
		workload.relativePath = "/guilds/" + dataPackage.guildId + "/stickers/" + dataPackage.stickerId;
		Jsonifier data{};
		data["description"] = dataPackage.description;
		data["name"] = dataPackage.name;
		data["tags"] = dataPackage.tags;
		data.refreshString(JsonifierSerializeType::Json);
		workload.content = data.operator std::string();
		workload.callStack = "Stickers::modifyGuildStickerAsync()";
		if (dataPackage.reason != "") {
			workload.headersToInsert["X-Audit-Log-Reason"] = dataPackage.reason;
		}
		Sticker returnValue{};
		co_return Stickers::httpsClient->submitWorkloadAndGetResult<Sticker>(workload, &returnValue);
	}

	CoRoutine<void> Stickers::deleteGuildStickerAsync(DeleteGuildStickerData dataPackage) {
		DiscordCoreInternal::HttpsWorkloadData workload{ DiscordCoreInternal::HttpsWorkloadType::Delete_Guild_Sticker };
		co_await NewThreadAwaitable<void>();
		workload.workloadClass = DiscordCoreInternal::HttpsWorkloadClass::Delete;
		workload.relativePath = "/guilds/" + dataPackage.guildId + "/stickers/" + dataPackage.stickerId;
		workload.callStack = "Stickers::deleteGuildStickerAsync()";
		if (dataPackage.reason != "") {
			workload.headersToInsert["X-Audit-Log-Reason"] = dataPackage.reason;
		}
		co_return Stickers::httpsClient->submitWorkloadAndGetResult<void>(workload);
	}
	DiscordCoreInternal::HttpsClient* Stickers::httpsClient{ nullptr };
};
