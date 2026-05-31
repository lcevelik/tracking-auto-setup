// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "FonixFlowTrackerSetupTypes.h"
#include "LensSetupTypes.h"

class ACineCameraActor;
class ULensFile;
class SScrollBox;

/**
 * Unified setup panel for FonixFlow Tracker Setup.
 * User selects a camera, then clicks "Setup Now" to configure everything.
 */
class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerSetupPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerSetupPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SFonixFlowTrackerSetupPanel();

private:
	// State
	ETrackingProtocol SelectedProtocol = ETrackingProtocol::FreeD;
	FString LocalIPAddress;
	int32 ListeningPort = 40000;
	FString SubjectName = TEXT("Camera");

	// Camera selection
	TArray<ACineCameraActor*> AvailableCameras;
	TArray<TWeakObjectPtr<ACineCameraActor>> CameraWeakPtrs;
	TSharedPtr<SListView<TWeakObjectPtr<ACineCameraActor>>> CameraListView;
	ACineCameraActor* SelectedCamera = nullptr;

	// Lens type
	bool bUsePrimeLens = false;
	float PrimeLensFocalLengthMM = 50.0f;

	// LiveLink polling — actual encoder values (0-1 normalized from FreeD)
	float LiveLinkFocusValue = 0.0f;
	float LiveLinkZoomValue = 0.0f;
	bool bLiveLinkPollingActive = false;
	FTimerHandle LiveLinkPollTimerHandle;

	// Calibration — captured raw encoder values
	int32 FocusEncoderMin = 0;
	int32 FocusEncoderMax = 0x00FFFFFF;
	int32 ZoomEncoderMin = 0;
	int32 ZoomEncoderMax = 0x00FFFFFF;
	bool bFocusMinCaptured = false;
	bool bFocusMaxCaptured = false;
	bool bZoomMinCaptured = false;
	bool bZoomMaxCaptured = false;

	float FocusDistanceMinCM = 60.0f;
	float FocusDistanceMaxCM = 4095.0f;
	float FocalLengthMinMM = 28.0f;
	float FocalLengthMaxMM = 100.0f;

	bool bSetupRunning = false;
	bool bSetupComplete = false;
	bool bSetupSuccess = false;
	TArray<FString> SetupLog;

	// Source GUID for cleanup
	FGuid ActiveSourceGuid;

	// Lens type SBox references for visibility control
	TSharedPtr<SBox> PrimeLensInputBox;
	TSharedPtr<SBox> ZoomLensInputBox;
	TSharedPtr<SBox> ZoomCalibrationBox;

	// Log scroll box for auto-scroll to bottom
	TSharedPtr<SScrollBox> LogScrollBox;

	// UI Build
	TSharedRef<SWidget> BuildHeader();
	TSharedRef<SWidget> BuildProtocolSection();
	TSharedRef<SWidget> BuildNetworkSection();
	TSharedRef<SWidget> BuildCameraSection();
	TSharedRef<SWidget> BuildLensTypeSection();
	TSharedRef<SWidget> BuildSetupButton();
	TSharedRef<SWidget> BuildCalibrationSection();
	TSharedRef<SWidget> BuildStatusSection();
	TSharedRef<SWidget> BuildLogSection();

	// Actions
	void RefreshCameraList();
	void RunOneClickSetup();
	void CaptureFocusMin();
	void CaptureFocusMax();
	void CaptureZoomMin();
	void CaptureZoomMax();
	void ApplyCalibration();
	void DetectLocalIP();
	void AddLog(const FString& Message);
	void PollLiveLinkData();
	void StopLiveLinkPolling();
	void UpdateLensTypeVisibility();

	// Camera list
	TSharedRef<ITableRow> OnGenerateCameraRow(TWeakObjectPtr<ACineCameraActor> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnCameraSelected(TWeakObjectPtr<ACineCameraActor> InItem, ESelectInfo::Type SelectInfo);

	// Queries
	FText GetIPAddressText() const;
	FText GetFocusMinText() const;
	FText GetFocusMaxText() const;
	FText GetZoomMinText() const;
	FText GetZoomMaxText() const;
	FText GetSetupStatusText() const;
	FText GetSelectedCameraText() const;
	FText GetLiveLinkFocusText() const;
	FText GetLiveLinkZoomText() const;
	EVisibility GetCalibrationVisibility() const;
	bool IsCalibrationReady() const;
	bool IsSetupButtonEnabled() const;
};
