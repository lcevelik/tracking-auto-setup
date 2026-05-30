// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;

class FFonixFlowTrackerSetupEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	static const FName MainPanelTabId;

	TSharedRef<SDockTab> OnSpawnMainPanelTab(const FSpawnTabArgs& Args);
	void OnOpenMainPanel();
	void OnSetupFreeDQuick();
	void OnSetupOpenTrackQuick();
};
