// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "TrackingAutoSetupEditorModule.h"
#include "TrackingAutoSetupSubsystem.h"
#include "TrackingSetupTypes.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Toolkits/AssetEditorManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "TrackingAutoSetupEditor"

static const FName TrackingSetupTabName("TrackingAutoSetup");

void FTrackingAutoSetupEditorModule::StartupModule()
{
	RegisterMenus();
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Editor: Module started"));
}

void FTrackingAutoSetupEditorModule::ShutdownModule()
{
	UnregisterMenus();
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Editor: Module shut down"));
}

void FTrackingAutoSetupEditorModule::RegisterMenus()
{
	// Register toolbar extension
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTrackingAutoSetupEditorModule::RegisterMenus));
}

void FTrackingAutoSetupEditorModule::UnregisterMenus()
{
	UToolMenus::UnRegisterOwnerByPluginName("TrackingAutoSetup");
}

void FTrackingAutoSetupEditorModule::RegisterMenus()
{
	// Add to the Level Editor toolbar
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = ToolbarMenu->AddSection("TrackingAutoSetup", LOCTEXT("TrackingAutoSetup", "Tracking"));

	// FreeD Quick Setup button
	Section.AddMenuEntry(
		"SetupFreeDCamera",
		LOCTEXT("SetupFreeD", "Setup FreeD Camera"),
		LOCTEXT("SetupFreeDTooltip", "One-click FreeD camera tracking setup"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupFreeDCamera))
	);

	// OpenTrack Quick Setup button
	Section.AddMenuEntry(
		"SetupOpenTrackCamera",
		LOCTEXT("SetupOpenTrack", "Setup OpenTrack Camera"),
		LOCTEXT("SetupOpenTrackTooltip", "One-click OpenTrack IO camera tracking setup"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupOpenTrackCamera))
	);

	// Full setup panel
	Section.AddMenuEntry(
		"OpenSetupPanel",
		LOCTEXT("OpenPanel", "Tracking Setup Panel"),
		LOCTEXT("OpenPanelTooltip", "Open the full tracking configuration panel"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings"),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnOpenSetupPanel))
	);

	// Add to the Tools menu
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& ToolsSection = ToolsMenu->AddSection("TrackingAutoSetup", LOCTEXT("TrackingAutoSetupTools", "Tracking Auto Setup"));

	ToolsSection.AddMenuEntry(
		"SetupFreeDCameraTools",
		LOCTEXT("SetupFreeDTools", "Setup FreeD Camera"),
		LOCTEXT("SetupFreeDTooltipTools", "One-click FreeD camera tracking setup"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupFreeDCamera))
	);

	ToolsSection.AddMenuEntry(
		"SetupOpenTrackCameraTools",
		LOCTEXT("SetupOpenTrackTools", "Setup OpenTrack Camera"),
		LOCTEXT("SetupOpenTrackTooltipTools", "One-click OpenTrack IO camera tracking setup"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSetupOpenTrackCamera))
	);
}

void FTrackingAutoSetupEditorModule::OnSetupFreeDCamera()
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

void FTrackingAutoSetupEditorModule::OnSetupOpenTrackCamera()
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

void FTrackingAutoSetupEditorModule::OnOpenSetupPanel()
{
	// TODO: Open a full settings panel/dialog
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Opening setup panel (not yet implemented)"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTrackingAutoSetupEditorModule, TrackingAutoSetupEditor)
