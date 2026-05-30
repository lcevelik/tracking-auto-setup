// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "TrackingAutoSetupEditorModule.h"
#include "TrackingAutoSetupSubsystem.h"
#include "TrackingSetupTypes.h"
#include "TrackingAutoSetupStyle.h"
#include "Widgets/STrackingAutoSetupPanel.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "TrackingAutoSetupEditor"

const FName FTrackingAutoSetupEditorModule::MainPanelTabId("TrackingAutoSetupMainPanel");

void FTrackingAutoSetupEditorModule::StartupModule()
{
	// 1. Register custom icons
	FTrackingAutoSetupStyle::Initialize();

	// 2. Register the tab that the toolbar button opens
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MainPanelTabId,
		FOnSpawnTab::CreateRaw(this, &FTrackingAutoSetupEditorModule::OnSpawnMainPanelTab))
		.SetDisplayName(LOCTEXT("PanelTab", "Tracking Auto Setup"))
		.SetTooltipText(LOCTEXT("PanelTabTooltip", "Tracking Auto Setup"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(), "TrackingAutoSetup.TabIcon"));

	// 3. Register toolbar button automatically (no Python, no extra steps)
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
		{
			// Toolbar button
			UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(
				"LevelEditor.LevelEditorToolBar.PlayToolBar");

			FToolMenuSection& Section = ToolbarMenu->AddSection(
				"TrackingAutoSetup",
				LOCTEXT("Section", "Tracking"));

			// Main button - click opens the panel
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				"OpenTrackingSetup",
				FUIAction(FExecuteAction::CreateRaw(
					this, &FTrackingAutoSetupEditorModule::OnOpenMainPanel)),
				LOCTEXT("Button", "Tracking Auto Setup"),
				LOCTEXT("Tooltip", "Open Tracking Auto Setup"),
				FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(),
					"TrackingAutoSetup.Icon")));

			// Quick setup submenu
			Section.AddSubMenu(
				"QuickSetup",
				LOCTEXT("QuickSetup", "Quick Setup"),
				LOCTEXT("QuickSetupTip", "One-click tracking setup"),
				FNewMenuDelegate::CreateLambda(
					[this](FMenuBuilder& Sub)
					{
						Sub.AddMenuEntry(
							LOCTEXT("FreeD", "FreeD Camera"),
							LOCTEXT("FreeDTip", "Quick FreeD setup"),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(
								this,
								&FTrackingAutoSetupEditorModule::OnSetupFreeDQuick)));

						Sub.AddMenuEntry(
							LOCTEXT("OpenTrack", "OpenTrack Camera"),
							LOCTEXT("OpenTrackTip", "Quick OpenTrack setup"),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(
								this,
								&FTrackingAutoSetupEditorModule::OnSetupOpenTrackQuick)));
					}));

			// Tools menu entry
			UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(
				"LevelEditor.MainMenu.Tools");

			FToolMenuSection& ToolsSection = ToolsMenu->AddSection(
				"TrackingAutoSetup",
				LOCTEXT("ToolsSection", "Tracking Auto Setup"));

			ToolsSection.AddMenuEntry(
				"OpenTrackingSetupTools",
				LOCTEXT("ToolsEntry", "Tracking Auto Setup"),
				LOCTEXT("ToolsEntryTip", "Open Tracking Auto Setup"),
				FSlateIcon(FTrackingAutoSetupStyle::GetStyleName(),
					"TrackingAutoSetup.Icon"),
				FUIAction(FExecuteAction::CreateRaw(
					this, &FTrackingAutoSetupEditorModule::OnOpenMainPanel)));
		}));

	UE_LOG(LogTemp, Log,
		TEXT("TrackingAutoSetup: Plugin loaded — toolbar button registered"));
}

void FTrackingAutoSetupEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MainPanelTabId);
	UToolMenus::UnRegisterOwnerByPluginName("TrackingAutoSetup");
	FTrackingAutoSetupStyle::Shutdown();
}

TSharedRef<SDockTab> FTrackingAutoSetupEditorModule::OnSpawnMainPanelTab(
	const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("TabLabel", "Tracking Auto Setup"))
		[
			SNew(STrackingAutoSetupPanel)
		];
}

void FTrackingAutoSetupEditorModule::OnOpenMainPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MainPanelTabId);
}

void FTrackingAutoSetupEditorModule::OnSetupFreeDQuick()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FTrackingSetupResult Result =
		UTrackingAutoSetupSubsystem::SetupFreeDCamera(World);
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: %s"), *Result.Message);
}

void FTrackingAutoSetupEditorModule::OnSetupOpenTrackQuick()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FTrackingSetupResult Result =
		UTrackingAutoSetupSubsystem::SetupOpenTrackCamera(World);
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: %s"), *Result.Message);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTrackingAutoSetupEditorModule, TrackingAutoSetupEditor)
