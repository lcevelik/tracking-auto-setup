// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FTrackingAutoSetupEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register toolbar buttons and menu entries */
	void RegisterMenus();
	
	/** Unregister toolbar buttons and menu entries */
	void UnregisterMenus();

	/** Callback for the "Setup FreeD Camera" action */
	void OnSetupFreeDCamera();
	
	/** Callback for the "Setup OpenTrack Camera" action */
	void OnSetupOpenTrackCamera();
	
	/** Callback for the "Tracking Setup Panel" action */
	void OnOpenSetupPanel();

	/** Delegate handles for cleanup */
	TSharedPtr<class FUICommandList> PluginCommands;
};
