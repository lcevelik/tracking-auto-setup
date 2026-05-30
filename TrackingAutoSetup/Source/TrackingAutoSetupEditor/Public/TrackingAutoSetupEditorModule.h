// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;

class FTrackingAutoSetupEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register toolbar buttons, menu entries, tab spawners, and styles */
	void RegisterMenus();
	void UnregisterMenus();

	/** Tab spawners */
	static const FName MainPanelTabId;
	static const FName WizardTabId;
	static const FName AIChatTabId;

	TSharedRef<SDockTab> OnSpawnMainPanelTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> OnSpawnWizardTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> OnSpawnAIChatTab(const FSpawnTabArgs& Args);

	/** Callbacks for toolbar actions */
	void OnOpenMainPanel();
	void OnOpenWizard();
	void OnOpenAIChat();
	void OnSetupFreeDQuick();
	void OnSetupOpenTrackQuick();
};
