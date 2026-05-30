// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LensSetupTypes.h"
#include "TrackingSetupTypes.generated.h"

/** Supported tracking protocols */
UENUM(BlueprintType)
enum class ETrackingProtocol : uint8
{
	FreeD			UMETA(DisplayName = "FreeD"),
	OpenTrackIO		UMETA(DisplayName = "OpenTrack IO"),
	Custom			UMETA(DisplayName = "Custom (Manual)")
};

/** Predefined camera rig presets */
UENUM(BlueprintType)
enum class ECameraRigPreset : uint8
{
	Generic,
	Panasonic,
	Sony,
	Stype			UMETA(DisplayName = "stYpe"),
	Mosys,
	Ncam,
	Custom
};

/** Connection settings for tracking source */
USTRUCT(BlueprintType)
struct TRACKINGAUTOSETUP_API FTrackingConnectionSettings
{
	GENERATED_BODY()

	/** Tracking protocol to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protocol")
	ETrackingProtocol Protocol = ETrackingProtocol::FreeD;

	/** Camera rig preset (manufacturer-specific defaults) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protocol")
	ECameraRigPreset RigPreset = ECameraRigPreset::Generic;

	/** IP address of the tracking source (FreeD) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (EditCondition = "Protocol == ETrackingProtocol::FreeD"))
	FString IPAddress = TEXT("127.0.0.1");

	/** UDP port for FreeD protocol */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (EditCondition = "Protocol == ETrackingProtocol::FreeD"))
	int32 FreeDPort = 40000;

	/** OpenTrack source number (1-200, used for multicast address) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (EditCondition = "Protocol == ETrackingProtocol::OpenTrackIO", ClampMin = "1", ClampMax = "200"))
	int32 OpenTrackSourceNumber = 1;

	/** Use multicast for OpenTrack (vs unicast) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (EditCondition = "Protocol == ETrackingProtocol::OpenTrackIO"))
	bool bOpenTrackUseMulticast = true;

	/** Unicast port for OpenTrack (if not multicast) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (EditCondition = "Protocol == ETrackingProtocol::OpenTrackIO && !bOpenTrackUseMulticast"))
	int32 OpenTrackUnicastPort = 0;

	/** Subject name for the Live Link source */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live Link")
	FString SubjectName = TEXT("Camera");

	/** Enable to auto-detect FreeD packets on the network */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (EditCondition = "Protocol == ETrackingProtocol::FreeD"))
	bool bAutoDetectPort = false;
};

/** Camera setup configuration */
USTRUCT(BlueprintType)
struct TRACKINGAUTOSETUP_API FCameraSetupConfig
{
	GENERATED_BODY()

	/** Name for the camera actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FString CameraName = TEXT("TrackedCamera");

	/** Whether to create a new CineCameraActor or configure existing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bCreateNewCamera = true;

	/** Existing camera actor to configure (if bCreateNewCamera is false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "!bCreateNewCamera"))
	AActor* ExistingCamera = nullptr;

	/** Create and attach a tracking anchor/origin point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bCreateAnchorPoint = true;

	/** Anchor point world location (if not using origin) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "bCreateAnchorPoint"))
	FVector AnchorLocation = FVector::ZeroVector;

	/** Anchor point world rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "bCreateAnchorPoint"))
	FRotator AnchorRotation = FRotator::ZeroRotator;

	/** Lens configuration (encoder ranges, sensor, calibration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens")
	FLensConfiguration LensConfig;

	/** Enable virtual camera integration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Virtual Camera")
	bool bEnableVirtualCamera = false;
};

/** Result of a tracking setup operation */
USTRUCT(BlueprintType)
struct TRACKINGAUTOSETUP_API FTrackingSetupResult
{
	GENERATED_BODY()

	/** Whether the setup completed successfully */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	/** Status message */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString Message;

	/** Created/modified camera actor */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	AActor* CameraActor = nullptr;

	/** Created Live Link source GUID */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FGuid LiveLinkSourceGuid;

	/** Created anchor point actor */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	AActor* AnchorPoint = nullptr;

	/** Configured lens file */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	ULensFile* LensFile = nullptr;

	/** Warnings encountered during setup */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FString> Warnings;

	/** Errors encountered during setup */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FString> Errors;
};
