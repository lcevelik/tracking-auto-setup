// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FonixFlowTrackerActions.generated.h"

class ACineCameraActor;

/**
 * Serializable snapshot of the current plugin state.
 * Injected into AI system prompt as JSON context.
 */
USTRUCT(BlueprintType)
struct FONIXFLOWTRACKERSETUP_API FFonixFlowTrackerState
{
	GENERATED_BODY()

	// Camera
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	FString SelectedCamera;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	bool bHasCamera = false;

	// Protocol
	UPROPERTY(BlueprintReadOnly, Category = "Protocol")
	FString Protocol; // "FreeD" or "OpenTrackIO"

	UPROPERTY(BlueprintReadOnly, Category = "Protocol")
	FString IPAddress;

	UPROPERTY(BlueprintReadOnly, Category = "Protocol")
	int32 Port = 40000;

	// Lens
	UPROPERTY(BlueprintReadOnly, Category = "Lens")
	FString LensType; // "Prime" or "Zoom"

	UPROPERTY(BlueprintReadOnly, Category = "Lens")
	float PrimeFocalLengthMM = 50.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lens")
	float ZoomMinMM = 28.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lens")
	float ZoomMaxMM = 100.0f;

	// Setup status
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bSetupComplete = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bLiveLinkActive = false;

	// Calibration
	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	bool bCalibrationApplied = false;

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	bool bNearCaptured = false;

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	bool bFarCaptured = false;

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	bool bWideCaptured = false;

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	bool bTeleCaptured = false;

	// Live values
	UPROPERTY(BlueprintReadOnly, Category = "Live")
	float LiveFocusCM = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Live")
	float LiveZoomMM = 0.0f;

	/** Serialize to JSON string for injection into AI prompt */
	FString ToJSON() const;
};

/**
 * Interface for plugin actions that the AI chat can invoke.
 * Implemented by SFonixFlowTrackerSetupPanel.
 */
class FONIXFLOWTRACKERSETUP_API IFonixFlowTrackerActions
{
public:
	virtual ~IFonixFlowTrackerActions() = default;

	/** Get current state snapshot */
	virtual FFonixFlowTrackerState GetState() const = 0;

	/** Select a camera by actor name. Returns true if found. */
	virtual bool SelectCamera(const FString& CameraName) = 0;

	/** Get list of available camera names */
	virtual TArray<FString> GetAvailableCameraNames() const = 0;

	/** Set tracking protocol ("FreeD" or "OpenTrackIO") */
	virtual bool SetProtocol(const FString& Protocol) = 0;

	/** Set lens type ("Prime" or "Zoom") */
	virtual bool SetLensType(const FString& LensType) = 0;

	/** Set prime lens focal length */
	virtual bool SetPrimeFocalLength(float MM) = 0;

	/** Set zoom range */
	virtual bool SetZoomRange(float MinMM, float MaxMM) = 0;

	/** Run one-click setup. Returns result message. */
	virtual FString RunSetup() = 0;

	/** Capture calibration target ("Near", "Far", "Wide", "Tele"). Returns result. */
	virtual FString CaptureCalibration(const FString& Target) = 0;

	/** Apply calibration. Returns result message. */
	virtual FString ApplyCalibrationAction() = 0;
};
