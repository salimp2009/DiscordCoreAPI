// InputEventManager.hpp - Header for the "input event handlermanager" class.
// June 4, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#ifndef _INPUT_EVENT_HANDLER_
#define _INPUT_EVENT_HANDLER_

#include "../pch.h"
#include "MessageStuff.hpp"
#include "InteractionManager.hpp"

namespace DiscordCoreAPI {

	class InputEventManager {
	public:
		InputEventManager() {}

		static void initialize(MessageManager* messagesNew, DiscordCoreClient* discordCoreClientNew, DiscordCoreInternal::HttpAgentResources agentResourcesNew, shared_ptr<DiscordCoreInternal::ThreadContext> threadContextNew, InteractionManager* interactionsNew) {
			InputEventManager::messages = messagesNew;
			InputEventManager::agentResources = agentResourcesNew;
			InputEventManager::discordCoreClient = discordCoreClientNew;
			InputEventManager::interactions = interactionsNew;
			InputEventManager::threadContext = threadContextNew;
			InputEventManager::groupId = InputEventManager::threadContext->createGroup();
		}

		static void cleanup() {
			InputEventManager::threadContext->releaseGroup(InputEventManager::groupId);
		}

		static InputEventData respondToEvent(CreateFollowUpMessageData dataPackage) {
			MessageData messageData = InputEventManager::interactions->createFollowUpMessageAsync(dataPackage).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::SLASH_COMMAND_INTERACTION;
			dataPackageNewer.messageData = messageData;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::INTERACTION_FOLLOW_UP_MESSAGE;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			dataPackageNewer.interactionData.applicationId = dataPackage.interactionPackage.applicationId;
			dataPackageNewer.interactionData.id = dataPackage.interactionPackage.interactionId;
			dataPackageNewer.interactionData.token = dataPackage.interactionPackage.interactionToken;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			return dataPackageNewer;
		}

		static InputEventData respondToEvent(EditFollowUpMessageData dataPackage) {
			MessageData messageData = InputEventManager::interactions->editFollowUpMessageAsync(dataPackage).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::SLASH_COMMAND_INTERACTION;
			dataPackageNewer.messageData = messageData;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::INTERACTION_FOLLOW_UP_MESSAGE_EDIT;
			dataPackageNewer.interactionData.applicationId = dataPackage.interactionPackage.applicationId;
			dataPackageNewer.interactionData.id = dataPackage.interactionPackage.interactionId;
			dataPackageNewer.interactionData.token = dataPackage.interactionPackage.interactionToken;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			return dataPackageNewer;
		}

		static InputEventData respondToEvent(CreateInteractionResponseData dataPackage) {
			InputEventManager::interactions->createInteractionResponseAsync(dataPackage).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::SLASH_COMMAND_INTERACTION;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::INTERACTION_RESPONSE;
			dataPackageNewer.interactionData.message.components = dataPackage.data.components;
			dataPackageNewer.interactionData.applicationId = dataPackage.interactionPackage.applicationId;
			dataPackageNewer.interactionData.id = dataPackage.interactionPackage.interactionId;
			dataPackageNewer.interactionData.token = dataPackage.interactionPackage.interactionToken;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			return dataPackageNewer;
		}

		static InputEventData respondToEvent(CreateDeferredInteractionResponseData dataPackage) {
			InputEventManager::interactions->createDeferredInteractionResponseAsync(dataPackage).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::SLASH_COMMAND_INTERACTION;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::INTERACTION_RESPONSE_DEFERRED;
			dataPackageNewer.interactionData.id = dataPackage.interactionPackage.interactionId;
			dataPackageNewer.interactionData.token = dataPackage.interactionPackage.interactionToken;
			dataPackageNewer.interactionData.applicationId = dataPackage.interactionPackage.applicationId;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			return dataPackageNewer;
		}

		static InputEventData respondToEvent(EditInteractionResponseData dataPackage) {
			MessageData messageData = InputEventManager::interactions->editInteractionResponseAsync(dataPackage).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::SLASH_COMMAND_INTERACTION;
			dataPackageNewer.messageData = messageData;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::INTERACTION_RESPONSE_EDIT;
			dataPackageNewer.interactionData.applicationId = dataPackage.interactionPackage.applicationId;
			dataPackageNewer.interactionData.id = dataPackage.interactionPackage.interactionId;
			dataPackageNewer.interactionData.token = dataPackage.interactionPackage.interactionToken;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			return dataPackageNewer;
		}

		static InputEventData respondToEvent(CreateEphemeralInteractionResponseData dataPackage) {
			InteractionPackageData interactionPackage;
			interactionPackage.applicationId = dataPackage.interactionPackage.applicationId;
			interactionPackage.interactionId = dataPackage.interactionPackage.interactionId;
			interactionPackage.interactionToken = dataPackage.interactionPackage.interactionToken;
			CreateInteractionResponseData newData(interactionPackage);
			newData.data = dataPackage.data;
			newData.data.flags = 64;
			newData.type = dataPackage.type;
			InputEventManager::interactions->createInteractionResponseAsync(newData).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::SLASH_COMMAND_INTERACTION;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::INTERACTION_RESPONSE_EPHEMERAL;
			dataPackageNewer.interactionData.applicationId = dataPackage.interactionPackage.applicationId;
			dataPackageNewer.interactionData.id = dataPackage.interactionPackage.interactionId;
			dataPackageNewer.interactionData.token = dataPackage.interactionPackage.interactionToken;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			return dataPackageNewer;
		}

		static InputEventData respondToEvent(ReplyMessageData dataPackage) {
			Message message = InputEventManager::messages->replyAsync(dataPackage).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::REGULAR_MESSAGE;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_RESPONSE;
			dataPackageNewer.messageData = message.data;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			return dataPackageNewer;
		}

		static InputEventData respondToEvent(EditMessageData dataPackage) {
			Message message = InputEventManager::messages->editMessageAsync(dataPackage).get();
			InputEventData dataPackageNewer;
			dataPackageNewer.eventType = InputEventType::REGULAR_MESSAGE;
			dataPackageNewer.messageData = message.data;
			dataPackageNewer.inputEventResponseType = InputEventResponseType::REGULAR_MESSAGE_EDIT;
			dataPackageNewer.requesterId = dataPackage.requesterId;
			dataPackageNewer.discordCoreClient = InputEventManager::discordCoreClient;
			return dataPackageNewer;
		}

		static void respondToEvent(DeferButtonResponseData dataPackage) {
			ButtonInteractionData newData;
			newData.token = dataPackage.interactionPackage.interactionToken;
			newData.id = dataPackage.interactionPackage.interactionId;
			newData.applicationId = dataPackage.interactionPackage.applicationId;
			newData.type = InteractionType::MessageComponent;
			CreateInteractionResponseData dataPackageNew(newData);
			dataPackageNew.interactionPackage.interactionId = dataPackage.interactionPackage.interactionId;
			dataPackageNew.interactionPackage.applicationId = dataPackage.interactionPackage.applicationId;
			dataPackageNew.interactionPackage.interactionToken = dataPackage.interactionPackage.interactionToken;
			dataPackageNew.type = InteractionCallbackType::DeferredUpdateMessage;
			InputEventManager::interactions->createInteractionResponseAsync(dataPackageNew).get();
			return;
		}

		static void deleteInputEventResponse(InputEventData dataPackage, unsigned int timeDelayNew = 0) {
			if ((dataPackage.inputEventResponseType == InputEventResponseType::REGULAR_MESSAGE_RESPONSE)|| (dataPackage.inputEventResponseType == InputEventResponseType::REGULAR_MESSAGE_EDIT)) {
				DeleteMessageData deleteData(dataPackage);
				deleteData.timeDelay = timeDelayNew;
				InputEventManager::messages->deleteMessageAsync(deleteData).get();
			}
			else if (dataPackage.inputEventResponseType == InputEventResponseType::INTERACTION_FOLLOW_UP_MESSAGE || dataPackage.inputEventResponseType == InputEventResponseType::INTERACTION_FOLLOW_UP_MESSAGE_EDIT){
				DeleteFollowUpMessageData dataPackageNewer(dataPackage);
				dataPackageNewer.timeDelay = timeDelayNew;
				InputEventManager::interactions->deleteFollowUpMessageAsync(dataPackageNewer).get();
			}
			else if (dataPackage.inputEventResponseType == InputEventResponseType::INTERACTION_RESPONSE || dataPackage.inputEventResponseType == InputEventResponseType::INTERACTION_RESPONSE_EDIT 
				|| dataPackage.inputEventResponseType == InputEventResponseType::INTERACTION_RESPONSE_DEFERRED) {
				DeleteInteractionResponseData dataPackageNewer(dataPackage);
				dataPackageNewer.timeDelay = timeDelayNew;
				InputEventManager::interactions->deleteInteractionResponseAsync(dataPackageNewer).get();
			}
			return;
		}

	protected:
		static MessageManager* messages;
		static InteractionManager* interactions;
		static DiscordCoreClient* discordCoreClient;
		static DiscordCoreInternal::HttpAgentResources agentResources;
		static shared_ptr<DiscordCoreInternal::ThreadContext> threadContext;
		static unsigned int groupId;
	};
	MessageManager* InputEventManager::messages;
	InteractionManager* InputEventManager::interactions;
	DiscordCoreClient* InputEventManager::discordCoreClient;
	DiscordCoreInternal::HttpAgentResources InputEventManager::agentResources;
	shared_ptr<DiscordCoreInternal::ThreadContext> InputEventManager::threadContext;
	unsigned int InputEventManager::groupId;


}
#endif
