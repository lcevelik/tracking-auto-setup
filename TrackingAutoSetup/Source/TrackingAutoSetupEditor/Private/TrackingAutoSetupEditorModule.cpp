// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "TrackingAutoSetupEditorModule.h"
#include "TrackingAutoSetupSubsystem.h"
#include "TrackingSetupTypes.h"
#include "Widgets/STrackingSetupWizard.h"
#include "Widgets/STrackingAIChatPanel.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "TrackingAutoSetupEditor"

const FName FTrackingAutoSetupEditorModule::WizardTabId("TrackingAutoSetupWizard");
const FName FTrackingAutoSetupEditorModule::AIChatTabId("TrackingAutoSetupAIChat");

void FTrackingAutoSetupEditorModule::StartupModule()
{
	// Register tab spawners
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(WizardTabId,
		FOnSpawnTab::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSpawnWizardTab))
		.SetDisplayName(LOCTEXT("WizardTab", "Tracking Setup Wizard"))
		.SetTooltipText(LOCTEXT("WizardTabTooltip", "Open the tracking setup wizard"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings"));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AIChatTabId,
		FOnSpawnTab::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSpawnAIChatTab))
		.SetDisplayName(LOCTEXT("AIChatTab", "Tracking AI Assistant"))
		.SetTooltipText(LOCTEXT("AIChatTabTooltip", "Open the AI chat assistant for tracking questions"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Icons.Help"));

	RegisterMenus();

	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Editor: Module started"));
}

void FTrackingAutoSetupEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(WizardTabId);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AIChatTabId);
	UnregisterMenus();

	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Editor: Module shut down"));
}

void FTrackingAutoSetupEditorModule::RegisterMenus()
{
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTrackingAutoSetupEditorModule::RegisterMenus));
}

void FTrackingAutoSetupEditorModule::RegisterMenus()
{
	// Add to the Level Editor toolbar
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = ToolbarMenu->AddSection("TrackingAutoSetup", LOCTEXT("TrackingAutoSetup", "Tracking"));

	// Wizard button
	Section.AddMenuEntry(
		"OpenWizard",
		LOCTEXT("OpenWizard", "Tracking Wizard"),
		LOCTEXT("OpenWizardTooltip", "Open the tracking setup wizard"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenWizard))
	);

	// AI Chat button
	Section.AddMenuEntry(
		"OpenAIChat",
		LOCTEXT("OpenAIChat", "Tracking AI"),
		LOCTEXT("OpenAIChatTooltip", "Open the AI chat assistant"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Icons.Help"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenAIChat))
	);

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
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupFreeDQuick))
			);
			SubMenuBuilder.AddMenuEntry(
				LOCTEXT("QuickOpenTrack", "OpenTrack Camera"),
				LOCTEXT("QuickOpenTrackTooltip", "Quick OpenTrack IO setup with defaults"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupOpenTrackQuick))
			);
		})
	);

	// Add to the Tools menu
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& ToolsSection = ToolsMenu->AddSection("TrackingAutoSetup", LOCTEXT("TrackingAutoSetupTools", "Tracking Auto Setup"));

	ToolsSection.AddMenuEntry(
		"OpenWizardTools",
		LOCTEXT("OpenWizardTools", "Tracking Setup Wizard"),
		LOCTEXT("OpenWizardToolsTooltip", "Open the tracking setup wizard"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenWizard))
	);

	ToolsSection.AddMenuEntry(
		"OpenAIChatTools",
		LOCTEXT("OpenAIChatTools", "Tracking AI Assistant"),
		LOCTEXT("OpenAIChatToolsTooltip", "Open the AI chat assistant for tracking questions"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenAIChat))
	);
}

void FTrackingAutoSetupEditorModule::UnregisterMenus()
{
	UToolMenus::UnRegisterOwnerByPluginName("TrackingAutoSetup");
}

TSharedRef<SDockTab> FTrackingAutoSetupEditorModule::OnSpawnWizardTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STrackingSetupWizard)
		];
}

TSharedRef<SDockTab> FTrackingAutoSetupEditorModule::OnSpawnAIChatTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STrackingAIChatPanel)
		];
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

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: %s"), *Result.Message);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TrackingAutoSetup: Setup failed - %s"), *Result.Message);
	}
}

void FTrackingAutoSetupEditorModule::OnSetupOpenTrackQuick()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupOpenTrackCamera(World);

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: %s"), *Result.Message);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TrackingAutoSetup: Setup failed - %s"), *Result.Message);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTrackingAutoSetupEditorModule, TrackingAutoSetupEditor)
