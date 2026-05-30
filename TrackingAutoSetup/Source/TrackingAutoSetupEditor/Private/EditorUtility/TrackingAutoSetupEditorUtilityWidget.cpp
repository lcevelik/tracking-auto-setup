// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "EditorUtility/TrackingAutoSetupEditorUtilityWidget.h"
#include "Widgets/STrackingAutoSetupPanel.h"
#include "TrackingAutoSetupSubsystem.h"
#include "TrackingSetupTypes.h"
#include "Widgets/SBoxPanel.h"
#include "Engine/World.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "TrackingAutoSetupEditorUtilityWidget"

const FName UTrackingAutoSetupEditorUtilityWidget::MainPanelTabId("TrackingAutoSetupMainPanel");
const FName UTrackingAutoSetupEditorUtilityWidget::WizardTabId("TrackingAutoSetupWizard");
const FName UTrackingAutoSetupEditorUtilityWidget::AIChatTabId("TrackingAutoSetupAIChat");

UTrackingAutoSetupEditorUtilityWidget::UTrackingAutoSetupEditorUtilityWidget()
{
	// Set the widget to be interactive (not just display)
	bIsVariable = true;
}

void UTrackingAutoSetupEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup EUW: Constructed"));
}

void UTrackingAutoSetupEditorUtilityWidget::NativeDestruct()
{
	Super::NativeDestruct();
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup EUW: Destructed"));
}

TSharedRef<SWidget> UTrackingAutoSetupEditorUtilityWidget::RebuildWidget()
{
	// Create the main panel Slate widget
	SAssignNew(SlatePanel, STrackingAutoSetupPanel);

	// Wrap in a UMG-compatible container
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.FillHeight(1.0f)
	[
		SlatePanel.ToSharedRef()
	];
}

void UTrackingAutoSetupEditorUtilityWidget::OpenWizard()
{
	FGlobalTabmanager::Get()->TryInvokeTab(WizardTabId);
}

void UTrackingAutoSetupEditorUtilityWidget::OpenAIChat()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AIChatTabId);
}

void UTrackingAutoSetupEditorUtilityWidget::QuickSetupFreeD(const FString& IPAddress, int32 Port)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupFreeDCamera(World, IPAddress, Port);
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Quick FreeD: %s"), *Result.Message);
}

void UTrackingAutoSetupEditorUtilityWidget::QuickSetupOpenTrack(int32 SourceNumber)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupOpenTrackCamera(World, SourceNumber);
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup Quick OpenTrack: %s"), *Result.Message);
}

#undef LOCTEXT_NAMESPACE
