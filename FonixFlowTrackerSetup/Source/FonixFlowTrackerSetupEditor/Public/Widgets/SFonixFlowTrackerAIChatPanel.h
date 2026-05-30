// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Http.h"

struct FChatMessage
{
	FString Role;
	FString Content;
	FDateTime Timestamp;
};

class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerAIChatPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerAIChatPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
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

	TSharedPtr<SWidget> ThinkingWidget;
	bool bIsWaitingForResponse = false;

	/** Show warning if API key not configured */
	TSharedPtr<SWidget> BuildAPIKeyWarning();
};
