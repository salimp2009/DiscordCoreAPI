// Index.hpp - An index file to round everything up.
// Jun 7, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#ifndef _INDEX_
#define _INDEX_

#include "DiscordCoreClient.hpp"
#include "Commands\CommandsList.hpp"

namespace DiscordCoreAPI {
    class IndexHost {
    public:

        static shared_ptr<DiscordCoreClient> discordCoreClient;

        static void onChannelCreation(OnChannelCreationData dataPackage) {
            IndexHost::discordCoreClient->channels->insertChannelAsync(dataPackage.channel).get();
        }

        static void onChannelUpdate(OnChannelUpdateData dataPackage) {
            IndexHost::discordCoreClient->channels->insertChannelAsync(dataPackage.channelNew).get();
        }

        static void onChannelDeletion(OnChannelDeletionData dataPackage) {
            IndexHost::discordCoreClient->channels->removeChannelAsync(dataPackage.channel.data.id).get();
        }

        static void onGuildCreation(OnGuildCreationData dataPackage) {
            IndexHost::discordCoreClient->guilds->insertGuildAsync(dataPackage.guild).get();
        }

        static void onGuildUpdate(OnGuildUpdateData dataPackage) {
            IndexHost::discordCoreClient->guilds->insertGuildAsync(dataPackage.guildNew).get();
        }

        static void onGuildDeletion(OnGuildDeletionData dataPackage) {
            IndexHost::discordCoreClient->guilds->removeGuildAsync(dataPackage.guild.data.id).get();
        }

        static void onGuildMemberAdd(OnGuildMemberAddData dataPackage) {
            IndexHost::discordCoreClient->guildMembers->insertGuildMemberAsync(dataPackage.guildMember, dataPackage.guildMember.data.guildId).get();
            Guild guild = dataPackage.guildMember.discordCoreClient->guilds->getGuildAsync({ dataPackage.guildMember.data.guildId }).get();
            guild.data.memberCount += 1;
            IndexHost::discordCoreClient->guilds->insertGuildAsync(guild).get();
        }

        static void onGuildMemberRemove(OnGuildMemberRemoveData dataPackage) {
            IndexHost::discordCoreClient->guildMembers->removeGuildMemberAsync(dataPackage.guildId, dataPackage.user.data.id).get();
            Guild guild = IndexHost::discordCoreClient->guilds->getGuildAsync({ dataPackage.guildId }).get();
            guild.data.memberCount -= 1;
            IndexHost::discordCoreClient->guilds->insertGuildAsync(guild).get();
        }

        static void onGuildMemberUpdate(OnGuildMemberUpdateData dataPackage) {
            IndexHost::discordCoreClient->guildMembers->insertGuildMemberAsync(dataPackage.guildMemberNew, dataPackage.guildMemberNew.data.guildId).get();
        }

        static void onRoleCreation(OnRoleCreationData dataPackage) {
            IndexHost::discordCoreClient->roles->insertRoleAsync(dataPackage.role).get();
        }

        static void onRoleUpdate(OnRoleUpdateData dataPackage) {
            IndexHost::discordCoreClient->roles->insertRoleAsync(dataPackage.roleNew).get();
        }

        static void onRoleDeletion(OnRoleDeletionData dataPackage) {
            IndexHost::discordCoreClient->roles->removeRoleAsync(dataPackage.roleOld.data.id).get();
        }

        static task<void> onInteractionCreation(OnInteractionCreationData dataPackage) {
            apartment_context mainThread;
            co_await resume_background();
            try {
                if (dataPackage.eventData.eventType == InputEventType::REGULAR_MESSAGE || dataPackage.eventData.eventType == InputEventType::SLASH_COMMAND_INTERACTION) {
                    CommandData commandData(dataPackage.eventData);
                    commandData.eventData = dataPackage.eventData;
                    CommandCenter::checkForAndRunCommand(commandData);
                }
                else if (dataPackage.eventData.eventType == InputEventType::BUTTON_INTERACTION) {
                    ButtonInteractionData dataPackageNew;
                    dataPackageNew.applicationId = dataPackage.eventData.getApplicationId();
                    dataPackageNew.channelId = dataPackage.eventData.getChannelId();
                    dataPackageNew.customId = dataPackage.eventData.getInteractionData().customId;
                    dataPackageNew.guildId = dataPackage.eventData.getGuildId();
                    dataPackageNew.id = dataPackage.eventData.getInteractionId();
                    dataPackageNew.member = dataPackage.eventData.getInteractionData().member;
                    dataPackageNew.message = dataPackage.eventData.getInteractionData().message;
                    dataPackageNew.token = dataPackage.eventData.getInteractionToken();
                    dataPackageNew.type = dataPackage.eventData.getInteractionData().type;
                    dataPackageNew.user = dataPackage.eventData.getInteractionData().user;
                    if (Button::buttonInteractionMap.contains(dataPackageNew.channelId + dataPackageNew.message.id)) {
                        asend(Button::buttonInteractionMap.at(dataPackageNew.channelId + dataPackageNew.message.id), dataPackageNew);
                    }
                }
                else if (dataPackage.eventData.eventType == InputEventType::SELECT_MENU_INPUT) {
                    SelectMenuInteractionData dataPackageNew;
                    dataPackageNew.applicationId = dataPackage.eventData.getApplicationId();
                    dataPackageNew.channelId = dataPackage.eventData.getChannelId();
                    dataPackageNew.customId = dataPackage.eventData.getInteractionData().customId;
                    dataPackageNew.guildId = dataPackage.eventData.getGuildId();
                    dataPackageNew.id = dataPackage.eventData.getInteractionId();
                    dataPackageNew.member = dataPackage.eventData.getInteractionData().member;
                    dataPackageNew.message = dataPackage.eventData.getInteractionData().message;
                    dataPackageNew.token = dataPackage.eventData.getInteractionToken();
                    dataPackageNew.values = dataPackage.eventData.getInteractionData().values;
                    dataPackageNew.type = dataPackage.eventData.getInteractionData().type;
                    dataPackageNew.user = dataPackage.eventData.getInteractionData().user;
                    if (SelectMenu::selectMenuInteractionMap.contains(dataPackageNew.channelId + dataPackageNew.message.id)) {
                        asend(SelectMenu::selectMenuInteractionMap.at(dataPackageNew.channelId + dataPackageNew.message.id), dataPackageNew);
                    }
                }
                co_await mainThread;
                co_return;
            }
            catch (const exception& e) {
                cout << "Exception: " << e.what() << endl;
            }
            catch (const winrt::hresult_invalid_argument& e) {
                cout << "Exception: " << to_string(e.message()) << endl;
            }
            catch (winrt::hresult_error& e) {
                cout << "Exception: " << to_string(e.message()) << endl;
            }
            co_await mainThread;
            co_return;
        }

        static void onVoiceStateUpdate(OnVoiceStateUpdateData dataPackage) {
            map<string, GuildMember> guildMemberMap;
            if (try_receive(GuildMemberManagerAgent::cache, guildMemberMap)) {
                if (guildMemberMap.contains(dataPackage.voiceStateData.guildId + " + " + dataPackage.voiceStateData.userId)) {
                    GuildMember guildMember = guildMemberMap.at(dataPackage.voiceStateData.guildId + " + " + dataPackage.voiceStateData.userId);
                    guildMemberMap.erase(dataPackage.voiceStateData.guildId + " + " + dataPackage.voiceStateData.userId);
                    guildMember.data.voiceData = dataPackage.voiceStateData;
                    guildMemberMap.insert(std::make_pair(dataPackage.voiceStateData.guildId + " + " + dataPackage.voiceStateData.userId, guildMember));
                }
                asend(GuildMemberManagerAgent::cache, guildMemberMap);
            }
            map<string, Guild> guildMap;
            if (try_receive(GuildManagerAgent::cache, guildMap)) {
                if (dataPackage.voiceStateData.userId == IndexHost::discordCoreClient->currentUser->data.id) {
                    if (dataPackage.voiceStateData.channelId == "") {
                        Guild guild = guildMap.at(dataPackage.voiceStateData.guildId);
                        guild.disconnectFromVoice();
                        guildMap.erase(dataPackage.voiceStateData.guildId);
                        guildMap.insert(make_pair(dataPackage.voiceStateData.guildId, guild));
                    }
                }
                asend(GuildManagerAgent::cache, guildMap);
            }
        }
    };

    void DiscordCoreClient::finalSetup(string botToken) {
        try {
            DiscordCoreInternal::ThreadManager::intialize();
            shared_ptr<DiscordCoreClient> pDiscordCoreClient = make_shared<DiscordCoreClient>(to_hstring(botToken));
            DiscordCoreClient::thisPointer = pDiscordCoreClient;
            IndexHost::discordCoreClient = pDiscordCoreClient;
            pDiscordCoreClient->initialize().get();
            executeFunctionAfterTimePeriod([]() {
                cout << "Heart beat!" << endl << endl;
                }, 60000, true);
            pDiscordCoreClient->eventManager->onChannelCreation(&IndexHost::onChannelCreation);
            pDiscordCoreClient->eventManager->onChannelUpdate(&IndexHost::onChannelUpdate);
            pDiscordCoreClient->eventManager->onChannelDeletion(&IndexHost::onChannelDeletion);
            pDiscordCoreClient->eventManager->onGuildCreation(&IndexHost::onGuildCreation);
            pDiscordCoreClient->eventManager->onGuildUpdate(&IndexHost::onGuildUpdate);
            pDiscordCoreClient->eventManager->onGuildDeletion(&IndexHost::onGuildDeletion);
            pDiscordCoreClient->eventManager->onGuildMemberAdd(&IndexHost::onGuildMemberAdd);
            pDiscordCoreClient->eventManager->onGuildMemberRemove(&IndexHost::onGuildMemberRemove);
            pDiscordCoreClient->eventManager->onGuildMemberUpdate(&IndexHost::onGuildMemberUpdate);
            pDiscordCoreClient->eventManager->onRoleCreation(&IndexHost::onRoleCreation);
            pDiscordCoreClient->eventManager->onRoleUpdate(&IndexHost::onRoleUpdate);
            pDiscordCoreClient->eventManager->onRoleDeletion(&IndexHost::onRoleDeletion);
            pDiscordCoreClient->eventManager->onInteractionCreation(&IndexHost::onInteractionCreation);
            pDiscordCoreClient->eventManager->onVoiceStateUpdate(&IndexHost::onVoiceStateUpdate);
            CommandCenter::registerFunction("botinfo", new BotInfo);
            CommandCenter::registerFunction("play", new Play);
            CommandCenter::registerFunction("test", new Test);
        }
        catch (exception& e) {
            cout << "DiscordCoreAPI::finalSetup Error: " << e.what() << endl;
        }
    };

    void DiscordCoreClient::runBot() {
        wait((agent*)DiscordCoreClient::thisPointer.get());
        exception error;
        while (DiscordCoreClient::getError(error)) {
            cout << "DiscordCoreClient Error: " << error.what() << endl;
        }
    }
    shared_ptr<DiscordCoreClient> IndexHost::discordCoreClient{ nullptr };
}
#endif
