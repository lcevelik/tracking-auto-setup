// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "TrackingSetupTypes.h"

class ACineCameraActor;
class ULensFile;

/** Wizard step enum */
enum class EWizardStep : uint8
{
	Protocol,           // Step 1: Choose FreeD or OpenTrack
	Network,            // Step 2: IP/Port configuration
	Camera,             // Step 3: Pick existing or create new camera
	LensCalibration,    // Step 4: Lens encoder range calibration
	AnchorPoint,        // Step 5: Anchor point configuration
	Review,             // Step 6: Review and apply
	Complete            // Step 7: Done
};

/**
 * Multi-step wizard for tracking setup.
 * Guides user through protocol, network, camera, lens calibration, and anchor setup.
 */
class TRACKINGAUTOSETUPEDITOR_API STrackingSetupWizard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STrackingSetupWizard) {}
		/** Called when wizard completes with settings */
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
	FTrackingSetupResult SetupResult;

	/** Step navigation */
	void GoToNextStep();
	void GoToPreviousStep();
	bool CanGoNext() const;
	bool CanGoPrevious() const;

	/** Build content for each step */
	TSharedRef<SWidget> BuildProtocolStep();
	TSharedRef<SWidget> BuildNetworkStep();
	TSharedRef<SWidget> BuildCameraStep();
	TSharedRef<SWidget> BuildLensCalibrationStep();
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

	/** Lens calibration state */
	bool bIsCapturingMinRange = false;
	bool bIsCapturingMaxRange = false;
	int32 CurrentFocusEncoderMin = 0;
	int32 CurrentFocusEncoderMax = 0x00FFFFFF;
	int32 CurrentZoomEncoderMin = 0;
	int32 CurrentZoomEncoderMax = 0x00FFFFFF;

	/** Capture encoder values for lens calibration */
	void OnCaptureFocusMin();
	void OnCaptureFocusMax();
	void OnCaptureZoomMin();
	void OnCaptureZoomMax();

	/** Apply the tracking setup */
	void ApplySetup();

	/** Delegate for setup completion */
	FSimpleDelegate OnSetupComplete;

	/** Step indicator widgets */
	TArray<TSharedPtr<SWidget>> StepIndicators;
	void UpdateStepIndicators();
};
