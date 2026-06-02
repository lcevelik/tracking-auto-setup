// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "Widgets/FonixFlowAIChatTools.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FString FChatToolDefinition::BuildToolsJSONArray()
{
	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);

	Writer->WriteArrayStart();

	// get_plugin_state
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("get_plugin_state"));
	Writer->WriteValue(TEXT("description"), TEXT("Get the current plugin state: selected camera, protocol, lens type, calibration status, live values. Call this first to understand what the user has configured."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// select_camera
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("select_camera"));
	Writer->WriteValue(TEXT("description"), TEXT("Select a CineCameraActor in the level by its actor label name. Use get_plugin_state first to see available cameras."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectStart(TEXT("camera_name"));
	Writer->WriteValue(TEXT("type"), TEXT("string"));
	Writer->WriteValue(TEXT("description"), TEXT("The exact actor label of the camera to select."));
	Writer->WriteObjectEnd(); // camera_name
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteValue(TEXT("camera_name"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// set_protocol
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("set_protocol"));
	Writer->WriteValue(TEXT("description"), TEXT("Set the tracking protocol. FreeD is the industry standard for camera tracking. OpenTrackIO is not yet implemented."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectStart(TEXT("protocol"));
	Writer->WriteValue(TEXT("type"), TEXT("string"));
	Writer->WriteValue(TEXT("description"), TEXT("Protocol name: 'FreeD' or 'OpenTrackIO'."));
	Writer->WriteArrayStart(TEXT("enum"));
	Writer->WriteValue(TEXT("FreeD"));
	Writer->WriteValue(TEXT("OpenTrackIO"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // protocol
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteValue(TEXT("protocol"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// set_lens_type
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("set_lens_type"));
	Writer->WriteValue(TEXT("description"), TEXT("Set the lens type (Prime or Zoom). Prime has a fixed focal length, Zoom has a variable range."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectStart(TEXT("lens_type"));
	Writer->WriteValue(TEXT("type"), TEXT("string"));
	Writer->WriteValue(TEXT("description"), TEXT("Lens type: 'Prime' or 'Zoom'."));
	Writer->WriteArrayStart(TEXT("enum"));
	Writer->WriteValue(TEXT("Prime"));
	Writer->WriteValue(TEXT("Zoom"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // lens_type
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteValue(TEXT("lens_type"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// set_prime_focal_length
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("set_prime_focal_length"));
	Writer->WriteValue(TEXT("description"), TEXT("Set the focal length for a prime lens in mm."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectStart(TEXT("focal_length_mm"));
	Writer->WriteValue(TEXT("type"), TEXT("number"));
	Writer->WriteValue(TEXT("description"), TEXT("Focal length in millimeters (8-300)."));
	Writer->WriteObjectEnd(); // focal_length_mm
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteValue(TEXT("focal_length_mm"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// set_zoom_range
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("set_zoom_range"));
	Writer->WriteValue(TEXT("description"), TEXT("Set the min/max focal length range for a zoom lens in mm."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectStart(TEXT("min_mm"));
	Writer->WriteValue(TEXT("type"), TEXT("number"));
	Writer->WriteValue(TEXT("description"), TEXT("Minimum focal length in mm (wide end, 8-300)."));
	Writer->WriteObjectEnd(); // min_mm
	Writer->WriteObjectStart(TEXT("max_mm"));
	Writer->WriteValue(TEXT("type"), TEXT("number"));
	Writer->WriteValue(TEXT("description"), TEXT("Maximum focal length in mm (tele end, 8-300)."));
	Writer->WriteObjectEnd(); // max_mm
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteValue(TEXT("min_mm"));
	Writer->WriteValue(TEXT("max_mm"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// run_setup
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("run_setup"));
	Writer->WriteValue(TEXT("description"), TEXT("Run one-click tracking setup. Configures LiveLink, FreeD source, and camera controller. Requires a camera to be selected first."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// capture_calibration
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("capture_calibration"));
	Writer->WriteValue(TEXT("description"), TEXT("Capture a calibration point from the live FreeD data stream. For focus: capture Near (lens at closest focus) and Far (lens at infinity). For zoom: capture Wide (lens at widest) and Tele (lens at longest)."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectStart(TEXT("target"));
	Writer->WriteValue(TEXT("type"), TEXT("string"));
	Writer->WriteValue(TEXT("description"), TEXT("Calibration target to capture."));
	Writer->WriteArrayStart(TEXT("enum"));
	Writer->WriteValue(TEXT("Near"));
	Writer->WriteValue(TEXT("Far"));
	Writer->WriteValue(TEXT("Wide"));
	Writer->WriteValue(TEXT("Tele"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // target
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteValue(TEXT("target"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	// apply_calibration
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("type"), TEXT("function"));
	Writer->WriteObjectStart(TEXT("function"));
	Writer->WriteValue(TEXT("name"), TEXT("apply_calibration"));
	Writer->WriteValue(TEXT("description"), TEXT("Apply the captured calibration data to create a lens file. Requires all calibration points to be captured first."));
	Writer->WriteObjectStart(TEXT("parameters"));
	Writer->WriteValue(TEXT("type"), TEXT("object"));
	Writer->WriteObjectStart(TEXT("properties"));
	Writer->WriteObjectEnd(); // properties
	Writer->WriteArrayStart(TEXT("required"));
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd(); // parameters
	Writer->WriteObjectEnd(); // function
	Writer->WriteObjectEnd();

	Writer->WriteArrayEnd(); // tools array
	Writer->Close();

	return Output;
}

FString FChatToolDefinition::ExecuteTool(
	const FString& ToolName,
	const FString& ArgumentsJSON,
	IFonixFlowTrackerActions* Actions)
{
	if (!Actions)
	{
		return TEXT("ERROR: Plugin actions not available.");
	}

	// Parse arguments
	TSharedPtr<FJsonObject> Args;
	if (!ArgumentsJSON.IsEmpty())
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ArgumentsJSON);
		FJsonSerializer::Deserialize(Reader, Args);
	}

	if (ToolName == TEXT("get_plugin_state"))
	{
		return Actions->GetState().ToJSON();
	}

	if (ToolName == TEXT("select_camera"))
	{
		FString CameraName = Args.IsValid() ? Args->GetStringField(TEXT("camera_name")) : TEXT("");
		if (CameraName.IsEmpty())
		{
			TArray<FString> Names = Actions->GetAvailableCameraNames();
			return FString::Printf(TEXT("Available cameras: %s. Specify camera_name to select one."),
				*FString::Join(Names, TEXT(", ")));
		}
		return Actions->SelectCamera(CameraName)
			? FString::Printf(TEXT("Camera '%s' selected."), *CameraName)
			: FString::Printf(TEXT("Camera '%s' not found."), *CameraName);
	}

	if (ToolName == TEXT("set_protocol"))
	{
		FString Protocol = Args.IsValid() ? Args->GetStringField(TEXT("protocol")) : TEXT("");
		return Actions->SetProtocol(Protocol)
			? FString::Printf(TEXT("Protocol set to %s."), *Protocol)
			: FString::Printf(TEXT("Invalid protocol '%s'. Use 'FreeD' or 'OpenTrackIO'."), *Protocol);
	}

	if (ToolName == TEXT("set_lens_type"))
	{
		FString LensType = Args.IsValid() ? Args->GetStringField(TEXT("lens_type")) : TEXT("");
		return Actions->SetLensType(LensType)
			? FString::Printf(TEXT("Lens type set to %s."), *LensType)
			: FString::Printf(TEXT("Invalid lens type '%s'. Use 'Prime' or 'Zoom'."), *LensType);
	}

	if (ToolName == TEXT("set_prime_focal_length"))
	{
		float MM = Args.IsValid() ? (float)Args->GetNumberField(TEXT("focal_length_mm")) : 0.0f;
		return Actions->SetPrimeFocalLength(MM)
			? FString::Printf(TEXT("Prime focal length set to %.0f mm."), MM)
			: FString::Printf(TEXT("Invalid focal length %.0f mm. Must be 8-300."), MM);
	}

	if (ToolName == TEXT("set_zoom_range"))
	{
		float MinMM = Args.IsValid() ? (float)Args->GetNumberField(TEXT("min_mm")) : 0.0f;
		float MaxMM = Args.IsValid() ? (float)Args->GetNumberField(TEXT("max_mm")) : 0.0f;
		return Actions->SetZoomRange(MinMM, MaxMM)
			? FString::Printf(TEXT("Zoom range set to %.0f-%.0f mm."), MinMM, MaxMM)
			: FString::Printf(TEXT("Invalid zoom range %.0f-%.0f mm. Min must be < Max, both 8-300."), MinMM, MaxMM);
	}

	if (ToolName == TEXT("run_setup"))
	{
		return Actions->RunSetup();
	}

	if (ToolName == TEXT("capture_calibration"))
	{
		FString Target = Args.IsValid() ? Args->GetStringField(TEXT("target")) : TEXT("");
		return Actions->CaptureCalibration(Target);
	}

	if (ToolName == TEXT("apply_calibration"))
	{
		return Actions->ApplyCalibrationAction();
	}

	return FString::Printf(TEXT("Unknown tool: %s"), *ToolName);
}
