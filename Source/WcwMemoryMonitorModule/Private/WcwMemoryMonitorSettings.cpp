// Copyright Epic Games, Inc. All Rights Reserved.
#include "WcwMemoryMonitorSettings.h"

UWcwMemoryMonitorSettings::UWcwMemoryMonitorSettings()
{
    FontSize    =   14;
}

FName UWcwMemoryMonitorSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}

FName UWcwMemoryMonitorSettings::GetSectionName() const
{
    return TEXT("WCW Memory Monitor");
}

static const TArray<FString> WcwSystemGroupNames = {
    TEXT("System:PhysicalMemory"),
    TEXT("System:VRAM-PC Only"),
    TEXT("System:TexturePool"),
    TEXT("System:TextureTotal")
};
static_assert(3 == static_cast<int>(EWcwSystemGroup::Max), "WcwSystemGroupNamesArray size mismatch with EWcwSystemGroup!");

TArray<FString> UWcwMemoryMonitorSettings::GetWcwSystemMemoryGroupNames()
{
	return WcwSystemGroupNames;
}

static const TArray<FString> WcwLLMGroupNames = {
    TEXT("LLN:Untracked"),
    TEXT("LLN:Textures"), 
    TEXT("LLN:RenderTargets"), 
    TEXT("LLN:StaticMeshes"), 
    TEXT("LLN:SkeletalMeshes"),
    TEXT("LLN:Materials"),
    TEXT("LLM:Shaders"),
    TEXT("LLM:NavigationRecast"),      
    TEXT("LLN:Animation"),       
    TEXT("LLN:Audio"),           
    TEXT("LLN:Physics"),         
    TEXT("LLN:Niagara"),         
    TEXT("LLN:UI"),              
    TEXT("LLN:UObject"),         
    TEXT("LLN:EngineMisc")  
};
static_assert(15 == static_cast<int>(EWcwLLMTag::Max), "WcwLLMMemoryGroupNamesArray size mismatch with EWcwLLMTag!");

TArray<FString> UWcwMemoryMonitorSettings::GetWcwLLMMemoryGroupNames()
{
    return WcwLLMGroupNames;
}

