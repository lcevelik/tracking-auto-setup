// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "FonixFlowTrackerActions.h"
#include "Serialization/JsonWriter.h"

FString FFonixFlowTrackerState::ToJSON() const
{
	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);

	Writer->WriteObjectStart();

	// Camera
	Writer->WriteValue(TEXT("selected_camera"), SelectedCamera);
	Writer->WriteValue(TEXT("has_camera"), bHasCamera);

	// Protocol
	Writer->WriteValue(TEXT("protocol"), Protocol);
	Writer->WriteValue(TEXT("ip_address"), IPAddress);
	Writer->WriteValue(TEXT("port"), Port);

	// Lens
	Writer->WriteValue(TEXT("lens_type"), LensType);
	Writer->WriteValue(TEXT("prime_focal_length_mm"), PrimeFocalLengthMM);
	Writer->WriteValue(TEXT("zoom_min_mm"), ZoomMinMM);
	Writer->WriteValue(TEXT("zoom_max_mm"), ZoomMaxMM);

	// Status
	Writer->WriteValue(TEXT("setup_complete"), bSetupComplete);
	Writer->WriteValue(TEXT("live_link_active"), bLiveLinkActive);

	// Calibration
	Writer->WriteValue(TEXT("calibration_applied"), bCalibrationApplied);
	Writer->WriteValue(TEXT("near_captured"), bNearCaptured);
	Writer->WriteValue(TEXT("far_captured"), bFarCaptured);
	Writer->WriteValue(TEXT("wide_captured"), bWideCaptured);
	Writer->WriteValue(TEXT("tele_captured"), bTeleCaptured);

	// Live values
	Writer->WriteValue(TEXT("live_focus_cm"), LiveFocusCM);
	Writer->WriteValue(TEXT("live_zoom_mm"), LiveZoomMM);

	Writer->WriteObjectEnd();
	Writer->Close();

	return Output;
}
