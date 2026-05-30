// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "FonixFlowTrackerSetupTypes.h"
#include "LensSetupTypes.h"

class ACineCameraActor;
class ULensFile;

/**
 * Unified setup panel for FonixFlow Tracker Setup.
 * Single "Setup Now" button automates the entire 3D tracking pipeline.
 */
class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerSetupPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerSetupPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// State
	ETrackingProtocol SelectedProtocol = ETrackingProtocol::FreeD;
	FString LocalIPAddress;
	int32 ListeningPort = 40000;
	FString SubjectName = TEXT("Camera");

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

	// UI Build
	TSharedRef<SWidget> BuildHeader();
	TSharedRef<SWidget> BuildProtocolSection();
	TSharedRef<SWidget> BuildNetworkSection();
	TSharedRef<SWidget> BuildSetupButton();
	TSharedRef<SWidget> BuildCalibrationSection();
	TSharedRef<SWidget> BuildStatusSection();
	TSharedRef<SWidget> BuildLogSection();

	// Actions
	void RunOneClickSetup();
	void CaptureFocusMin();
	void CaptureFocusMax();
	void CaptureZoomMin();
	void CaptureZoomMax();
	void ApplyCalibration();
	void DetectLocalIP();
	void AddLog(const FString& Message);

	// Queries
	FText GetIPAddressText() const;
	FText GetPortText() const;
	FText GetFocusMinText() const;
	FText GetFocusMaxText() const;
	FText GetZoomMinText() const;
	FText GetZoomMaxText() const;
	FText GetSetupStatusText() const;
	EVisibility GetCalibrationVisibility() const;
	bool IsCalibrationReady() const;
	bool IsSetupButtonEnabled() const;
};
