// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LensFile.h"
#include "LensData.h"
#include "LensSetupTypes.generated.h"

class UCineCameraComponent;
class ULensComponent;
class ULensFile;
class ACineCameraActor;

/**
 * Encoder range data captured during lens calibration
 */
USTRUCT(BlueprintType)
struct TRACKINGAUTOSETUP_API FLensEncoderRange
{
	GENERATED_BODY()

	/** Raw encoder minimum value (lens rotated to min position) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encoder")
	int32 RawMin = 0;

	/** Raw encoder maximum value (lens rotated to max position) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encoder")
	int32 RawMax = 0x00FFFFFF;

	/** Physical minimum value (e.g., focus distance in cm, focal length in mm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
	float PhysicalMin = 0.0f;

	/** Physical maximum value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
	float PhysicalMax = 10000.0f;

	/** Encoder bit depth (24 for FreeD focus/zoom, 16 for user defined) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encoder")
	int32 BitDepth = 24;

	/** Whether this range has been calibrated (min/max captured) */
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bIsCalibrated = false;

	/** Get the max raw value for this bit depth */
	int32 GetMaxRawValue() const { return (1 << BitDepth) - 1; }

	/** Map a raw encoder value to physical value */
	float MapRawToPhysical(int32 RawValue) const;

	/** Map a physical value to raw encoder value */
	int32 MapPhysicalToRaw(float PhysicalValue) const;
};

/**
 * Complete lens configuration for a tracked camera
 */
USTRUCT(BlueprintType)
struct TRACKINGAUTOSETUP_API FLensConfiguration
{
	GENERATED_BODY()

	/** Lens file asset to use (existing or newly created) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens File")
	ULensFile* LensFile = nullptr;

	/** Whether to create a new lens file or use existing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens File")
	bool bCreateNewLensFile = true;

	/** Name for new lens file asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens File", meta = (EditCondition = "bCreateNewLensFile"))
	FString LensFileName = TEXT("TrackedLens");

	/** Lens model name (e.g., "Cooke S4 50mm") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Info")
	FString LensModelName;

	/** Lens serial number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Info")
	FString LensSerialNumber;

	/** Sensor dimensions in mm (width, height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Info")
	FVector2D SensorDimensions = FVector2D(23.76f, 13.365f); // Super 35 default

	/** Image resolution used for calibration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Info")
	FIntPoint ImageDimensions = FIntPoint(1920, 1080);

	/** Squeeze factor for anamorphic lenses (1.0 for spherical) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Info")
	float SqueezeFactor = 1.0f;

	/** Focus distance encoder range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Encoder")
	FLensEncoderRange FocusEncoderRange;

	/** Focal length (zoom) encoder range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom Encoder")
	FLensEncoderRange ZoomEncoderRange;

	/** Iris encoder range (optional, 16-bit for FreeD) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris Encoder")
	FLensEncoderRange IrisEncoderRange;

	/** Physical focus distance range in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Range")
	float FocusDistanceMinCM = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Range")
	float FocusDistanceMaxCM = 10000.0f;

	/** Physical focal length range in mm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom Range")
	float FocalLengthMinMM = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom Range")
	float FocalLengthMaxMM = 100.0f;

	/** Number of calibration points to generate (spread across range) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration", meta = (ClampMin = "2", ClampMax = "50"))
	int32 NumCalibrationPoints = 5;

	/** Filmback settings for the camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filmback")
	float SensorWidthMM = 24.576f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filmback")
	float SensorHeightMM = 13.824f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filmback")
	float SensorAspectRatio = 1.78f;
};

/**
 * Utility class for lens file creation and configuration
 */
UCLASS(BlueprintType)
class TRACKINGAUTOSETUP_API ULensSetupUtility : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Create a complete lens file from the given configuration.
	 * Populates encoder tables, focal length tables, and distortion data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	static ULensFile* CreateLensFile(const FLensConfiguration& Config);

	/**
	 * Configure a CineCameraComponent with the given lens settings.
	 * Sets filmback, focal length range, focus distance range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	static void ConfigureCineCamera(UCineCameraComponent* Camera, const FLensConfiguration& Config);

	/**
	 * Configure a LensComponent with the lens file and FIZ evaluation mode.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	static void ConfigureLensComponent(ULensComponent* LensComp, ULensFile* LensFile, bool bUseLiveLink = true);

	/**
	 * Apply complete lens configuration to a camera actor.
	 * Creates lens file, configures CineCameraComponent and LensComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	static ULensFile* ApplyLensConfiguration(ACineCameraActor* Camera, const FLensConfiguration& Config);

	/**
	 * Populate encoder mapping tables in a lens file.
	 * Maps raw encoder values to physical focus/zoom values.
	 */
	static void PopulateEncoderTable(ULensFile* LensFile, const FLensConfiguration& Config);

	/**
	 * Populate focal length table with calibration points.
	 */
	static void PopulateFocalLengthTable(ULensFile* LensFile, const FLensConfiguration& Config);

	/**
	 * Populate image center table with default values.
	 */
	static void PopulateImageCenterTable(ULensFile* LensFile, const FLensConfiguration& Config);

	/**
	 * Find all ULensFile assets in the project.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	static TArray<ULensFile*> FindAllLensFiles();

	/**
	 * Get common sensor presets (Super 35, Full Frame, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	static TArray<FVector2D> GetSensorPresets();

	/**
	 * Get preset name for a sensor size.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tracking Auto Setup")
	static FString GetSensorPresetName(const FVector2D& SensorSize);
};
