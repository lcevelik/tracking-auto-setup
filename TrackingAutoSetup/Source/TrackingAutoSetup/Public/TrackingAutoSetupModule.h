// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FTrackingAutoSetupModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get the singleton instance */
	static FTrackingAutoSetupModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FTrackingAutoSetupModule>("TrackingAutoSetup");
	}

	/** Check if the module is loaded and available */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("TrackingAutoSetup");
	}
};
