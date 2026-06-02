// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SFonixFlowTrackerAIChatPanel.h"
#include "FonixFlowTrackerSetupTypes.h"
#include "FonixFlowTrackerActions.h"
#include "LensSetupTypes.h"

class ACineCameraActor;
class ULensFile;
class SScrollBox;

/**
 * Unified setup panel for FonixFlow Tracker Setup.
 * User selects a camera, then clicks "Setup Now" to configure everything.
 */
class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerSetupPanel : public SCompoundWidget, public IFonixFlowTrackerActions
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerSetupPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SFonixFlowTrackerSetupPanel();

	// IFonixFlowTrackerActions interface
	virtual FFonixFlowTrackerState GetState() const override;
	virtual bool SelectCamera(const FString& CameraName) override;
	virtual TArray<FString> GetAvailableCameraNames() const override;
	virtual bool SetProtocol(const FString& Protocol) override;
	virtual bool SetLensType(const FString& LensType) override;
	virtual bool SetPrimeFocalLength(float MM) override;
	virtual bool SetZoomRange(float MinMM, float MaxMM) override;
	virtual FString RunSetup() override;
	virtual FString CaptureCalibration(const FString& Target) override;
	virtual FString ApplyCalibrationAction() override;

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
	float FocalLengthMinMM = 28.0f;
	float FocalLengthMaxMM = 100.0f;

	// LiveLink polling — actual values from FreeD (physical cm/mm when source is calibrated)
	float LiveLinkFocusValue = 0.0f;
	float LiveLinkZoomValue = 0.0f;    // live focal length reconstructed from FreeD zoom encoder
	bool bLiveLinkPollingActive = false;
	FTimerHandle LiveLinkPollTimerHandle;

	// Calibration — captured physical values direct from LiveLink
	float FocusEncoderMin = 0.0f;
	float FocusEncoderMax = 0.0f;
	bool bFocusMinCaptured = false;
	bool bFocusMaxCaptured = false;

	float ZoomEncoderWide = 0.0f;    // focal length (mm) captured at wide end
	float ZoomEncoderTele = 0.0f;    // focal length (mm) captured at tele end
	bool bZoomWideCaptured = false;
	bool bZoomTeleCaptured = false;

	bool bSetupRunning = false;
	bool bSetupComplete = false;
	bool bSetupSuccess = false;
	bool bCalibrationApplied = false;

	// Source GUID for cleanup
	FGuid ActiveSourceGuid;

	// Whether the LiveLink subject has been auto-assigned to the controller
	bool bSubjectAutoAssigned = false;

	// Lens type SBox references for visibility control
	TSharedPtr<SBox> PrimeLensInputBox;
	TSharedPtr<SBox> ZoomLensInputBox;
	TSharedPtr<SBox> ZoomCalibBox;

	// Calibration: "Apply Lens File" button box (shown after Apply Calibration)
	TSharedPtr<SBox> ApplyLensFileBox;

	// Tab state: 0 = Camera Setup, 1 = Calibration
	int32 ActiveTab = 0;
	TSharedPtr<SWidgetSwitcher> TabSwitcher;

	// UI Build
	TSharedRef<SWidget> BuildHeader();
	TSharedRef<SWidget> BuildTabBar();
	TSharedRef<SWidget> BuildCameraSetupTab();
	TSharedRef<SWidget> BuildCalibrationTab();
	TSharedRef<SWidget> BuildProtocolSection();
	TSharedRef<SWidget> BuildNetworkSection();
	TSharedRef<SWidget> BuildCameraSection();
	TSharedRef<SWidget> BuildLensTypeSection();
	TSharedRef<SWidget> BuildSetupButton();
	TSharedRef<SWidget> BuildCalibrationSection();
	TSharedRef<SWidget> BuildStatusSection();
	TSharedRef<SWidget> BuildLogSection();

	void SwitchTab(int32 TabIndex);

	// Actions
	void RefreshCameraList();
	void RunOneClickSetup();
	void CaptureFocusMin();
	void CaptureFocusMax();
	void CaptureZoomWide();
	void CaptureZoomTele();
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
	FText GetZoomWideText() const;
	FText GetZoomTeleText() const;
	FText GetSetupStatusText() const;
	FText GetSelectedCameraText() const;
	FText GetLiveLinkFocusText() const;
	FText GetLiveLinkZoomText() const;
	EVisibility GetCalibrationVisibility() const;
	bool IsCalibrationReady() const;
	bool IsSetupButtonEnabled() const;
};
