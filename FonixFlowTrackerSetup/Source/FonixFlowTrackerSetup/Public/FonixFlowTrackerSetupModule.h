// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFonixFlowTrackerSetupModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get the singleton instance */
	static FFonixFlowTrackerSetupModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FFonixFlowTrackerSetupModule>("FonixFlowTrackerSetup");
	}

	/** Check if the module is loaded and available */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("FonixFlowTrackerSetup");
	}
};
