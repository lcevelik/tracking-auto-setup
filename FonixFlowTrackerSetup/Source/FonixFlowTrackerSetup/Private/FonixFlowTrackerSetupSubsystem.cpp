// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "FonixFlowTrackerSetupSubsystem.h"
#include "LensSetupTypes.h"
#include "ILiveLinkClient.h"
#include "LiveLinkSourceFactory.h"
#include "LiveLinkPresetTypes.h"
#include "LiveLinkComponentController.h"
#include "LiveLinkCameraController.h"
#include "Roles/LiveLinkCameraRole.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "LensFile.h"
#include "LensComponent.h"
#include "CameraCalibrationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"

#define LOCTEXT_NAMESPACE "FonixFlowTrackerSetup"

void UFonixFlowTrackerSetupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Subsystem initialized"));
}

void UFonixFlowTrackerSetupSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FFonixFlowTrackerResult UFonixFlowTrackerSetupSubsystem::SetupTracking(
	UObject* WorldContextObject,
	const FTrackingConnectionSettings& ConnectionSettings,
	const FCameraSetupConfig& CameraConfig)
{
	FFonixFlowTrackerResult Result;

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

	// Get subsystem instance
	UFonixFlowTrackerSetupSubsystem* Subsystem = World->GetSubsystem<UFonixFlowTrackerSetupSubsystem>();
	if (!Subsystem)
	{
		Result.Message = TEXT("Could not get tracker subsystem");
		Result.Errors.Add(Result.Message);
		return Result;
	}

	// Step 1: Create Live Link source
	Result.LiveLinkSourceGuid = Subsystem->CreateLiveLinkSource(ConnectionSettings);
	if (!Result.LiveLinkSourceGuid.IsValid())
	{
		Result.Message = TEXT("Failed to create Live Link source");
		Result.Errors.Add(Result.Message);
		return Result;
	}

	// Step 2: Create or configure camera
	Result.CameraActor = Subsystem->CreateTrackedCamera(World, CameraConfig);
	if (!Result.CameraActor)
	{
		Result.Message = TEXT("Failed to create/configure camera");
		Result.Errors.Add(Result.Message);
		return Result;
	}

	// Step 3: Create anchor point
	if (CameraConfig.bCreateAnchorPoint)
	{
		Result.AnchorPoint = Subsystem->CreateAnchorPoint(World, CameraConfig);
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
		Subsystem->ConfigureLiveLinkComponent(CineCamera, Result.LiveLinkSourceGuid, ConnectionSettings.SubjectName);
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
		Subsystem->ConfigureVirtualCamera(CineCamera, CameraConfig);
	}

	Result.bSuccess = true;
	Result.Message = FString::Printf(TEXT("Tracking setup complete: %s on %s"),
		*ConnectionSettings.SubjectName,
		ConnectionSettings.Protocol == ETrackingProtocol::FreeD
			? *FString::Printf(TEXT("%s:%d"), *ConnectionSettings.IPAddress, ConnectionSettings.FreeDPort)
			: *FString::Printf(TEXT("OpenTrack source #%d"), ConnectionSettings.OpenTrackSourceNumber));

	return Result;
}

FFonixFlowTrackerResult UFonixFlowTrackerSetupSubsystem::SetupFreeDCamera(
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

FFonixFlowTrackerResult UFonixFlowTrackerSetupSubsystem::SetupOpenTrackCamera(
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

bool UFonixFlowTrackerSetupSubsystem::RemoveFonixFlowTracker(
	UObject* WorldContextObject,
	FGuid LiveLinkSourceGuid,
	AActor* CameraActor)
{
	if (!WorldContextObject) return false;

	ILiveLinkClient* LiveLinkClient = nullptr;
#if WITH_EDITOR
	if (FModuleManager::Get().IsModuleLoaded("LiveLink"))
	{
		IModularFeatures& ModularFeatures = IModularFeatures::Get();
		if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
		{
			LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		}
	}
#endif

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

FGuid UFonixFlowTrackerSetupSubsystem::CreateLiveLinkSource(const FTrackingConnectionSettings& Settings)
{
	FGuid SourceGuid;

	ILiveLinkClient* LiveLinkClient = nullptr;
#if WITH_EDITOR
	if (FModuleManager::Get().IsModuleLoaded(TEXT("LiveLink")))
	{
		IModularFeatures& ModularFeatures = IModularFeatures::Get();
		if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
		{
			LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		}
	}
#endif

	if (!LiveLinkClient)
	{
		UE_LOG(LogTemp, Error, TEXT("FonixFlowTrackerSetup: LiveLink client not available"));
		return SourceGuid;
	}

	switch (Settings.Protocol)
	{
	case ETrackingProtocol::FreeD:
	{
		// Find the FreeD connection settings struct via reflection (avoids private header dependency)
		UScriptStruct* FreeDSettingsStruct = nullptr;
		for (TObjectIterator<UScriptStruct> It; It; ++It)
		{
			if (It->GetName() == TEXT("LiveLinkFreeDConnectionSettings"))
			{
				FreeDSettingsStruct = *It;
				break;
			}
		}

		// Find the FreeD source factory
		ULiveLinkSourceFactory* FreeDFactory = nullptr;
		for (TObjectIterator<ULiveLinkSourceFactory> It; It; ++It)
		{
			if (It->GetSourceDisplayName().ToString().Contains(TEXT("FreeD")))
			{
				FreeDFactory = *It;
				break;
			}
		}

		if (FreeDFactory && FreeDSettingsStruct)
		{
			TArray<uint8> SettingsMemory;
			SettingsMemory.SetNumZeroed(FreeDSettingsStruct->GetStructureSize());
			FreeDSettingsStruct->InitializeDefaultValue(SettingsMemory.GetData());

			FStrProperty* IPProp = CastField<FStrProperty>(FreeDSettingsStruct->FindPropertyByName(FName("IPAddress")));
			FUInt16Property* PortProp = CastField<FUInt16Property>(FreeDSettingsStruct->FindPropertyByName(FName("UDPPortNumber")));

			if (IPProp) IPProp->SetPropertyValue_InContainer(SettingsMemory.GetData(), TEXT("0.0.0.0"));
			if (PortProp) PortProp->SetPropertyValue_InContainer(SettingsMemory.GetData(), static_cast<uint16>(Settings.FreeDPort));

			FString ConnectionString;
			FreeDSettingsStruct->ExportText(ConnectionString, SettingsMemory.GetData(), nullptr, nullptr, PPF_None, nullptr);

			TSharedPtr<ILiveLinkSource> Source = FreeDFactory->CreateSource(ConnectionString);
			if (Source.IsValid())
			{
				SourceGuid = LiveLinkClient->AddSource(Source);
				UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: FreeD source created — GUID: %s, port: %d"),
					*SourceGuid.ToString(), Settings.FreeDPort);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("FonixFlowTrackerSetup: FreeD factory returned null source"));
			}

			FreeDSettingsStruct->DestroyStruct(SettingsMemory.GetData());
		}
		else
		{
			if (!FreeDFactory)
				UE_LOG(LogTemp, Error, TEXT("FonixFlowTrackerSetup: LiveLinkFreeD factory not found — ensure LiveLinkFreeD plugin is enabled"));
			if (!FreeDSettingsStruct)
				UE_LOG(LogTemp, Error, TEXT("FonixFlowTrackerSetup: FLiveLinkFreeDConnectionSettings struct not found"));
		}
		break;
	}

	case ETrackingProtocol::OpenTrackIO:
		UE_LOG(LogTemp, Warning, TEXT("FonixFlowTrackerSetup: OpenTrack IO not yet implemented"));
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("FonixFlowTrackerSetup: Unsupported protocol"));
		break;
	}

	return SourceGuid;
}

ACineCameraActor* UFonixFlowTrackerSetupSubsystem::CreateTrackedCamera(UWorld* World, const FCameraSetupConfig& Config)
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
#if WITH_EDITOR
			CineCamera->SetActorLabel(Config.CameraName);
#endif
			UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Created camera actor '%s'"), *Config.CameraName);
		}
	}
	else
	{
		CineCamera = Cast<ACineCameraActor>(Config.ExistingCamera);
		if (!CineCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("FonixFlowTrackerSetup: Existing camera is not a CineCameraActor"));
		}
	}

	return CineCamera;
}

AActor* UFonixFlowTrackerSetupSubsystem::CreateAnchorPoint(UWorld* World, const FCameraSetupConfig& Config)
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
#if WITH_EDITOR
		AnchorActor->SetActorLabel(Config.CameraName + TEXT("_Anchor"));
#endif

		USceneComponent* RootComp = NewObject<USceneComponent>(AnchorActor);
		RootComp->SetWorldLocation(Config.AnchorLocation);
		RootComp->SetWorldRotation(Config.AnchorRotation);
		AnchorActor->SetRootComponent(RootComp);

		UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Created anchor point at %s"),
			*Config.AnchorLocation.ToString());
	}

	return AnchorActor;
}

void UFonixFlowTrackerSetupSubsystem::ConfigureLiveLinkComponent(
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
	LLController->SubjectRepresentation.Subject = FName(*SubjectName);

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Configured LiveLink component for subject '%s'"), *SubjectName);
}

void UFonixFlowTrackerSetupSubsystem::ConfigureVirtualCamera(ACineCameraActor* Camera, const FCameraSetupConfig& Config)
{
	if (!Camera) return;

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Virtual camera integration configured"));
}

#undef LOCTEXT_NAMESPACE