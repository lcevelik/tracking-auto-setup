// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FonixFlowTrackerSetupTypes.h"
#include "FonixFlowTrackerSetupSubsystem.generated.h"

class ULensFile;
class ACineCameraActor;
class AActor;

/**
 * Subsystem that automates Live Link camera tracking setup.
 * Handles FreeD and OpenTrack IO protocol configuration in one click.
 */
UCLASS()
class FONIXFLOWTRACKERSETUP_API UFonixFlowTrackerSetupSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Perform complete tracking setup with the given settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "FonixFlow Tracker Setup", meta = (WorldContext = "WorldContextObject"))
	static FFonixFlowTrackerResult SetupTracking(
		UObject* WorldContextObject,
		const FTrackingConnectionSettings& ConnectionSettings,
		const FCameraSetupConfig& CameraConfig
	);

	/**
	 * Quick setup with defaults for FreeD protocol.
	 */
	UFUNCTION(BlueprintCallable, Category = "FonixFlow Tracker Setup", meta = (WorldContext = "WorldContextObject"))
	static FFonixFlowTrackerResult SetupFreeDCamera(
		UObject* WorldContextObject,
		const FString& IPAddress = TEXT("127.0.0.1"),
		int32 Port = 40000,
		const FString& CameraName = TEXT("TrackedCamera")
	);

	/**
	 * Quick setup with defaults for OpenTrack IO protocol.
	 */
	UFUNCTION(BlueprintCallable, Category = "FonixFlow Tracker Setup", meta = (WorldContext = "WorldContextObject"))
	static FFonixFlowTrackerResult SetupOpenTrackCamera(
		UObject* WorldContextObject,
		int32 SourceNumber = 1,
		const FString& CameraName = TEXT("TrackedCamera")
	);

	/**
	 * Remove a previously configured tracking setup.
	 */
	UFUNCTION(BlueprintCallable, Category = "FonixFlow Tracker Setup", meta = (WorldContext = "WorldContextObject"))
	static bool RemoveFonixFlowTracker(
		UObject* WorldContextObject,
		FGuid LiveLinkSourceGuid,
		AActor* CameraActor = nullptr
	);

private:
	/** Create or find the Live Link source for the given protocol */
	FGuid CreateLiveLinkSource(const FTrackingConnectionSettings& Settings);

	/** Create and configure a CineCameraActor with tracking */
	ACineCameraActor* CreateTrackedCamera(UWorld* World, const FCameraSetupConfig& Config);

	/** Create the anchor point actor */
	AActor* CreateAnchorPoint(UWorld* World, const FCameraSetupConfig& Config);

	/** Configure the Live Link component on the camera */
	void ConfigureLiveLinkComponent(ACineCameraActor* Camera, const FGuid& SourceGuid, const FString& SubjectName);

	/** Wire up virtual camera if enabled */
	void ConfigureVirtualCamera(ACineCameraActor* Camera, const FCameraSetupConfig& Config);
};
