// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "Widgets/SFonixFlowTrackerAIChatPanel.h"
#include "Widgets/FonixFlowAIChatTools.h"
#include "FonixFlowTrackerActions.h"
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
	ActionsPtr = InArgs._Actions;

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

		// Quick action buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 2)
		[
			BuildQuickActions()
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
				.HintText(LOCTEXT("InputHint", "Ask about tracking setup, FreeD, calibration..."))
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
		WelcomeMsg.Content = TEXT("Welcome! I'm your tracking setup assistant. I can:\n\n"
			"• Check your current setup state\n"
			"• Select cameras and configure settings\n"
			"• Run the tracking setup for you\n"
			"• Guide you through lens calibration\n"
			"• Answer FreeD/OpenTrack questions\n\n"
			"Try the quick actions below, or ask me anything!");
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

TSharedRef<SWidget> SFonixFlowTrackerAIChatPanel::BuildQuickActions()
{
	auto MakeActionBtn = [this](FText Label, FString Prompt) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.Text(Label)
			.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
			.TextStyle(FAppStyle::Get(), "SmallText")
			.OnClicked_Lambda([this, Prompt]() -> FReply
			{
				if (!bIsWaitingForResponse)
				{
					FText Input = FText::FromString(Prompt);
					OnInputTextCommitted(Input, ETextCommit::OnEnter);
				}
				return FReply::Handled();
			})
			.IsEnabled_Lambda([this]() -> bool
			{
				const UFonixFlowTrackerSettings* Settings = UFonixFlowTrackerSettings::Get();
				return Settings && !Settings->AIAPIKey.IsEmpty() && !bIsWaitingForResponse;
			});
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
		[ MakeActionBtn(LOCTEXT("BtnStatus", "Status"), TEXT("What's my current setup state?")) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
		[ MakeActionBtn(LOCTEXT("BtnSetup", "Setup"), TEXT("Set up tracking for my camera")) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
		[ MakeActionBtn(LOCTEXT("BtnCalibrate", "Calibrate"), TEXT("Guide me through lens calibration")) ]
		+ SHorizontalBox::Slot().AutoWidth()
		[ MakeActionBtn(LOCTEXT("BtnHelp", "Help"), TEXT("What can you do?")) ];
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
				.Text(LOCTEXT("NoAPIKey", "AI Chat requires an API key"))
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

	FText RoleLabel;
	FLinearColor BorderColor;

	if (Message.Role == TEXT("user"))
	{
		RoleLabel = LOCTEXT("UserLabel", "You");
		BorderColor = FLinearColor(0.15f, 0.2f, 0.3f);
	}
	else if (Message.Role == TEXT("tool"))
	{
		RoleLabel = FText::FromString(FString::Printf(TEXT("Action: %s"), *Message.ToolName));
		BorderColor = FLinearColor(0.15f, 0.3f, 0.15f);
	}
	else
	{
		RoleLabel = LOCTEXT("AssistantLabel", "Assistant");
		BorderColor = FLinearColor(0.22f, 0.22f, 0.22f);
	}

	ChatMessagesBox->AddSlot()
	.AutoHeight()
	.Padding(4, 2)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FSlateColor(BorderColor))
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
		.BorderBackgroundColor(FSlateColor(FLinearColor(0.2f, 0.2f, 0.25f)))
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
	FString Prompt = TEXT(
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
		"- FreeD encoders: Focus distance (24-bit), Focal length (24-bit)\n"
		"- Encoder range calibration: user must rotate lens to min and max positions\n"
		"- UE 5.6+ has LiveLinkFreeD and LiveLinkOpenTrackIO as built-in plugins\n\n"
		"You have access to tools that let you check and control the plugin state. "
		"ALWAYS use get_plugin_state first to understand what the user has configured before suggesting actions. "
		"When the user asks to set up tracking, use the tools to do it directly — don't just describe the steps. "
		"For calibration: guide the user to physically rotate the lens, then use capture_calibration to record values. "
		"Keep answers concise and actionable."
	);

	// Inject current state
	if (ActionsPtr)
	{
		FFonixFlowTrackerState State = ActionsPtr->GetState();
		Prompt += FString::Printf(TEXT("\n\nCurrent plugin state:\n%s"), *State.ToJSON());

		// Add available cameras
		TArray<FString> Cameras = ActionsPtr->GetAvailableCameraNames();
		if (Cameras.Num() > 0)
		{
			Prompt += FString::Printf(TEXT("\nAvailable cameras: %s"), *FString::Join(Cameras, TEXT(", ")));
		}
		else
		{
			Prompt += TEXT("\nNo CineCameraActors found in the level.");
		}
	}

	return Prompt;
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
	Writer->WriteValue(TEXT("max_tokens"), 2048);

	// Add tools
	FString ToolsJSON = FChatToolDefinition::BuildToolsJSONArray();
	TSharedPtr<FJsonObject> ToolsObj;
	{
		// Write tools as raw JSON array
		Writer->WriteRawJSONValue(TEXT("tools"), ToolsJSON);
	}

	Writer->WriteArrayStart(TEXT("messages"));

	// System message
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("role"), TEXT("system"));
	Writer->WriteValue(TEXT("content"), BuildSystemPrompt());
	Writer->WriteObjectEnd();

	// Conversation history (last 15 messages to leave room for tool calls)
	int32 StartIdx = FMath::Max(0, Messages.Num() - 15);
	for (int32 i = StartIdx; i < Messages.Num(); i++)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("role"), Messages[i].Role);
		Writer->WriteValue(TEXT("content"), Messages[i].Content);

		// Include tool_call_id for tool result messages
		if (Messages[i].Role == TEXT("tool") && !Messages[i].ToolCallId.IsEmpty())
		{
			Writer->WriteValue(TEXT("tool_call_id"), Messages[i].ToolCallId);
		}

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
					// Check for tool_calls
					const TArray<TSharedPtr<FJsonValue>>* ToolCalls;
					if (MessageObj->TryGetArrayField(TEXT("tool_calls"), ToolCalls) && ToolCalls->Num() > 0)
					{
						// AI wants to call tools
						FString ContentText = MessageObj->HasField(TEXT("content"))
							? MessageObj->GetStringField(TEXT("content"))
							: TEXT("");
						if (!ContentText.IsEmpty())
						{
							AssistantMsg.Content = ContentText;
							Messages.Add(AssistantMsg);
							AddMessageToChat(AssistantMsg);
						}

						ProcessToolCalls(*ToolCalls);
						return; // Don't add assistant message yet — tool loop continues
					}

					// No tool_calls — plain text response
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

void SFonixFlowTrackerAIChatPanel::ProcessToolCalls(const TArray<TSharedPtr<FJsonValue>>& ToolCalls)
{
	PendingToolResults.Empty();

	for (const auto& ToolCallVal : ToolCalls)
	{
		TSharedPtr<FJsonObject> ToolCall = ToolCallVal->AsObject();
		if (!ToolCall.IsValid()) continue;

		FString ToolCallId = ToolCall->GetStringField(TEXT("id"));
		TSharedPtr<FJsonObject> FunctionObj = ToolCall->GetObjectField(TEXT("function"));
		if (!FunctionObj.IsValid()) continue;

		FString FunctionName = FunctionObj->GetStringField(TEXT("name"));
		FString ArgumentsStr = FunctionObj->GetStringField(TEXT("arguments"));

		// Show action in chat
		FChatMessage ActionMsg;
		ActionMsg.Role = TEXT("tool");
		ActionMsg.ToolCallId = ToolCallId;
		ActionMsg.ToolName = FunctionName;
		ActionMsg.Timestamp = FDateTime::Now();

		// Execute the tool
		FString Result = FChatToolDefinition::ExecuteTool(FunctionName, ArgumentsStr, ActionsPtr);
		ActionMsg.Content = Result;

		Messages.Add(ActionMsg);
		PendingToolResults.Add(ActionMsg);
		AddMessageToChat(ActionMsg);
	}

	// Send tool results back to the AI
	SendToolResults();
}

void SFonixFlowTrackerAIChatPanel::SendToolResults()
{
	const UFonixFlowTrackerSettings* Settings = UFonixFlowTrackerSettings::Get();
	if (!Settings || Settings->AIAPIKey.IsEmpty()) return;

	bIsWaitingForResponse = true;
	AddThinkingIndicator();

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("model"), Settings->AIModel);
	Writer->WriteValue(TEXT("max_tokens"), 2048);

	// Add tools again
	FString ToolsJSON = FChatToolDefinition::BuildToolsJSONArray();
	Writer->WriteRawJSONValue(TEXT("tools"), ToolsJSON);

	Writer->WriteArrayStart(TEXT("messages"));

	// System message
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("role"), TEXT("system"));
	Writer->WriteValue(TEXT("content"), BuildSystemPrompt());
	Writer->WriteObjectEnd();

	// Full conversation history including tool calls and results
	int32 StartIdx = FMath::Max(0, Messages.Num() - 20);
	for (int32 i = StartIdx; i < Messages.Num(); i++)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("role"), Messages[i].Role);
		Writer->WriteValue(TEXT("content"), Messages[i].Content);

		if (Messages[i].Role == TEXT("tool") && !Messages[i].ToolCallId.IsEmpty())
		{
			Writer->WriteValue(TEXT("tool_call_id"), Messages[i].ToolCallId);
		}

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

#undef LOCTEXT_NAMESPACE
