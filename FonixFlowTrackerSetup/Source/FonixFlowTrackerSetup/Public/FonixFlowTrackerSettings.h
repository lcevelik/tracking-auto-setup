// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DeveloperSettings.h"
#include "FonixFlowTrackerSettings.generated.h"

/**
 * Plugin settings for FonixFlow Tracker Setup.
 * Accessible via Edit > Project Settings > Plugins > FonixFlow Tracker Setup
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="FonixFlow Tracker Setup"))
class FONIXFLOWTRACKERSETUP_API UFonixFlowTrackerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFonixFlowTrackerSettings();

	//~ Begin UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;
	//~ End UDeveloperSettings interface

	/** OpenRouter API key (or any OpenAI-compatible API) */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AI Chat", meta = (DisplayName = "API Key"))
	FString AIAPIKey;

	/** API endpoint URL */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AI Chat", meta = (DisplayName = "API Endpoint"))
	FString AIEndpoint;

	/** Model to use for AI chat */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AI Chat", meta = (DisplayName = "Model"))
	FString AIModel;

	/** Default FreeD IP address */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Defaults", meta = (DisplayName = "Default FreeD IP"))
	FString DefaultFreeDIP;

	/** Default FreeD port */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Defaults", meta = (DisplayName = "Default FreeD Port"))
	int32 DefaultFreeDPort;

	/** Default OpenTrack source number */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Defaults", meta = (DisplayName = "Default OpenTrack Source"))
	int32 DefaultOpenTrackSource;

	/** Get the singleton settings instance */
	static const UFonixFlowTrackerSettings* Get();

	/** Check if AI chat is configured (has API key) */
	bool IsAIChatConfigured() const;
};
