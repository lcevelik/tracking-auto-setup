// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "TrackingAutoSetupEditorModule.h"
#include "TrackingAutoSetupSubsystem.h"
#include "TrackingSetupTypes.h"
#include "TrackingAutoSetupStyle.h"
#include "Widgets/STrackingAutoSetupPanel.h"
#include "Widgets/STrackingSetupWizard.h"
#include "Widgets/STrackingAIChatPanel.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "TrackingAutoSetupEditor"

const FName FTrackingAutoSetupEditorModule::MainPanelTabId("TrackingAutoSetupMainPanel");
const FName FTrackingAutoSetupEditorModule::WizardTabId("TrackingAutoSetupWizard");
const FName FTrackingAutoSetupEditorModule::AIChatTabId("TrackingAutoSetupAIChat");

void FTrackingAutoSetupEditorModule::StartupModule()
{
	// Initialize custom style (icons)
	FTrackingAutoSetupStyle::Initialize();

	// Register tab spawners
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MainPanelTabId,
		FOnSpawnTab::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSpawnMainPanelTab))
		.SetDisplayName(LOCTEXT("MainPanelTab", "Tracking Auto Setup"))
		.SetTooltipText(LOCTEXT("MainPanelTabTooltip", "Open Tracking Auto Setup panel"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.TabIcon"));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(WizardTabId,
		FOnSpawnTab::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSpawnWizardTab))
		.SetDisplayName(LOCTEXT("WizardTab", "Tracking Setup Wizard"))
		.SetTooltipText(LOCTEXT("WizardTabTooltip", "Open the tracking setup wizard"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.TabIcon"));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AIChatTabId,
		FOnSpawnTab::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSpawnAIChatTab))
		.SetDisplayName(LOCTEXT("AIChatTab", "Tracking AI Assistant"))
		.SetTooltipText(LOCTEXT("AIChatTabTooltip", "Open the AI chat assistant"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.TabIcon"));

	RegisterMenus();

	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Editor: Module started with custom icons"));
}

void FTrackingAutoSetupEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MainPanelTabId);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(WizardTabId);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AIChatTabId);

	UnregisterMenus();
	FTrackingAutoSetupStyle::Shutdown();

	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Editor: Module shut down"));
}

void FTrackingAutoSetupEditorModule::RegisterMenus()
{
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTrackingAutoSetupEditorModule::RegisterMenus));
}

void FTrackingAutoSetupEditorModule::RegisterMenus()
{
	// Main toolbar button (like the Python pattern - opens the panel widget)
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = ToolbarMenu->AddSection("TrackingAutoSetup", LOCTEXT("TrackingAutoSetup", "Tracking"));

	// Main panel button with custom icon
	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"OpenMainPanel",
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenMainPanel)),
		LOCTEXT("MainPanel", "Tracking Auto Setup"),
		LOCTEXT("MainPanelTooltip", "Open Tracking Auto Setup panel with wizard, AI chat, and quick setup"),
		FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.Icon")
	));

	// Quick setup submenu
	Section.AddSubMenu(
		"QuickSetup",
		LOCTEXT("QuickSetup", "Quick Setup"),
		LOCTEXT("QuickSetupTooltip", "One-click tracking setup"),
		FNewMenuDelegate::CreateLambda([this](FMenuBuilder& SubMenuBuilder)
		{
			SubMenuBuilder.AddMenuEntry(
				LOCTEXT("QuickFreeD", "FreeD Camera"),
				LOCTEXT("QuickFreeDTooltip", "Quick FreeD setup with defaults"),
				FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.QuickSetupIcon"),
				FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupFreeDQuick))
			);
			SubMenuBuilder.AddMenuEntry(
				LOCTEXT("QuickOpenTrack", "OpenTrack Camera"),
				LOCTEXT("QuickOpenTrackTooltip", "Quick OpenTrack IO setup with defaults"),
				FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.QuickSetupIcon"),
				FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupOpenTrackQuick))
			);
		})
	);

	// Add to the Tools menu
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& ToolsSection = ToolsMenu->AddSection("TrackingAutoSetup", LOCTEXT("TrackingAutoSetupTools", "Tracking Auto Setup"));

	ToolsSection.AddMenuEntry(
		"OpenMainPanelTools",
		LOCTEXT("OpenMainPanelTools", "Tracking Auto Setup"),
		LOCTEXT("OpenMainPanelToolsTooltip", "Open the main Tracking Auto Setup panel"),
		FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.Icon"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenMainPanel))
	);

	ToolsSection.AddMenuEntry(
		"OpenWizardTools",
		LOCTEXT("OpenWizardTools", "Tracking Setup Wizard"),
		LOCTEXT("OpenWizardToolsTooltip", "Open the tracking setup wizard"),
		FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.WizardIcon"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenWizard))
	);

	ToolsSection.AddMenuEntry(
		"OpenAIChatTools",
		LOCTEXT("OpenAIChatTools", "Tracking AI Assistant"),
		LOCTEXT("OpenAIChatToolsTooltip", "Open the AI chat assistant"),
		FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.AIChatIcon"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenAIChat))
	);
}

void FTrackingAutoSetupEditorModule::UnregisterMenus()
{
	UToolMenus::UnRegisterOwnerByPluginName("TrackingAutoSetup");
}

TSharedRef<SDockTab> FTrackingAutoSetupEditorModule::OnSpawnMainPanelTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("MainPanelLabel", "Tracking Auto Setup"))
		[
			SNew(STrackingAutoSetupPanel)
		];
}

TSharedRef<SDockTab> FTrackingAutoSetupEditorModule::OnSpawnWizardTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("WizardLabel", "Tracking Wizard"))
		[
			SNew(STrackingSetupWizard)
		];
}

TSharedRef<SDockTab> FTrackingAutoSetupEditorModule::OnSpawnAIChatTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("AIChatLabel", "Tracking AI"))
		[
			SNew(STrackingAIChatPanel)
		];
}

void FTrackingAutoSetupEditorModule::OnOpenMainPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MainPanelTabId);
}

void FTrackingAutoSetupEditorModule::OnOpenWizard()
{
	FGlobalTabmanager::Get()->TryInvokeTab(WizardTabId);
}

void FTrackingAutoSetupEditorModule::OnOpenAIChat()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AIChatTabId);
}

void FTrackingAutoSetupEditorModule::OnSetupFreeDQuick()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupFreeDCamera(World);
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: %s"), *Result.Message);
}

void FTrackingAutoSetupEditorModule::OnSetupOpenTrackQuick()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupOpenTrackCamera(World);
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: %s"), *Result.Message);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTrackingAutoSetupEditorModule, TrackingAutoSetupEditor)
