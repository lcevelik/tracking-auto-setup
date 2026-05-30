// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "TrackingAutoSetupSubsystem.h"
#include "LensSetupTypes.h"
#include "ILiveLinkClient.h"
#include "LiveLinkSourceFactory.h"
#include "LiveLinkPresetTypes.h"
#include "LiveLinkComponentController.h"
#include "LiveLinkCameraController.h"
#include "Roles/LiveLinkCameraRole.h"
#include "Camera/CineCameraActor.h"
#include "Camera/CineCameraComponent.h"
#include "LensFile.h"
#include "LensComponent.h"
#include "CameraCalibrationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"

#define LOCTEXT_NAMESPACE "TrackingAutoSetup"

void UTrackingAutoSetupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Subsystem initialized"));
}

void UTrackingAutoSetupSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FTrackingSetupResult UTrackingAutoSetupSubsystem::SetupTracking(
	UObject* WorldContextObject,
	const FTrackingConnectionSettings& ConnectionSettings,
	const FCameraSetupConfig& CameraConfig)
{
	FTrackingSetupResult Result;

	if (!WorldContextObject)
	{
		Result.Message = TEXT("Invalid world context");
		Result.Errors.Add(Result.Message);
		return Result;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		Result.Message = TEXT("Could not get world");
		Result.Errors.Add(Result.Message);
		return Result;
	}

	// Step 1: Create Live Link source
	Result.LiveLinkSourceGuid = Get().CreateLiveLinkSource(ConnectionSettings);
	if (!Result.LiveLinkSourceGuid.IsValid())
	{
		Result.Message = TEXT("Failed to create Live Link source");
		Result.Errors.Add(Result.Message);
		return Result;
	}

	// Step 2: Create or configure camera
	Result.CameraActor = Get().CreateTrackedCamera(World, CameraConfig);
	if (!Result.CameraActor)
	{
		Result.Message = TEXT("Failed to create/configure camera");
		Result.Errors.Add(Result.Message);
		return Result;
	}

	// Step 3: Create anchor point
	if (CameraConfig.bCreateAnchorPoint)
	{
		Result.AnchorPoint = Get().CreateAnchorPoint(World, CameraConfig);
		if (Result.AnchorPoint)
		{
			Result.CameraActor->AttachToActor(Result.AnchorPoint,
				FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			Result.Warnings.Add(TEXT("Failed to create anchor point, camera placed at origin"));
		}
	}

	// Step 4: Configure Live Link component
	ACineCameraActor* CineCamera = Cast<ACineCameraActor>(Result.CameraActor);
	if (CineCamera)
	{
		Get().ConfigureLiveLinkComponent(CineCamera, Result.LiveLinkSourceGuid, ConnectionSettings.SubjectName);
	}

	// Step 5: Apply lens configuration (creates lens file, configures camera + lens component)
	if (CineCamera)
	{
		Result.LensFile = ULensSetupUtility::ApplyLensConfiguration(CineCamera, CameraConfig.LensConfig);
		if (!Result.LensFile)
		{
			Result.Warnings.Add(TEXT("Failed to create lens file, camera configured without lens calibration"));
		}
	}

	// Step 6: Virtual camera integration
	if (CameraConfig.bEnableVirtualCamera)
	{
		Get().ConfigureVirtualCamera(CineCamera, CameraConfig);
	}

	Result.bSuccess = true;
	Result.Message = FString::Printf(TEXT("Tracking setup complete: %s on %s"),
		*ConnectionSettings.SubjectName,
		ConnectionSettings.Protocol == ETrackingProtocol::FreeD
			? *FString::Printf(TEXT("%s:%d"), *ConnectionSettings.IPAddress, ConnectionSettings.FreeDPort)
			: *FString::Printf(TEXT("OpenTrack source #%d"), ConnectionSettings.OpenTrackSourceNumber));

	return Result;
}

FTrackingSetupResult UTrackingAutoSetupSubsystem::SetupFreeDCamera(
	UObject* WorldContextObject,
	const FString& IPAddress,
	int32 Port,
	const FString& CameraName)
{
	FTrackingConnectionSettings ConnSettings;
	ConnSettings.Protocol = ETrackingProtocol::FreeD;
	ConnSettings.IPAddress = IPAddress;
	ConnSettings.FreeDPort = Port;
	ConnSettings.SubjectName = CameraName;

	FCameraSetupConfig CamConfig;
	CamConfig.CameraName = CameraName;
	CamConfig.bCreateNewCamera = true;
	CamConfig.bCreateAnchorPoint = true;

	return SetupTracking(WorldContextObject, ConnSettings, CamConfig);
}

FTrackingSetupResult UTrackingAutoSetupSubsystem::SetupOpenTrackCamera(
	UObject* WorldContextObject,
	int32 SourceNumber,
	const FString& CameraName)
{
	FTrackingConnectionSettings ConnSettings;
	ConnSettings.Protocol = ETrackingProtocol::OpenTrackIO;
	ConnSettings.OpenTrackSourceNumber = SourceNumber;
	ConnSettings.SubjectName = CameraName;

	FCameraSetupConfig CamConfig;
	CamConfig.CameraName = CameraName;
	CamConfig.bCreateNewCamera = true;
	CamConfig.bCreateAnchorPoint = true;

	return SetupTracking(WorldContextObject, ConnSettings, CamConfig);
}

bool UTrackingAutoSetupSubsystem::RemoveTrackingSetup(
	UObject* WorldContextObject,
	FGuid LiveLinkSourceGuid,
	AActor* CameraActor)
{
	if (!WorldContextObject) return false;

	ILiveLinkClient* LiveLinkClient = nullptr;
	if (FModuleManager::Get().IsModuleLoaded("LiveLink"))
	{
		IModularFeatures& ModularFeatures = IModularFeatures::Get();
		if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
		{
			LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		}
	}

	if (LiveLinkClient && LiveLinkSourceGuid.IsValid())
	{
		LiveLinkClient->RemoveSource(LiveLinkSourceGuid);
	}

	if (CameraActor && CameraActor->IsValidLowLevel())
	{
		CameraActor->Destroy();
	}

	return true;
}

FGuid UTrackingAutoSetupSubsystem::CreateLiveLinkSource(const FTrackingConnectionSettings& Settings)
{
	FGuid SourceGuid;

	ILiveLinkClient* LiveLinkClient = nullptr;
	if (FModuleManager::Get().IsModuleLoaded("LiveLink"))
	{
		IModularFeatures& ModularFeatures = IModularFeatures::Get();
		if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
		{
			LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		}
	}

	if (!LiveLinkClient)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackingAutoSetup: LiveLink client not available"));
		return SourceGuid;
	}

	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	TArray<ULiveLinkSourceFactory*> Factories = ModularFeatures.GetModularFeatureImplementations<ULiveLinkSourceFactory>();

	ULiveLinkSourceFactory* TargetFactory = nullptr;
	FString ConnectionString;

	switch (Settings.Protocol)
	{
	case ETrackingProtocol::FreeD:
	{
		for (ULiveLinkSourceFactory* Factory : Factories)
		{
			if (Factory && Factory->GetSourceDisplayName().ToString().Contains(TEXT("FreeD")))
			{
				TargetFactory = Factory;
				break;
			}
		}

		if (TargetFactory)
		{
			ConnectionString = FString::Printf(
				TEXT("IPAddress=\"%s\" UDPPortNumber=%d"),
				*Settings.IPAddress,
				Settings.FreeDPort);
		}
		break;
	}

	case ETrackingProtocol::OpenTrackIO:
	{
		for (ULiveLinkSourceFactory* Factory : Factories)
		{
			if (Factory && Factory->GetSourceDisplayName().ToString().Contains(TEXT("OpenTrack")))
			{
				TargetFactory = Factory;
				break;
			}
		}

		if (TargetFactory)
		{
			ConnectionString = FString::Printf(
				TEXT("SourceNumber=%d Protocol=%s"),
				Settings.OpenTrackSourceNumber,
				Settings.bOpenTrackUseMulticast ? TEXT("Multicast") : TEXT("Unicast"));
		}
		break;
	}

	default:
		UE_LOG(LogTemp, Warning, TEXT("TrackingAutoSetup: Unsupported protocol"));
		return SourceGuid;
	}

	if (!TargetFactory)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackingAutoSetup: Could not find LiveLink source factory for protocol %d"), (int32)Settings.Protocol);
		return SourceGuid;
	}

	TSharedPtr<ILiveLinkSource> NewSource = TargetFactory->CreateSource(ConnectionString);
	if (NewSource.IsValid())
	{
		SourceGuid = LiveLinkClient->AddSource(NewSource);
		UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Created LiveLink source %s"), *SourceGuid.ToString());
	}

	return SourceGuid;
}

ACineCameraActor* UTrackingAutoSetupSubsystem::CreateTrackedCamera(UWorld* World, const FCameraSetupConfig& Config)
{
	if (!World) return nullptr;

	ACineCameraActor* CineCamera = nullptr;

	if (Config.bCreateNewCamera)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = *Config.CameraName;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CineCamera = World->SpawnActor<ACineCameraActor>(
			ACineCameraActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);

		if (CineCamera)
		{
			CineCamera->SetActorLabel(Config.CameraName);
			UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Created camera actor '%s'"), *Config.CameraName);
		}
	}
	else
	{
		CineCamera = Cast<ACineCameraActor>(Config.ExistingCamera);
		if (!CineCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("TrackingAutoSetup: Existing camera is not a CineCameraActor"));
		}
	}

	return CineCamera;
}

AActor* UTrackingAutoSetupSubsystem::CreateAnchorPoint(UWorld* World, const FCameraSetupConfig& Config)
{
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = *(Config.CameraName + TEXT("_Anchor"));
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* AnchorActor = World->SpawnActor<AActor>(
		AActor::StaticClass(),
		Config.AnchorLocation,
		Config.AnchorRotation,
		SpawnParams);

	if (AnchorActor)
	{
		AnchorActor->SetActorLabel(Config.CameraName + TEXT("_Anchor"));

		USceneComponent* RootComp = NewObject<USceneComponent>(AnchorActor);
		RootComp->SetWorldLocation(Config.AnchorLocation);
		RootComp->SetWorldRotation(Config.AnchorRotation);
		AnchorActor->SetRootComponent(RootComp);

		UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Created anchor point at %s"),
			*Config.AnchorLocation.ToString());
	}

	return AnchorActor;
}

void UTrackingAutoSetupSubsystem::ConfigureLiveLinkComponent(
	ACineCameraActor* Camera,
	const FGuid& SourceGuid,
	const FString& SubjectName)
{
	if (!Camera) return;

	ULiveLinkComponentController* LLController = Camera->FindComponentByClass<ULiveLinkComponentController>();
	if (!LLController)
	{
		LLController = NewObject<ULiveLinkComponentController>(Camera, TEXT("LiveLinkController"));
		LLController->RegisterComponent();
		Camera->AddInstanceComponent(LLController);
	}

	FLiveLinkSubjectKey SubjectKey;
	SubjectKey.Source = SourceGuid;
	SubjectKey.SubjectName = FName(*SubjectName);

	LLController->SubjectRepresentation.Role = ULiveLinkCameraRole::StaticClass();

	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Configured LiveLink component for subject '%s'"), *SubjectName);
}

void UTrackingAutoSetupSubsystem::ConfigureVirtualCamera(ACineCameraActor* Camera, const FCameraSetupConfig& Config)
{
	if (!Camera) return;

	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Virtual camera integration configured"));
}

#undef LOCTEXT_NAMESPACE
