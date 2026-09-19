// Copyright Epic Games, Inc. All Rights Reserved.
#include "WcwMemoryMonitorSettings.h"
#include "Engine/TextureDefines.h"
#include "Engine/Texture.h"
#include "Engine/TextureLODSettings.h"

UWcwMemoryMonitorSettings::UWcwMemoryMonitorSettings()
{
    FontSize    =   14;
}
#if WITH_EDITOR
void UWcwMemoryMonitorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) 
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (!PropertyChangedEvent.Property)
    {
        return;
    }

    if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWcwMemoryMonitorSettings, SystemGroups))
    {
		if (SystemGroups.Num() > (int)EWcwSystemGroup::Max )
        {
            SystemGroups.SetNum((int)EWcwSystemGroup::Max);
            UE_LOG(LogTemp, Warning, TEXT("SystemGroups cannot exceed the maximum number of available system groups (%d)."), (int)EWcwSystemGroup::Max);
            return;
        }

        TSet<EWcwSystemGroup> UsedValues;
        for (auto& Item : SystemGroups){
            if (UsedValues.Contains(Item.SystemGroupEnum)){
                for (int32 i = 0; i < static_cast<int32>(EWcwSystemGroup::Max); ++i)
                {
                    EWcwSystemGroup Candidate = static_cast<EWcwSystemGroup>(i);
                    if (!UsedValues.Contains(Candidate))
                    {
                        Item.SystemGroupEnum = Candidate;
                        break;
                    }
                }
            }
            UsedValues.Add(Item.SystemGroupEnum);
        }
    }
    else if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWcwMemoryMonitorSettings, TargetGroups))
    {
        TSet<TextureGroup> UsedValues;
        const int32 NumGroups = UTextureLODSettings::GetTextureGroupNames().Num();
        if (TargetGroups.Num() > NumGroups)
        {
            TargetGroups.SetNum(NumGroups);
            UE_LOG(LogTemp, Warning, TEXT("TargetGroups cannot exceed the maximum number of available texture groups (%d)."), NumGroups);
            return;
        }

        for (auto& Item : TargetGroups)
        {
            if (UsedValues.Contains(Item.TextureGroupEnum))
            {
                for (int32 i = 0; i < NumGroups; ++i)
                {
                    TextureGroup Candidate = static_cast<TextureGroup>(i);
                    if (!UsedValues.Contains(Candidate))
                    {
                        Item.TextureGroupEnum = Candidate;
                        break;
                    }
                }
            }
            UsedValues.Add(Item.TextureGroupEnum);
        }
    }
    else if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWcwMemoryMonitorSettings, LLMGroups))
    {
		
		if (LLMGroups.Num() > (int)EWcwLLMTag::Max )
        {
            LLMGroups.SetNum((int)EWcwLLMTag::Max);
            UE_LOG(LogTemp, Warning, TEXT("LLMGroups cannot exceed the maximum number of available llm groups (%d)."), (int)EWcwLLMTag::Max);
            return;
        }

        TSet<EWcwLLMTag> UsedValues;
        for (auto& Item : LLMGroups)
        {
            if (UsedValues.Contains(Item.WcwLLMTag))
            {
                for (int32 i = 0; i < static_cast<int32>(EWcwLLMTag::Max); ++i)
                {
                    EWcwLLMTag Candidate = static_cast<EWcwLLMTag>(i);
                    if (!UsedValues.Contains(Candidate))
                    {
                        Item.WcwLLMTag = Candidate;
                        break;
                    }
                }
            }
            UsedValues.Add(Item.WcwLLMTag);
        }
    }else if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWcwMemoryMonitorSettings, RhiResourceGroups))
    {
        const int32 MaxGroupIndex = static_cast<int32>(EWcwRhiResourceGroup::Max);
       
        if (RhiResourceGroups.Num() > MaxGroupIndex){
            RhiResourceGroups.SetNum(MaxGroupIndex);
            return;
        }

        TSet<EWcwRhiResourceGroup> UsedValues;
        for (auto& Item : RhiResourceGroups)
        {
            // すでに使われているEnumなら、空いているものを探して割り当てる
            if (UsedValues.Contains(Item.RhiResourceGroup))
            {
                for (int32 i = 0; i < MaxGroupIndex; ++i)
                {
                    EWcwRhiResourceGroup Candidate = static_cast<EWcwRhiResourceGroup>(i);
                    if (!UsedValues.Contains(Candidate))
                    {
                        Item.RhiResourceGroup = Candidate;
                        break;
                    }
                }
            }
            UsedValues.Add(Item.RhiResourceGroup);
        }
    }
}
#endif

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
    TEXT("LLM:Untracked"),
    TEXT("LLM:Textures"), 
    TEXT("LLM:RenderTargets"), 
    TEXT("LLM:StaticMeshes"), 
    TEXT("LLM:SkeletalMeshes"),
    TEXT("LLM:Materials"),
    TEXT("LLM:Shaders"),
    TEXT("LLM:NavigationRecast"),      
    TEXT("LLM:Animation"),       
    TEXT("LLM:Audio"),           
    TEXT("LLM:Physics"),         
    TEXT("LLM:Niagara"),         
    TEXT("LLM:UI"),              
    TEXT("LLM:UObject"),         
    TEXT("LLM:EngineMisc")  
};
static_assert(15 == static_cast<int>(EWcwLLMTag::Max), "WcwLLMMemoryGroupNamesArray size mismatch with EWcwLLMTag!");

TArray<FString> UWcwMemoryMonitorSettings::GetWcwLLMMemoryGroupNames()
{
    return WcwLLMGroupNames;
}

static const TArray<FString> WcwRhiGroupNames = {
    TEXT("RHI:Lumen"),
    TEXT("RHI:Nanite"), 
    TEXT("RHI:Shadow"), 
    TEXT("RHI:DistanceFields"), 
    TEXT("RHI:IndexBuffer"),
    TEXT("RHI:VertexBuffer"),
    TEXT("RHI:VirtualTexture"),
    TEXT("RHI:Hair"),      
};
static_assert(8 == static_cast<int>(EWcwRhiResourceGroup::Max), "WcwRhiGroupNamesArray size mismatch with EWcwRhiResourceGroup!");

TArray<FString> UWcwMemoryMonitorSettings::GetWcwRhiMemoryGroupNames()
{
	return WcwRhiGroupNames;
}

