// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "FonixFlowTrackerSetupEditorModule.h"
#include "FonixFlowTrackerSetupSubsystem.h"
#include "FonixFlowTrackerSetupTypes.h"
#include "FonixFlowTrackerSetupStyle.h"
#include "Widgets/SFonixFlowTrackerSetupPanel.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "FonixFlowTrackerSetupEditor"

const FName FFonixFlowTrackerSetupEditorModule::MainPanelTabId("FonixFlowTrackerSetupMainPanel");

void FFonixFlowTrackerSetupEditorModule::StartupModule()
{
	// 1. Register custom icons
	FFonixFlowTrackerSetupStyle::Initialize();

	// 2. Register the tab that the toolbar button opens
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MainPanelTabId,
		FOnSpawnTab::CreateRaw(this, &FFonixFlowTrackerSetupEditorModule::OnSpawnMainPanelTab))
		.SetDisplayName(LOCTEXT("PanelTab", "FonixFlow Tracker Setup"))
		.SetTooltipText(LOCTEXT("PanelTabTooltip", "FonixFlow Tracker Setup"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FFonixFlowTrackerSetupStyle::GetStyleName(), "FonixFlowTrackerSetup.TabIcon"));

	// 3. Register toolbar button automatically (no Python, no extra steps)
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
		{
			// Toolbar button
			UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(
				"LevelEditor.LevelEditorToolBar.PlayToolBar");

			FToolMenuSection& Section = ToolbarMenu->AddSection(
				"FonixFlowTrackerSetup",
				LOCTEXT("Section", "Tracking"));

			// Main button - click opens the panel
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				"OpenFonixFlowTracker",
				FUIAction(FExecuteAction::CreateRaw(
					this, &FFonixFlowTrackerSetupEditorModule::OnOpenMainPanel)),
				LOCTEXT("Button", "FonixFlow Tracker Setup"),
				LOCTEXT("Tooltip", "Open FonixFlow Tracker Setup"),
				FSlateIcon(FFonixFlowTrackerSetupStyle::GetStyleName(),
					"FonixFlowTrackerSetup.Icon")));

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
								&FFonixFlowTrackerSetupEditorModule::OnSetupFreeDQuick)));

						Sub.AddMenuEntry(
							LOCTEXT("OpenTrack", "OpenTrack Camera"),
							LOCTEXT("OpenTrackTip", "Quick OpenTrack setup"),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(
								this,
								&FFonixFlowTrackerSetupEditorModule::OnSetupOpenTrackQuick)));
					}));

			// Tools menu entry
			UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(
				"LevelEditor.MainMenu.Tools");

			FToolMenuSection& ToolsSection = ToolsMenu->AddSection(
				"FonixFlowTrackerSetup",
				LOCTEXT("ToolsSection", "FonixFlow Tracker Setup"));

			ToolsSection.AddMenuEntry(
				"OpenFonixFlowTrackerTools",
				LOCTEXT("ToolsEntry", "FonixFlow Tracker Setup"),
				LOCTEXT("ToolsEntryTip", "Open FonixFlow Tracker Setup"),
				FSlateIcon(FFonixFlowTrackerSetupStyle::GetStyleName(),
					"FonixFlowTrackerSetup.Icon"),
				FUIAction(FExecuteAction::CreateRaw(
					this, &FFonixFlowTrackerSetupEditorModule::OnOpenMainPanel)));
		}));

	UE_LOG(LogTemp, Log,
		TEXT("FonixFlowTrackerSetup: Plugin loaded — toolbar button registered"));
}

void FFonixFlowTrackerSetupEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MainPanelTabId);
	UToolMenus::Get()->UnregisterOwnerByName(FName("FonixFlowTrackerSetup"));
	FFonixFlowTrackerSetupStyle::Shutdown();
}

TSharedRef<SDockTab> FFonixFlowTrackerSetupEditorModule::OnSpawnMainPanelTab(
	const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("TabLabel", "FonixFlow Tracker Setup"))
		[
			SNew(SFonixFlowTrackerSetupPanel)
		];
}

void FFonixFlowTrackerSetupEditorModule::OnOpenMainPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MainPanelTabId);
}

void FFonixFlowTrackerSetupEditorModule::OnSetupFreeDQuick()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FFonixFlowTrackerResult Result =
		UFonixFlowTrackerSetupSubsystem::SetupFreeDCamera(World);
	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: %s"), *Result.Message);
}

void FFonixFlowTrackerSetupEditorModule::OnSetupOpenTrackQuick()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FFonixFlowTrackerResult Result =
		UFonixFlowTrackerSetupSubsystem::SetupOpenTrackCamera(World);
	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: %s"), *Result.Message);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFonixFlowTrackerSetupEditorModule, FonixFlowTrackerSetupEditor)
