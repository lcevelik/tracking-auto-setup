// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "FonixFlowTrackerSetupTypes.h"
#include "LensSetupTypes.h"

class ACineCameraActor;
class ULensFile;

/** Wizard step enum */
enum class EWizardStep : uint8
{
	Protocol,           // Step 1: Choose FreeD or OpenTrack
	Network,            // Step 2: IP/Port configuration
	Camera,             // Step 3: Pick existing or create new camera
	LensSelection,      // Step 4: Select or create lens file
	LensCalibration,    // Step 5: Lens encoder range calibration
	SensorSettings,     // Step 6: Sensor/filmback configuration
	AnchorPoint,        // Step 7: Anchor point configuration
	Review,             // Step 8: Review and apply
	Complete            // Step 9: Done
};

/**
 * Multi-step wizard for tracking setup.
 */
class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerSetupWizard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerSetupWizard) {}
		SLATE_EVENT(FSimpleDelegate, OnSetupComplete)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** Current wizard step */
	EWizardStep CurrentStep = EWizardStep::Protocol;

	/** Connection settings being configured */
	FTrackingConnectionSettings ConnectionSettings;

	/** Camera setup config being configured */
	FCameraSetupConfig CameraConfig;

	/** Setup result after completion */
	FFonixFlowTrackerResult SetupResult;

	/** Step navigation */
	void GoToNextStep();
	void GoToPreviousStep();
	bool CanGoNext() const;
	bool CanGoPrevious() const;

	/** Build content for each step */
	TSharedRef<SWidget> BuildProtocolStep();
	TSharedRef<SWidget> BuildNetworkStep();
	TSharedRef<SWidget> BuildCameraStep();
	TSharedRef<SWidget> BuildLensSelectionStep();
	TSharedRef<SWidget> BuildLensCalibrationStep();
	TSharedRef<SWidget> BuildSensorSettingsStep();
	TSharedRef<SWidget> BuildAnchorPointStep();
	TSharedRef<SWidget> BuildReviewStep();
	TSharedRef<SWidget> BuildCompleteStep();

	/** Get the current step content */
	TSharedRef<SWidget> GetCurrentStepContent();

	/** Get step title */
	FText GetStepTitle() const;

	/** Get step description */
	FText GetStepDescription() const;

	/** Get step number text */
	FText GetStepNumberText() const;

	/** Camera picker helpers */
	void RefreshCameraList();
	TArray<ACineCameraActor*> AvailableCameras;
	TSharedPtr<SListView<TWeakObjectPtr<ACineCameraActor>>> CameraListView;

	/** Lens file picker helpers */
	void RefreshLensFileList();
	TArray<ULensFile*> AvailableLensFiles;
	TSharedPtr<SListView<TWeakObjectPtr<ULensFile>>> LensFileListView;

	/** Sensor presets */
	TArray<FVector2D> SensorPresets;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> SensorPresetCombo;

	/** Apply the tracking setup */
	void ApplySetup();

	/** Delegate for setup completion */
	FSimpleDelegate OnSetupComplete;
};
