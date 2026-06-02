// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Http.h"

class IFonixFlowTrackerActions;

struct FChatMessage
{
	FString Role;
	FString Content;
	FDateTime Timestamp;
	/** For tool_calls: the tool call ID this message responds to */
	FString ToolCallId;
	/** For tool_calls: the tool name */
	FString ToolName;
};

class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerAIChatPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerAIChatPanel) {}
		/** Reference to the setup panel for executing actions */
		SLATE_ARGUMENT(IFonixFlowTrackerActions*, Actions)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** Actions interface (back-pointer to setup panel) */
	IFonixFlowTrackerActions* ActionsPtr = nullptr;

	TArray<FChatMessage> Messages;
	TSharedPtr<SScrollBox> ChatScrollBox;
	TSharedPtr<SVerticalBox> ChatMessagesBox;
	TSharedPtr<SMultiLineEditableTextBox> InputBox;

	void OnSendMessage();
	void OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitType);
	void AddMessageToChat(const FChatMessage& Message);
	void CallAIAPI(const FString& UserMessage);
	void OnAPIResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	FString BuildSystemPrompt() const;
	void AddThinkingIndicator();
	void RemoveThinkingIndicator();

	/** Process tool_calls from the AI response, execute them, and continue the conversation */
	void ProcessToolCalls(const TArray<TSharedPtr<FJsonValue>>& ToolCalls);

	/** Send a follow-up request with tool results */
	void SendToolResults();

	/** Pending tool results to send in next request */
	TArray<FChatMessage> PendingToolResults;

	/** Whether we're in a tool-calling loop */
	bool bInToolLoop = false;

	TSharedPtr<SWidget> ThinkingWidget;
	bool bIsWaitingForResponse = false;

	/** Show warning if API key not configured */
	TSharedPtr<SWidget> BuildAPIKeyWarning();

	/** Build quick action buttons */
	TSharedRef<SWidget> BuildQuickActions();
};
