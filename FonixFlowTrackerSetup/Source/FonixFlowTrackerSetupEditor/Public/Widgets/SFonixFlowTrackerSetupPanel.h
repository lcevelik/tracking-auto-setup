// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"

class SFonixFlowTrackerWizard;
class SFonixFlowTrackerAIChatPanel;

/**
 * Main panel widget for FonixFlow Tracker Setup plugin.
 * Contains tabs for Wizard, AI Chat, and Settings.
 * Opened from the toolbar button.
 */
class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerSetupPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerSetupPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** Tab identifiers */
	static const FName WizardTabId;
	static const FName AIChatTabId;
	static const FName SettingsTabId;

	/** Tab widget */
	TSharedPtr<SDockTab> MainTab;
	TSharedPtr<SWidget> TabManager;

	/** Child widgets */
	TSharedPtr<SFonixFlowTrackerWizard> WizardWidget;
	TSharedPtr<SFonixFlowTrackerAIChatPanel> AIChatWidget;

	/** Build tab content */
	TSharedRef<SDockTab> OnSpawnWizardTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> OnSpawnAIChatTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> OnSpawnSettingsTab(const FSpawnTabArgs& Args);

	/** Build the panel header with icon */
	TSharedRef<SWidget> BuildHeader();
};
