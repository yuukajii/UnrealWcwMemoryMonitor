// Copyright Epic Games, Inc. All Rights Reserved.

#include "WcwMemoryMonitorModule.h"

#define LOCTEXT_NAMESPACE "FWcwMemoryMonitorModuleModule"

void FWcwMemoryMonitorModuleModule::StartupModule()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Log, TEXT("WcwMemoryMonitor Module Started."));
#endif
}

void FWcwMemoryMonitorModuleModule::ShutdownModule()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Log, TEXT("WcwMemoryMonitor Module Shutdown."));
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWcwMemoryMonitorModuleModule, WcwMemoryMonitorModule)