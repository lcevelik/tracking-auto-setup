// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "Widgets/SFonixFlowTrackerAIChatPanel.h"
#include "FonixFlowTrackerSettings.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Styling/AppStyle.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "SFonixFlowTrackerAIChatPanel"

void SFonixFlowTrackerAIChatPanel::Construct(const FArguments& InArgs)
{
	const UFonixFlowTrackerSettings* Settings = UFonixFlowTrackerSettings::Get();
	bool bHasKey = Settings && !Settings->AIAPIKey.IsEmpty();

	ChildSlot
	[
		SNew(SVerticalBox)

		// API Key warning (if not configured)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildAPIKeyWarning().ToSharedRef()
		]

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 4)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ChatHeader", "FonixFlow Tracker — AI Assistant"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
		]

		// Chat messages area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(ChatScrollBox, SScrollBox)
			.Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SAssignNew(ChatMessagesBox, SVerticalBox)
			]
		]

		// Input area
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 4)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
			SAssignNew(InputBox, SMultiLineEditableTextBox)
			.HintText(LOCTEXT("InputHint", "Ask about tracking setup, FreeD, OpenTrack, lens calibration..."))
			.OnTextCommitted_Raw(this, &SFonixFlowTrackerAIChatPanel::OnInputTextCommitted)
			.IsReadOnly(!bHasKey)
			.AutoWrapText(true)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Send", "Send"))
				.OnClicked_Lambda([this]() -> FReply
				{
					OnSendMessage();
					return FReply::Handled();
				})
				.IsEnabled_Lambda([this, bHasKey]() -> bool
				{
					return bHasKey && !bIsWaitingForResponse;
				})
			]
		]
	];

	// Welcome message
	FChatMessage WelcomeMsg;
	WelcomeMsg.Role = TEXT("assistant");
	WelcomeMsg.Timestamp = FDateTime::Now();

	if (bHasKey)
	{
		WelcomeMsg.Content = TEXT("Welcome! I'm your tracking setup assistant. I can help with:\n\n"
			"• FreeD protocol configuration\n"
			"• OpenTrack IO setup\n"
			"• Lens calibration and encoder ranges\n"
			"• Camera tracking best practices\n"
			"• Virtual Production workflows\n\n"
			"What would you like to know?");
	}
	else
	{
		WelcomeMsg.Content = TEXT("AI Chat is not configured yet.\n\n"
			"To enable it, add your API key:\n"
			"Edit > Project Settings > Plugins > FonixFlow Tracker Setup > AI Chat > API Key\n\n"
			"Supports OpenRouter, OpenAI, or any OpenAI-compatible API.");
	}

	Messages.Add(WelcomeMsg);
	AddMessageToChat(WelcomeMsg);
}

TSharedPtr<SWidget> SFonixFlowTrackerAIChatPanel::BuildAPIKeyWarning()
{
	const UFonixFlowTrackerSettings* Settings = UFonixFlowTrackerSettings::Get();
	bool bHasKey = Settings && !Settings->AIAPIKey.IsEmpty();

	if (bHasKey)
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	}

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FSlateColor(FLinearColor(0.8f, 0.4f, 0.1f)))
		.Padding(12, 8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoAPIKey", "⚠ AI Chat requires an API key"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 4, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("APIKeyInstructions", "Go to: Edit > Project Settings > Plugins > FonixFlow Tracker Setup"))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 4, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("APIKeyHint", "Paste your OpenRouter or OpenAI API key in the 'API Key' field."))
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
		];
}

void SFonixFlowTrackerAIChatPanel::OnSendMessage()
{
	if (InputBox.IsValid())
	{
		FText InputText = InputBox->GetText();
		if (!InputText.IsEmpty())
		{
			OnInputTextCommitted(InputText, ETextCommit::OnEnter);
		}
	}
}

void SFonixFlowTrackerAIChatPanel::OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType != ETextCommit::OnEnter || Text.IsEmpty() || bIsWaitingForResponse)
	{
		return;
	}

	FString UserMessage = Text.ToString();

	FChatMessage UserMsg;
	UserMsg.Role = TEXT("user");
	UserMsg.Content = UserMessage;
	UserMsg.Timestamp = FDateTime::Now();
	Messages.Add(UserMsg);
	AddMessageToChat(UserMsg);

	InputBox->SetText(FText::GetEmpty());
	CallAIAPI(UserMessage);
}

void SFonixFlowTrackerAIChatPanel::AddMessageToChat(const FChatMessage& Message)
{
	if (!ChatMessagesBox.IsValid()) return;

	FText RoleLabel = Message.Role == TEXT("user")
		? LOCTEXT("UserLabel", "You")
		: LOCTEXT("AssistantLabel", "Assistant");

	ChatMessagesBox->AddSlot()
	.AutoHeight()
	.Padding(4, 2)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(STextBlock).Text(RoleLabel).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
				+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
				[ SNew(STextBlock).Text(FText::FromString(Message.Timestamp.ToString(TEXT("%H:%M"))))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
					.ColorAndOpacity(FSlateColor(FLinearColor::Gray)) ]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 4, 0, 0)
			[ SNew(STextBlock).Text(FText::FromString(Message.Content)).AutoWrapText(true) ]
		]
	];

	if (ChatScrollBox.IsValid()) ChatScrollBox->ScrollToEnd();
}

void SFonixFlowTrackerAIChatPanel::AddThinkingIndicator()
{
	if (!ChatMessagesBox.IsValid()) return;
	RemoveThinkingIndicator();

	ChatMessagesBox->AddSlot()
	.AutoHeight()
	.Padding(4, 2)
	[
		SAssignNew(ThinkingWidget, SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Thinking", "Thinking..."))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
			.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
		]
	];

	if (ChatScrollBox.IsValid()) ChatScrollBox->ScrollToEnd();
}

void SFonixFlowTrackerAIChatPanel::RemoveThinkingIndicator()
{
	if (ThinkingWidget.IsValid())
	{
		ChatMessagesBox->RemoveSlot(ThinkingWidget.ToSharedRef());
		ThinkingWidget.Reset();
	}
}

FString SFonixFlowTrackerAIChatPanel::BuildSystemPrompt() const
{
	return TEXT(
		"You are a Virtual Production tracking setup assistant embedded in Unreal Engine. "
		"You specialize in:\n"
		"- FreeD protocol (camera tracking data: PTZ, encoded heads)\n"
		"- OpenTrack IO (open protocol for head/camera tracking)\n"
		"- UE Live Link framework for real-time data streaming\n"
		"- Lens calibration (encoder ranges, focus/zoom mapping)\n"
		"- Virtual Camera systems\n"
		"- Camera tracking workflows (LED volume, green screen)\n\n"
		"Key facts:\n"
		"- FreeD default UDP port: 40000\n"
		"- OpenTrack multicast port: 55555, multicast address: 235.135.1.[SourceNumber]\n"
		"- FreeD encoders: Focus distance (24-bit), Focal length (24-bit), User defined (16-bit)\n"
		"- Encoder range calibration: user must rotate lens to min and max positions\n"
		"- UE 5.6 has LiveLinkFreeD and LiveLinkOpenTrackIO as built-in plugins\n\n"
		"Keep answers concise and actionable. Include UE Blueprint paths and C++ class names when relevant."
	);
}

void SFonixFlowTrackerAIChatPanel::CallAIAPI(const FString& UserMessage)
{
	const UFonixFlowTrackerSettings* Settings = UFonixFlowTrackerSettings::Get();
	if (!Settings || Settings->AIAPIKey.IsEmpty())
	{
		FChatMessage ErrorMsg;
		ErrorMsg.Role = TEXT("assistant");
		ErrorMsg.Content = TEXT("API key not configured. Go to Edit > Project Settings > Plugins > FonixFlow Tracker Setup");
		ErrorMsg.Timestamp = FDateTime::Now();
		Messages.Add(ErrorMsg);
		AddMessageToChat(ErrorMsg);
		return;
	}

	bIsWaitingForResponse = true;
	AddThinkingIndicator();

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("model"), Settings->AIModel);
	Writer->WriteValue(TEXT("max_tokens"), 1024);
	Writer->WriteArrayStart(TEXT("messages"));

	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("role"), TEXT("system"));
	Writer->WriteValue(TEXT("content"), BuildSystemPrompt());
	Writer->WriteObjectEnd();

	int32 StartIdx = FMath::Max(0, Messages.Num() - 10);
	for (int32 i = StartIdx; i < Messages.Num(); i++)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("role"), Messages[i].Role);
		Writer->WriteValue(TEXT("content"), Messages[i].Content);
		Writer->WriteObjectEnd();
	}

	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd();
	Writer->Close();

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Settings->AIEndpoint);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings->AIAPIKey));
	HttpRequest->SetHeader(TEXT("HTTP-Referer"), TEXT("https://github.com/lcevelik/fonixflow-tracker-setup"));
	HttpRequest->SetHeader(TEXT("X-Title"), TEXT("FonixFlow Tracker Setup"));
	HttpRequest->SetContentAsString(RequestBody);
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &SFonixFlowTrackerAIChatPanel::OnAPIResponse);
	HttpRequest->ProcessRequest();
}

void SFonixFlowTrackerAIChatPanel::OnAPIResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	bIsWaitingForResponse = false;
	RemoveThinkingIndicator();

	FChatMessage AssistantMsg;
	AssistantMsg.Role = TEXT("assistant");
	AssistantMsg.Timestamp = FDateTime::Now();

	if (!bSuccess || !Response.IsValid())
	{
		AssistantMsg.Content = TEXT("Failed to get response. Check your API key and network connection.");
		Messages.Add(AssistantMsg);
		AddMessageToChat(AssistantMsg);
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	FString ResponseBody = Response->GetContentAsString();

	if (ResponseCode != 200)
	{
		AssistantMsg.Content = FString::Printf(TEXT("API error (HTTP %d): %s"), ResponseCode, *ResponseBody);
		Messages.Add(AssistantMsg);
		AddMessageToChat(AssistantMsg);
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* Choices;
		if (JsonObject->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
		{
			TSharedPtr<FJsonObject> Choice = (*Choices)[0]->AsObject();
			if (Choice.IsValid())
			{
				TSharedPtr<FJsonObject> MessageObj = Choice->GetObjectField(TEXT("message"));
				if (MessageObj.IsValid())
				{
					AssistantMsg.Content = MessageObj->GetStringField(TEXT("content"));
				}
			}
		}
		else
		{
			AssistantMsg.Content = TEXT("Unexpected API response format.");
		}
	}
	else
	{
		AssistantMsg.Content = TEXT("Failed to parse API response.");
	}

	Messages.Add(AssistantMsg);
	AddMessageToChat(AssistantMsg);
}

#undef LOCTEXT_NAMESPACE
