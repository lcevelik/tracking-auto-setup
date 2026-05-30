// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "LensSetupTypes.h"
#include "LensFile.h"
#include "LensData.h"
#include "Tables/EncodersTable.h"
#include "Tables/FocalLengthTable.h"
#include "Tables/ImageCenterTable.h"
#include "CineCameraComponent.h"
#include "CineCameraActor.h"
#include "LensComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/ObjectLibrary.h"
#include "UObject/SavePackage.h"

// FLensEncoderRange implementation

float FLensEncoderRange::MapRawToPhysical(int32 RawValue) const
{
	if (!bIsCalibrated)
	{
		return static_cast<float>(RawValue);
	}

	int32 MaxRaw = GetMaxRawValue();
	if (MaxRaw == 0) return PhysicalMin;

	float Normalized = FMath::Clamp(static_cast<float>(RawValue - RawMin) / static_cast<float>(RawMax - RawMin), 0.0f, 1.0f);
	return FMath::Lerp(PhysicalMin, PhysicalMax, Normalized);
}

int32 FLensEncoderRange::MapPhysicalToRaw(float PhysicalValue) const
{
	if (!bIsCalibrated)
	{
		return static_cast<int32>(PhysicalValue);
	}

	float Normalized = FMath::Clamp((PhysicalValue - PhysicalMin) / (PhysicalMax - PhysicalMin), 0.0f, 1.0f);
	return FMath::Lerp(RawMin, RawMax, Normalized);
}

// ULensSetupUtility implementation

ULensFile* ULensSetupUtility::CreateLensFile(const FLensConfiguration& Config)
{
	ULensFile* LensFile = Config.LensFile;

	if (Config.bCreateNewLensFile || !LensFile)
	{
		// Create a new lens file as an asset
		FString PackageName = FString::Printf(TEXT("/Game/FonixFlowTrackerSetup/%s"), *Config.LensFileName);
		UPackage* Package = CreatePackage(*PackageName);

		LensFile = NewObject<ULensFile>(Package, FName(*Config.LensFileName), RF_Public | RF_Standalone | RF_MarkAsNative);

		if (LensFile)
		{
			// Set lens info
			LensFile->LensInfo.LensModelName = Config.LensModelName;
			LensFile->LensInfo.LensSerialNumber = Config.LensSerialNumber;
			LensFile->LensInfo.SensorDimensions = Config.SensorDimensions;
			LensFile->LensInfo.ImageDimensions = Config.ImageDimensions;
			LensFile->LensInfo.SqueezeFactor = Config.SqueezeFactor;

			// Set data mode to Parameters (not STMap)
			LensFile->DataMode = ELensDataMode::Parameters;

			// Populate tables
			PopulateEncoderTable(LensFile, Config);
			PopulateFocalLengthTable(LensFile, Config);
			PopulateImageCenterTable(LensFile, Config);

			// Mark package dirty and save
			Package->MarkPackageDirty();
			LensFile->MarkPackageDirty();

			UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Created lens file '%s' in package '%s'"),
				*Config.LensFileName, *PackageName);
		}
	}
	else if (LensFile)
	{
		// Update existing lens file with new encoder ranges
		PopulateEncoderTable(LensFile, Config);

		LensFile->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Updated existing lens file '%s'"), *LensFile->GetName());
	}

	return LensFile;
}

void ULensSetupUtility::PopulateEncoderTable(ULensFile* LensFile, const FLensConfiguration& Config)
{
	if (!LensFile) return;

	// Clear existing encoder data
	LensFile->EncodersTable.ClearAll();

	// Populate focus encoder mapping
	// Maps raw encoder values (0 to MaxRaw) to physical focus distance (in cm)
	const FLensEncoderRange& FocusRange = Config.FocusEncoderRange;
	int32 NumFocusPoints = FMath::Max(2, Config.NumCalibrationPoints);

	for (int32 i = 0; i < NumFocusPoints; i++)
	{
		float Alpha = static_cast<float>(i) / static_cast<float>(NumFocusPoints - 1);

		// Raw encoder value
		float RawValue = FMath::Lerp(static_cast<float>(FocusRange.RawMin), static_cast<float>(FocusRange.RawMax), Alpha);

		// Physical focus distance in cm
		float PhysicalValue = FMath::Lerp(Config.FocusDistanceMinCM, Config.FocusDistanceMaxCM, Alpha);

		// Add key to the focus encoder curve
		FKeyHandle KeyHandle = LensFile->EncodersTable.Focus.AddKey(RawValue, PhysicalValue);

		UE_LOG(LogTemp, Verbose, TEXT("FonixFlowTrackerSetup: Focus encoder point %d: raw=%.0f -> physical=%.1f cm"),
			i, RawValue, PhysicalValue);
	}

	// Populate zoom/iris encoder mapping
	const FLensEncoderRange& ZoomRange = Config.ZoomEncoderRange;
	int32 NumZoomPoints = FMath::Max(2, Config.NumCalibrationPoints);

	for (int32 i = 0; i < NumZoomPoints; i++)
	{
		float Alpha = static_cast<float>(i) / static_cast<float>(NumZoomPoints - 1);

		// Raw encoder value
		float RawValue = FMath::Lerp(static_cast<float>(ZoomRange.RawMin), static_cast<float>(ZoomRange.RawMax), Alpha);

		// Physical focal length in mm
		float PhysicalValue = FMath::Lerp(Config.FocalLengthMinMM, Config.FocalLengthMaxMM, Alpha);

		// Add key to the iris encoder curve (used for zoom in FreeD)
		FKeyHandle KeyHandle = LensFile->EncodersTable.Iris.AddKey(RawValue, PhysicalValue);

		UE_LOG(LogTemp, Verbose, TEXT("FonixFlowTrackerSetup: Zoom encoder point %d: raw=%.0f -> physical=%.1f mm"),
			i, RawValue, PhysicalValue);
	}

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Populated encoder table with %d focus points and %d zoom points"),
		NumFocusPoints, NumZoomPoints);
}

void ULensSetupUtility::PopulateFocalLengthTable(ULensFile* LensFile, const FLensConfiguration& Config)
{
	if (!LensFile) return;

	// Clear existing focal length data
	LensFile->FocalLengthTable.FocusPoints.Empty();

	// Create calibration points across the zoom range
	int32 NumPoints = FMath::Max(2, Config.NumCalibrationPoints);

	for (int32 i = 0; i < NumPoints; i++)
	{
		float Alpha = static_cast<float>(i) / static_cast<float>(NumPoints - 1);

		// Focus value (use midpoint for each zoom point)
		float FocusValue = FMath::Lerp(Config.FocusDistanceMinCM, Config.FocusDistanceMaxCM, 0.5f);

		// Zoom value (focal length in mm)
		float ZoomValue = FMath::Lerp(Config.FocalLengthMinMM, Config.FocalLengthMaxMM, Alpha);

		// Calculate normalized focal length (FxFy)
		// Fx = focal_length_mm / sensor_width_mm
		// Fy = focal_length_mm / sensor_height_mm
		float Fx = ZoomValue / Config.SensorWidthMM;
		float Fy = ZoomValue / Config.SensorHeightMM;

		FFocalLengthInfo FocalLengthInfo;
		FocalLengthInfo.FxFy = FVector2D(Fx, Fy);

		// Add to focal length table
		LensFile->FocalLengthTable.AddPoint(FocusValue, ZoomValue, FocalLengthInfo, KINDA_SMALL_NUMBER, true);

		UE_LOG(LogTemp, Verbose, TEXT("FonixFlowTrackerSetup: Focal length point %d: focus=%.1f zoom=%.1f Fx=%.3f Fy=%.3f"),
			i, FocusValue, ZoomValue, Fx, Fy);
	}

	// Build focus curves from the added points
	LensFile->FocalLengthTable.BuildFocusCurves();

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Populated focal length table with %d points"), NumPoints);
}

void ULensSetupUtility::PopulateImageCenterTable(ULensFile* LensFile, const FLensConfiguration& Config)
{
	if (!LensFile) return;

	// Clear existing image center data
	LensFile->ImageCenterTable.FocusPoints.Empty();

	// Add default image center (center of sensor)
	float FocusValue = FMath::Lerp(Config.FocusDistanceMinCM, Config.FocusDistanceMaxCM, 0.5f);
	float ZoomValue = FMath::Lerp(Config.FocalLengthMinMM, Config.FocalLengthMaxMM, 0.5f);

	FImageCenterInfo ImageCenterInfo;
	ImageCenterInfo.PrincipalPoint = FVector2D(0.5f, 0.5f); // Center of sensor

	LensFile->ImageCenterTable.AddPoint(FocusValue, ZoomValue, ImageCenterInfo, KINDA_SMALL_NUMBER, true);

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Populated image center table with default center point"));
}

void ULensSetupUtility::ConfigureCineCamera(UCineCameraComponent* Camera, const FLensConfiguration& Config)
{
	if (!Camera) return;

	// Set filmback settings
	Camera->Filmback.SensorWidth = Config.SensorWidthMM;
	Camera->Filmback.SensorHeight = Config.SensorHeightMM;
	Camera->Filmback.SensorAspectRatio = Config.SensorAspectRatio;

	// Set lens settings
	Camera->LensSettings.MinFocalLength = Config.FocalLengthMinMM;
	Camera->LensSettings.MaxFocalLength = Config.FocalLengthMaxMM;
	Camera->LensSettings.MinFStop = 1.2f; // Common minimum
	Camera->LensSettings.MaxFStop = 22.0f; // Common maximum

	// Set focus settings
	Camera->FocusSettings.ManualFocusDistance = Config.FocusDistanceMinCM; // Start at min
	Camera->FocusSettings.FocusSmoothingInterpSpeed = 0.0f; // No smoothing for tracked cameras
	Camera->FocusSettings.bSmoothFocusChanges = false;

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Configured CineCamera - Sensor: %.1fx%.1f mm, Focal: %.1f-%.1f mm, Focus: %.0f-%.0f cm"),
		Config.SensorWidthMM, Config.SensorHeightMM,
		Config.FocalLengthMinMM, Config.FocalLengthMaxMM,
		Config.FocusDistanceMinCM, Config.FocusDistanceMaxCM);
}

void ULensSetupUtility::ConfigureLensComponent(ULensComponent* LensComp, ULensFile* LensFile, bool bUseLiveLink)
{
	if (!LensComp || !LensFile) return;

	// Set the lens file
	FLensFilePicker Picker;
	Picker.bUseDefaultLensFile = false;
	Picker.LensFile = LensFile;
	LensComp->SetLensFilePicker(Picker);

	// Set FIZ evaluation mode
	if (bUseLiveLink)
	{
		LensComp->SetFIZEvaluationMode(EFIZEvaluationMode::UseLiveLink);
	}
	else
	{
		LensComp->SetFIZEvaluationMode(EFIZEvaluationMode::UseCameraSettings);
	}

	// Set filmback override to use lens file sensor dimensions
	LensComp->SetFilmbackOverrideSetting(EFilmbackOverrideSource::LensFile);

	// Set distortion source to lens file
	LensComp->SetDistortionSource(EDistortionSource::LensFile);

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Configured LensComponent with lens file '%s', LiveLink: %s"),
		*LensFile->GetName(), bUseLiveLink ? TEXT("Yes") : TEXT("No"));
}

ULensFile* ULensSetupUtility::ApplyLensConfiguration(ACineCameraActor* Camera, const FLensConfiguration& Config)
{
	if (!Camera) return nullptr;

	UCineCameraComponent* CineCamera = Camera->GetCineCameraComponent();
	if (!CineCamera) return nullptr;

	// Create or update lens file
	ULensFile* LensFile = CreateLensFile(Config);
	if (!LensFile) return nullptr;

	// Configure CineCameraComponent
	ConfigureCineCamera(CineCamera, Config);

	// Find or add LensComponent
	ULensComponent* LensComp = Camera->FindComponentByClass<ULensComponent>();
	if (!LensComp)
	{
		LensComp = NewObject<ULensComponent>(Camera, TEXT("LensComponent"));
		LensComp->RegisterComponent();
		Camera->AddInstanceComponent(LensComp);
	}

	// Configure LensComponent
	ConfigureLensComponent(LensComp, LensFile, true);

	return LensFile;
}

TArray<ULensFile*> ULensSetupUtility::FindAllLensFiles()
{
	TArray<ULensFile*> Result;

	// Use object library to find all ULensFile assets
	TArray<FAssetData> AssetDataList;
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(ULensFile::StaticClass(), false, GIsEditor);
	ObjectLibrary->LoadAssetDataFromPath(TEXT("/Game"));
	ObjectLibrary->GetAssetDataList(AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		ULensFile* LensFile = Cast<ULensFile>(AssetData.GetAsset());
		if (LensFile)
		{
			Result.Add(LensFile);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Found %d lens file assets"), Result.Num());
	return Result;
}

TArray<FVector2D> ULensSetupUtility::GetSensorPresets()
{
	return {
		FVector2D(23.76f, 13.365f),   // Super 35mm (default)
		FVector2D(24.576f, 13.824f),  // ARRI Alexa 35
		FVector2D(27.03f, 14.26f),    // RED V-Raptor
		FVector2D(36.0f, 24.0f),      // Full Frame 35mm
		FVector2D(54.12f, 25.59f),    // ARRI Alexa 65
		FVector2D(21.0f, 11.8f),      // Super 16mm
		FVector2D(17.8f, 10.0f),      // MFT (Micro Four Thirds)
	};
}

FString ULensSetupUtility::GetSensorPresetName(const FVector2D& SensorSize)
{
	// Match within tolerance
	auto IsClose = [](const FVector2D& A, const FVector2D& B, float Tolerance = 0.5f) -> bool
	{
		return FMath::Abs(A.X - B.X) < Tolerance && FMath::Abs(A.Y - B.Y) < Tolerance;
	};

	if (IsClose(SensorSize, FVector2D(23.76f, 13.365f)))   return TEXT("Super 35mm");
	if (IsClose(SensorSize, FVector2D(24.576f, 13.824f)))   return TEXT("ARRI Alexa 35");
	if (IsClose(SensorSize, FVector2D(27.03f, 14.26f)))     return TEXT("RED V-Raptor");
	if (IsClose(SensorSize, FVector2D(36.0f, 24.0f)))       return TEXT("Full Frame 35mm");
	if (IsClose(SensorSize, FVector2D(54.12f, 25.59f)))     return TEXT("ARRI Alexa 65");
	if (IsClose(SensorSize, FVector2D(21.0f, 11.8f)))       return TEXT("Super 16mm");
	if (IsClose(SensorSize, FVector2D(17.8f, 10.0f)))       return TEXT("MFT");

	return TEXT("Custom");
}