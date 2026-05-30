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
	/** Register toolbar buttons, menu entries, and tab spawners */
	void RegisterMenus();
	void UnregisterMenus();

	/** Tab spawners */
	static const FName WizardTabId;
	static const FName AIChatTabId;

	TSharedRef<SDockTab> OnSpawnWizardTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> OnSpawnAIChatTab(const FSpawnTabArgs& Args);

	/** Callbacks for toolbar actions */
	void OnOpenWizard();
	void OnOpenAIChat();
	void OnSetupFreeDQuick();
	void OnSetupOpenTrackQuick();

	/** Delegate handles for cleanup */
	TSharedPtr<class FUICommandList> PluginCommands;
};
