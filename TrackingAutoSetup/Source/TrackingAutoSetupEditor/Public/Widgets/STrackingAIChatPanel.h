// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Http.h"

/** A single message in the chat */
struct FChatMessage
{
	FString Role;    // "user" or "assistant"
	FString Content;
	FDateTime Timestamp;
};

/**
 * AI Chat panel for in-editor tracking/VP assistance.
 * Connects to OpenRouter or any OpenAI-compatible API.
 */
class TRACKINGAUTOSETUPEDITOR_API STrackingAIChatPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STrackingAIChatPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** Chat message history */
	TArray<FChatMessage> Messages;

	/** Chat display area */
	TSharedPtr<SScrollBox> ChatScrollBox;
	TSharedPtr<SVerticalBox> ChatMessagesBox;

	/** Input box */
	TSharedPtr<SMultiLineEditableTextBox> InputBox;

	/** Send button callback */
	void OnSendMessage();

	/** Input box committed callback */
	void OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitType);

	/** Add a message to the chat display */
	void AddMessageToChat(const FChatMessage& Message);

	/** Call the AI API */
	void CallAIAPI(const FString& UserMessage);

	/** Handle API response */
	void OnAPIResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

	/** Build the system prompt with VP context */
	FString BuildSystemPrompt() const;

	/** Add thinking indicator */
	void AddThinkingIndicator();

	/** Remove thinking indicator */
	void RemoveThinkingIndicator();

	/** API key storage */
	FString GetAPIKey() const;

	/** API endpoint */
	FString GetAPIEndpoint() const;

	/** Model to use */
	FString GetModel() const;

	/** Thinking indicator widget */
	TSharedPtr<SWidget> ThinkingWidget;

	/** Is waiting for response */
	bool bIsWaitingForResponse = false;
};
