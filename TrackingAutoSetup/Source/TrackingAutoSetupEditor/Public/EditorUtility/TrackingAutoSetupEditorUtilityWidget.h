// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "TrackingAutoSetupEditorUtilityWidget.generated.h"

class STrackingSetupWizard;
class STrackingAIChatPanel;

/**
 * Editor Utility Widget for Tracking Auto Setup.
 * This is the C++ base class that hosts the Slate widgets.
 * Users can create Blueprint subclasses to customize the UI.
 *
 * In C++, use SpawnAndRegisterTab() to open this widget.
 * In Python, use EditorUtilitySubsystem.spawn_and_register_tab().
 */
UCLASS(BlueprintType, Blueprintable)
class TRACKINGAUTOSETUPEDITOR_API UTrackingAutoSetupEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UTrackingAutoSetupEditorUtilityWidget();

	//~ Begin UEditorUtilityWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End UEditorUtilityWidget interface

	/** Get the wizard widget */
	UFUNCTION(BlueprintPure, Category = "Tracking Auto Setup")
	UUserWidget* GetWizardWidget() const { return nullptr; } // Blueprint accessor

	/** Get the AI chat widget */
	UFUNCTION(BlueprintPure, Category = "Tracking Auto Setup")
	UUserWidget* GetAIChatWidget() const { return nullptr; } // Blueprint accessor

	/** Open the wizard tab */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	void OpenWizard();

	/** Open the AI chat tab */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	void OpenAIChat();

	/** Run quick FreeD setup */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	void QuickSetupFreeD(const FString& IPAddress = TEXT("127.0.0.1"), int32 Port = 40000);

	/** Run quick OpenTrack setup */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	void QuickSetupOpenTrack(int32 SourceNumber = 1);

	/** The Slate widget container */
	TSharedPtr<class STrackingAutoSetupPanel> SlatePanel;

protected:
	/** Override to create custom Slate content */
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	/** Tab IDs */
	static const FName MainPanelTabId;
	static const FName WizardTabId;
	static const FName AIChatTabId;
};
