// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "FonixFlowTrackerSetupModule.h"

#define LOCTEXT_NAMESPACE "FFonixFlowTrackerSetupModule"

void FFonixFlowTrackerSetupModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Module started"));
}

void FFonixFlowTrackerSetupModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("FonixFlowTrackerSetup: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFonixFlowTrackerSetupModule, FonixFlowTrackerSetup)
