// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "TrackingAutoSetupModule.h"

#define LOCTEXT_NAMESPACE "FTrackingAutoSetupModule"

void FTrackingAutoSetupModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Module started"));
}

void FTrackingAutoSetupModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTrackingAutoSetupModule, TrackingAutoSetup)
