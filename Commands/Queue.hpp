// Queue.hpp - Header for the "Queue" command.
// Sep 1, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#ifndef _QUEUE_
#define _QUEUE_

#include "../DiscordCoreClient02.hpp"

namespace DiscordCoreAPI {

    vector<EmbedData> updateMessageEmbeds(vector<Song> playlist, DiscordGuild discordGuild, InputEventData interaction,InputEventData originalEvent, int currentPageIndex ){
        vector<vector<EmbedFieldData>> msgEmbedFields{};
        msgEmbedFields.push_back(vector<EmbedFieldData>());
        int msgEmbedFieldsPage{ 0 };
       for (int y = 0; y < playlist.size(); y += 1) {
           if (y % 25 == 0 && y > 0) {
               msgEmbedFieldsPage += 1;
               msgEmbedFields.push_back(vector<EmbedFieldData>());
           }
           EmbedFieldData msgEmbedField = { .Inline = false,.value = "__**Title:**__ [" + playlist.at(y).songTitle + "](" + playlist.at(y).viewURL + ")\n__**Added By:**__ <@!" + playlist.at(y).addedByUserId + "> (" + playlist.at(y).addedByUserName + ")", .name = "__**" + to_string(y + 1) + " of " + to_string(playlist.size()) + "**__" };
           msgEmbedFields[msgEmbedFieldsPage].push_back(msgEmbedField);
       }
       msgEmbedFieldsPage = 0;
       vector<EmbedData> newMsgEmbeds{};
       for (int y = 0; y < msgEmbedFields.size(); y += 1) {
           EmbedData msgEmbed;
           msgEmbed.setAuthor(originalEvent.getUserName(), originalEvent.getAvatarURL());
           msgEmbed.setColor(discordGuild.data.borderColor);
           msgEmbed.setTimeStamp(getTimeAndDate());
           msgEmbed.setTitle("__**Playlist, Page " + to_string(y + 1) + " of " + to_string(msgEmbedFields.size()) + "**__");
           msgEmbed.setFooter("React with ✅ to edit the contents of the current page. React with ❌ to exit!");
           msgEmbed.setDescription("__**React with ✅ to edit the contents of the current page. React with ❌ to exit!**__")->fields = msgEmbedFields[y];
           newMsgEmbeds.push_back(msgEmbed);
       }
       if (interaction.eventType == InputEventType::REGULAR_MESSAGE) {
           RespondToInputEventData dataPackage(originalEvent);
           dataPackage.type = DesiredInputEventResponseType::RegularMessageEdit;
           dataPackage.addMessageEmbed(newMsgEmbeds[currentPageIndex]);
           dataPackage.addContent("");
           dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
           dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
           dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
           dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
           InputEvents::respondToEvent(dataPackage);
       }
       else {
           RespondToInputEventData dataPackage(originalEvent);
           dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
           dataPackage.addMessageEmbed(newMsgEmbeds[currentPageIndex]);
           dataPackage.addContent("");
           dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
           dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
           dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
           dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
           InputEvents::respondToEvent(dataPackage);
       }
       return newMsgEmbeds;
    }

	class Queue :public BaseFunction {
	public:

		Queue() {
			this->commandName = "queue";
            this->helpDescription = "View and edit the song queue.";
            EmbedData msgEmbed;
            msgEmbed.setDescription("------\nSimply enter !queue or /queue, and follow the instructions!\n------");
            msgEmbed.setTitle("__**Queue Usage:**__");
            msgEmbed.setTimeStamp(getTimeAndDate());
            msgEmbed.setColor("FeFeFe");
            this->helpEmbed = msgEmbed;
		}

		Queue* create() {
			return new Queue;
		}

        virtual task<void> execute(shared_ptr<BaseFunctionArguments>args) {
            Channel channel = Channels::getChannelAsync({ args->eventData.getChannelId() }).get();

            bool areWeInADm = areWeInADM(args->eventData, channel);

            if (areWeInADm) {
                co_return;
            }

            InputEvents::deleteInputEventResponseAsync(args->eventData).get();

            Guild guild = Guilds::getGuildAsync({ args->eventData.getGuildId() }).get();
            DiscordGuild discordGuild(guild);

            bool checkIfAllowedInChannel = checkIfAllowedPlayingInChannel(args->eventData, discordGuild);

            if (!checkIfAllowedInChannel) {
                co_return;
            }

            GuildMember guildMember = GuildMembers::getGuildMemberAsync({ .guildMemberId = args->eventData.getAuthorId(),.guildId = args->eventData.getGuildId() }).get();

            bool doWeHaveControl = checkIfWeHaveControl(args->eventData, discordGuild, guildMember);

            if (!doWeHaveControl) {
                co_return;
            }

            shared_ptr<VoiceConnection>* voiceConnectionRaw = guild.connectToVoice(guildMember.voiceData.channelId);

            if (voiceConnectionRaw == nullptr) {
                EmbedData newEmbed;
                newEmbed.setAuthor(args->eventData.getUserName(), args->eventData.getAvatarURL());
                newEmbed.setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
                newEmbed.setTimeStamp(getTimeAndDate());
                newEmbed.setTitle("__**Connection Issue:**__");
                newEmbed.setColor(discordGuild.data.borderColor);
                if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                    RespondToInputEventData dataPackage(args->eventData);
                    dataPackage.type = DesiredInputEventResponseType::RegularMessage;
                    dataPackage.addMessageEmbed(newEmbed);
                    auto newEvent = InputEvents::respondToEvent(dataPackage);
                    InputEvents::deleteInputEventResponseAsync(newEvent, 20000).get();
                }
                else {
                    RespondToInputEventData dataPackage(args->eventData);
                    dataPackage.type = DesiredInputEventResponseType::EphemeralInteractionResponse;
                    dataPackage.addMessageEmbed(newEmbed);
                    auto newEvent = InputEvents::respondToEvent(dataPackage);
                }
                co_return;
            }

            auto voiceConnection = *voiceConnectionRaw;

            if (guildMember.voiceData.channelId == "" || guildMember.voiceData.channelId != voiceConnection->getChannelId()) {
                EmbedData newEmbed;
                newEmbed.setAuthor(args->eventData.getUserName(), args->eventData.getAvatarURL());
                newEmbed.setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
                newEmbed.setTimeStamp(getTimeAndDate());
                newEmbed.setTitle("__**Voice Channel Issue:**__");
                newEmbed.setColor(discordGuild.data.borderColor);
                if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                    RespondToInputEventData dataPackage(args->eventData);
                    dataPackage.type = DesiredInputEventResponseType::RegularMessage;
                    dataPackage.addMessageEmbed(newEmbed);
                    auto newEvent = InputEvents::respondToEvent(dataPackage);
                    InputEvents::deleteInputEventResponseAsync(newEvent, 20000).get();
                }
                else {
                    RespondToInputEventData dataPackage(args->eventData);
                    dataPackage.type = DesiredInputEventResponseType::EphemeralInteractionResponse;
                    dataPackage.addMessageEmbed(newEmbed);
                    auto newEvent = InputEvents::respondToEvent(dataPackage);
                }
                co_return;
            }

            InputEventData newEvent{ args->eventData };

            if (SongAPI::getPlaylist(guild.id).songQueue.size() == 0) {
                EmbedData msgEmbed;
                msgEmbed.setAuthor(args->eventData.getUserName(), args->eventData.getAvatarURL());
                msgEmbed.setColor(discordGuild.data.borderColor);
                msgEmbed.setTimeStamp(getTimeAndDate());
                msgEmbed.setTitle("__**Empty Playlist:**__");
                msgEmbed.setDescription("------\n__**Sorry, but there is nothing here to display!**__\n------");
                if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                    RespondToInputEventData dataPackage(args->eventData);
                    dataPackage.type = DesiredInputEventResponseType::RegularMessage;
                    dataPackage.addMessageEmbed(msgEmbed);
                    newEvent = InputEvents::respondToEvent(dataPackage);
                    InputEvents::deleteInputEventResponseAsync(newEvent, 20000).get();
                }
                else {
                    RespondToInputEventData dataPackage(args->eventData);
                    dataPackage.type = DesiredInputEventResponseType::EphemeralInteractionResponse;
                    dataPackage.addMessageEmbed(msgEmbed);
                    newEvent = InputEvents::respondToEvent(dataPackage);
                }
                co_return;
            }

            int currentPageIndex = 0;
            if (args->eventData.eventType == InputEventType::SLASH_COMMAND_INTERACTION) {
                RespondToInputEventData dataPackage(args->eventData);
                dataPackage.type = DesiredInputEventResponseType::DeferredResponse;
                newEvent = InputEvents::respondToEvent(dataPackage);
            }

            vector<vector<EmbedFieldData>> msgEmbedFields;
            msgEmbedFields.push_back(vector<EmbedFieldData>());
            int msgEmbedFieldsPage{ 0 };
            for (int y = 0; y < SongAPI::getPlaylist(guild.id).songQueue.size(); y += 1) {
                if (y % 25 == 0 && y > 0) {
                    if (y > 0) {
                        msgEmbedFieldsPage += 1;
                    }
                    msgEmbedFields.push_back(vector<EmbedFieldData>());
                }
                EmbedFieldData msgEmbedField = { .Inline = false,.value = "__**Title:**__ [" + SongAPI::getPlaylist(guild.id).songQueue.at(y).songTitle + "](" + SongAPI::getPlaylist(guild.id).songQueue.at(y).viewURL+ ")\n__**Added By:**__ <@!" +
                    SongAPI::getPlaylist(guild.id).songQueue.at(y).addedByUserId + "> (" + SongAPI::getPlaylist(guild.id).songQueue.at(y).addedByUserName + ")",.name = "__**" + to_string(y + 1) + " of " + to_string(SongAPI::getPlaylist(guild.id).songQueue.size()) + "**__" };
                msgEmbedFields[msgEmbedFieldsPage].push_back(msgEmbedField);
            }
            vector<EmbedData> msgEmbeds;
            msgEmbedFieldsPage = 0;
            for (int y = 0; y < msgEmbedFields.size(); y += 1) {
                EmbedData msgEmbed;
                msgEmbed.setAuthor(args->eventData.getUserName(), args->eventData.getAvatarURL())->setColor(discordGuild.data.borderColor)->setTimeStamp(getTimeAndDate())->setTitle("__**Playlist, Page " + to_string(y + 1) + " of " + to_string(msgEmbedFields.size()) + "**__")->
                    setFooter("React with ✅ to edit the contents of the current page. React with ❌ to exit!")->setDescription("__**React with ✅ to edit the contents of the current page. React with ❌ to exit!**__")->fields = msgEmbedFields[y];
                msgEmbeds.push_back(msgEmbed);
            }
            if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                RespondToInputEventData dataPackage(args->eventData);
                dataPackage.type = DesiredInputEventResponseType::RegularMessage;
                dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                dataPackage.addContent("");
                dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                newEvent = InputEvents::respondToEvent(dataPackage);
            }
            else {
                RespondToInputEventData dataPackage(args->eventData);
                dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
                dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                dataPackage.addContent("");
                dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                newEvent = InputEvents::respondToEvent(dataPackage);
            }
            for (int y = 0; y < 1; y) {
                Button button(newEvent);
                auto buttonCollectedData = button.collectButtonData(false, 120000, args->eventData.getAuthorId());
                string userID = args->eventData.getAuthorId();
                if (buttonCollectedData.size() == 0 || buttonCollectedData.at(0).buttonId == "exit") {
                    if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::RegularMessageEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    else {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    break;
                }
                else if (buttonCollectedData.at(0).buttonId == "next" && (currentPageIndex == (msgEmbeds.size() - 1))) {
                    currentPageIndex = 0;
                    if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::RegularMessageEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    else {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    continue;
                }
                else if (buttonCollectedData.at(0).buttonId == "next" && (currentPageIndex < msgEmbeds.size())) {
                    currentPageIndex += 1;
                    if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::RegularMessageEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    else {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    continue;
                }
                else if (buttonCollectedData.at(0).buttonId == "back" && (currentPageIndex > 0)) {
                    currentPageIndex -= 1;
                    if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::RegularMessageEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    else {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    continue;
                }
                else if (buttonCollectedData.at(0).buttonId == "back" && (currentPageIndex == 0)) {
                    currentPageIndex = (int)msgEmbeds.size() - 1;
                    if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::RegularMessageEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    else {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        dataPackage.addButton(false, "check", "Edit", "✅", ButtonStyle::Success);
                        dataPackage.addButton(false, "back", "Back", "◀️", ButtonStyle::Success);
                        dataPackage.addButton(false, "next", "Next", "▶️", ButtonStyle::Success);
                        dataPackage.addButton(false, "exit", "Exit", "❌", ButtonStyle::Success);
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    continue;
                }
                else if (buttonCollectedData.at(0).buttonId == "check") {
                bool doWeQuit{ false };
                msgEmbeds[currentPageIndex].setDescription("__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.__\n");
                msgEmbeds[currentPageIndex].setFooter("Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
                while (!doWeQuit) {
                    if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::RegularMessageEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }
                    else {
                        RespondToInputEventData dataPackage(args->eventData);
                        dataPackage.type = DesiredInputEventResponseType::InteractionResponseEdit;
                        dataPackage.addMessageEmbed(msgEmbeds[currentPageIndex]);
                        dataPackage.addContent("");
                        newEvent = InputEvents::respondToEvent(dataPackage);
                    }

                    function<bool(Message)> messageFilter = [=](Message message)-> bool {
                        if (userID == message.author.id) {
                            return true;
                        }
                        else {
                            return false;
                        }
                    };

                    MessageCollector messageCollector(1, 120000, userID, messageFilter);
                    auto returnedMessages = messageCollector.collectMessages().get();
                    if (returnedMessages.messages.size() == 0) {
                        msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex, msgEmbeds.begin() + currentPageIndex + 1);
                        msgEmbeds = updateMessageEmbeds(SongAPI::getPlaylist(guild.id).songQueue, discordGuild, newEvent, args->eventData, currentPageIndex);
                        doWeQuit = true;
                        break;
                    }
                    vector<string> args2;
                    string newString = convertToLowerCase(returnedMessages.messages.at(0).content);
                    regex wordRegex("[a-z]{1,12}");
                    smatch wordRegexMatch;
                    regex_search(newString, wordRegexMatch, wordRegex, regex_constants::match_flag_type::match_any | regex_constants::match_flag_type::match_not_null | regex_constants::match_flag_type::match_prev_avail);
                    args2.push_back(wordRegexMatch.str());
                    regex_iterator<const char*>::regex_type rx("\\d{1,4}");
                    regex_iterator<const char*> next(newString.c_str(), newString.c_str() + strlen(newString.c_str()), rx);
                    regex_iterator<const char*> end;

                    for (; next != end; ++next) {
                        args2.push_back(next->str());
                    }
                    regex digitRegex("\\d{1,3}");
                    if (args2.size() == 0 || convertToLowerCase(args2[0]) == "exit") {
                        InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                        dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                        InputEvents::deleteInputEventResponseAsync(dataPackage).get();
                        msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex, msgEmbeds.begin() + currentPageIndex + 1);
                        msgEmbeds = updateMessageEmbeds(SongAPI::getPlaylist(guild.id).songQueue, discordGuild, newEvent, args->eventData, currentPageIndex);
                        doWeQuit = true;
                        break;
                    }
                    else if (convertToLowerCase(args2[0]) != "remove" && convertToLowerCase(args2[0]) != "swap" && convertToLowerCase(args2[0]) != "exit" && convertToLowerCase(args2[0]) != "shuffle") {
                        msgEmbeds[currentPageIndex].setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType exit to exit.__\n");
                        msgEmbeds[currentPageIndex].setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
                        InputEventData dataPackage02(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                        dataPackage02.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                        InputEvents::deleteInputEventResponseAsync(dataPackage02).get();
                        if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                            RespondToInputEventData dataPackage03(args->eventData);
                            dataPackage03.type = DesiredInputEventResponseType::RegularMessageEdit;
                            dataPackage03.addMessageEmbed(msgEmbeds[currentPageIndex]);
                            dataPackage03.addContent("");
                            newEvent = InputEvents::respondToEvent(dataPackage03);
                        }
                        else {
                            RespondToInputEventData dataPackage03(args->eventData);
                            dataPackage03.type = DesiredInputEventResponseType::InteractionResponseEdit;
                            dataPackage03.addMessageEmbed(msgEmbeds[currentPageIndex]);
                            dataPackage03.addContent("");
                            newEvent = InputEvents::respondToEvent(dataPackage03);
                        }
                        continue;
                    }
                    else if (convertToLowerCase(args2[0]) == "remove") {
                        if (args2.size() < 2 || !regex_search(args2[1].c_str(), digitRegex)) {
                            msgEmbeds[currentPageIndex].setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType exit to exit.__\n");
                            msgEmbeds[currentPageIndex].setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
                            InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                            dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                            InputEvents::deleteInputEventResponseAsync(dataPackage).get();;
                            if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::RegularMessageEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            else {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::InteractionResponseEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            continue;
                        }
                        if ((stoll(args2[1]) - 1) < 0 || (size_t)(stoll(args2[1]) - 1) >= SongAPI::getPlaylist(guild.id).songQueue.size() || args2.size() < 1) {
                            msgEmbeds[currentPageIndex].setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType exit to exit.__\n");
                            msgEmbeds[currentPageIndex].setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
                            InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                            dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                            InputEvents::deleteInputEventResponseAsync(dataPackage).get();;
                            if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::RegularMessageEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            else {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::InteractionResponseEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            continue;
                        }
                        int removeIndex = (int)stoll(args2[1]);

                        auto playlist = SongAPI::getPlaylist(guild.id);
                        playlist.songQueue.erase(playlist.songQueue.begin() + removeIndex - 1, playlist.songQueue.begin() + removeIndex);
                        SongAPI::setPlaylist(playlist, guild.id);
                        InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                        dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                        InputEvents::deleteInputEventResponseAsync(dataPackage).get();;
                        msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex, msgEmbeds.begin() + currentPageIndex + 1);
                        msgEmbeds = updateMessageEmbeds(SongAPI::getPlaylist(guild.id).songQueue, discordGuild, newEvent, args->eventData, currentPageIndex);
                        doWeQuit = true;
                        break;
                    }
                    else if (convertToLowerCase(args2[0]) == "swap") {
                        if (args2.size() < 3 || !regex_search(args2[1].c_str(), digitRegex) || !regex_search(args2[2].c_str(), digitRegex)) {
                            msgEmbeds[currentPageIndex].setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType exit to exit.__\n");
                            msgEmbeds[currentPageIndex].setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
                            InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                            dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                            InputEvents::deleteInputEventResponseAsync(dataPackage).get();;
                            if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::RegularMessageEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            else {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::InteractionResponseEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            continue;
                        }
                        if (args2.size() < 2|| ((stoll(args2[1]) - 1) < 0 || (size_t)(stoll(args2[1]) - 1) >= SongAPI::getPlaylist(guild.id).songQueue.size() || (stoll(args2[2]) - 1) < 0 || (size_t)(stoll(args2[2]) - 1) >= SongAPI::getPlaylist(guild.id).songQueue.size() || args2.size() < 2)) {
                            msgEmbeds[currentPageIndex].setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType exit to exit.__\n");
                            msgEmbeds[currentPageIndex].setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> <destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
                            InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                            dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                            InputEvents::deleteInputEventResponseAsync(dataPackage).get();;
                            if (args->eventData.eventType == InputEventType::REGULAR_MESSAGE) {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::RegularMessageEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            else {
                                RespondToInputEventData dataPackage02(args->eventData);
                                dataPackage02.type = DesiredInputEventResponseType::InteractionResponseEdit;
                                dataPackage02.addMessageEmbed(msgEmbeds[currentPageIndex]);
                                dataPackage02.addContent("");
                                newEvent = InputEvents::respondToEvent(dataPackage02);
                            }
                            continue;
                        }

                        int sourceIndex = (int)stoll(args2[1]) - 1;
                        int destinationIndex = (int)stoll(args2[2]) - 1;
                        SongAPI::modifyQueue(sourceIndex, destinationIndex, guild.id);
                        InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                        dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                        InputEvents::deleteInputEventResponseAsync(dataPackage).get();;
                        msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex, msgEmbeds.begin() + currentPageIndex + 1);
                        msgEmbeds = updateMessageEmbeds(SongAPI::getPlaylist(guild.id).songQueue, discordGuild, newEvent, args->eventData, currentPageIndex);
                        doWeQuit = true;
                        break;
                    }
                    else if (convertToLowerCase(args2[0]) == "shuffle") {
                        auto oldSongArray = SongAPI::getPlaylist(guild.id);
                        vector<Song> newVector{};
                        while (oldSongArray.songQueue.size() > 0) {
                            srand((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
                            int randomIndex = (unsigned int)trunc(((float)rand() / (float)RAND_MAX) * oldSongArray.songQueue.size());
                            newVector.push_back(oldSongArray.songQueue.at(randomIndex));
                            oldSongArray.songQueue.erase(oldSongArray.songQueue.begin() + randomIndex, oldSongArray.songQueue.begin() + randomIndex + 1);
                        }
                        oldSongArray.songQueue = newVector;
                        SongAPI::setPlaylist(oldSongArray, guild.id);
                        InputEventData dataPackage(returnedMessages.messages.at(0), InteractionData(), InputEventType::REGULAR_MESSAGE, args->eventData.discordCoreClient);
                        dataPackage.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
                        InputEvents::deleteInputEventResponseAsync(dataPackage).get();;
                        msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex, msgEmbeds.begin() + currentPageIndex + 1);
                        msgEmbeds = updateMessageEmbeds(SongAPI::getPlaylist(guild.id).songQueue, discordGuild, newEvent, args->eventData, currentPageIndex);
                        doWeQuit = true;
                        break;
                    }
                    
                }
                if (doWeQuit) {
                    discordGuild.data.playlist = SongAPI::getPlaylist(guild.id);
                    discordGuild.writeDataToDB();
                    continue;
                }
                    
                };
                co_return;
            };
        };

	};

};
#endif