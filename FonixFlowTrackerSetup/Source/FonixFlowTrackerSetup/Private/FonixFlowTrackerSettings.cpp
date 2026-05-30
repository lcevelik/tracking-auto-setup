// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "FonixFlowTrackerSettings.h"
#include "Misc/ConfigCacheIni.h"

UFonixFlowTrackerSettings::UFonixFlowTrackerSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("FonixFlow Tracker Setup");

	// Defaults
	AIAPIKey = TEXT("");
	AIEndpoint = TEXT("https://openrouter.ai/api/v1/chat/completions");
	AIModel = TEXT("anthropic/claude-sonnet-4");
	DefaultFreeDIP = TEXT("127.0.0.1");
	DefaultFreeDPort = 40000;
	DefaultOpenTrackSource = 1;
}

FName UFonixFlowTrackerSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FText UFonixFlowTrackerSettings::GetSectionText() const
{
	return FText::FromString(TEXT("FonixFlow Tracker Setup"));
}

const UFonixFlowTrackerSettings* UFonixFlowTrackerSettings::Get()
{
	return GetDefault<UFonixFlowTrackerSettings>();
}

bool UFonixFlowTrackerSettings::IsAIChatConfigured() const
{
	return !AIAPIKey.IsEmpty();
}
