#include "WcwMemoryAccessSubsystem.h"
#include "SWcwMemoryBudgetWidget.h"

#if WITH_EDITOR
#include "Editor.h"
#endif
#if !UE_BUILD_SHIPPING
#include "Engine/Texture.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "UObject/UObjectIterator.h"
#include "ContentStreaming.h"
#endif
#include "WcwMemoryMonitorSettings.h"
#include "ContentStreaming.h"
#include "RHI.h"

#if PLATFORM_WINDOWS || PLATFORM_MICROSOFT
#include "Windows/AllowWindowsPlatformTypes.h"
#include <dxgi1_4.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

    static FAutoConsoleCommand Uc_ToggleMemoryBudget(
        TEXT("Wcw.MemoryMonitor.ToggleUI"),
        TEXT("Toggles the Memory Budget debug UI on/off."),
        FConsoleCommandDelegate::CreateStatic([]()
        {
    #if !UE_BUILD_SHIPPING
            if (GEngine)
            {
                if (UWcwMemoryAccessSubsystem* Subsystem = GEngine->GetEngineSubsystem<UWcwMemoryAccessSubsystem>())
                {
                    Subsystem->ToggleWidget();
                }
            }
   #endif
        })
    );



void UWcwMemoryAccessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if WITH_EDITOR
    FEditorDelegates::EndPIE.AddUObject(this, &UWcwMemoryAccessSubsystem::HandleEndPIE);
#endif
}

void UWcwMemoryAccessSubsystem::Deinitialize()
{
#if WITH_EDITOR
    FEditorDelegates::EndPIE.RemoveAll(this);
#endif
    ForceCloseWidget();
    Super::Deinitialize();
}

void UWcwMemoryAccessSubsystem::ToggleWidget()
{
    if (!GEngine || !GEngine->GameViewport)
    {
        return;
    }

    if (BudgetWidgetInstance.IsValid())
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(BudgetWidgetInstance.ToSharedRef());
        BudgetWidgetInstance.Reset();
        UE_LOG(LogTemp, Log, TEXT("Memory Budget UI: OFF"));
    }
    else
    {
        BudgetWidgetInstance = SNew(SWcwMemoryBudgetWidget);
        GEngine->GameViewport->AddViewportWidgetContent(BudgetWidgetInstance.ToSharedRef(), INT32_MAX);
        UE_LOG(LogTemp, Log, TEXT("Memory Budget UI: ON"));
    }
}
void UWcwMemoryAccessSubsystem::ForceCloseWidget()
{
    if (GEngine && GEngine->GameViewport && BudgetWidgetInstance.IsValid())
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(BudgetWidgetInstance.ToSharedRef());
        BudgetWidgetInstance.Reset();
        UE_LOG(LogTemp, Log, TEXT("Memory Budget UI: Automatically closed due to PIE End."));
    }
}

#if WITH_EDITOR
void UWcwMemoryAccessSubsystem::HandleEndPIE(bool bIsSimulating)
{
    ForceCloseWidget();
}
#endif

void UWcwMemoryAccessSubsystem::FetchMemoryStats()
{
    const UWcwMemoryMonitorSettings* Settings = UWcwMemoryMonitorSettings::Get();


    {
        TArray<FString> TextureGroupNames = UTextureLODSettings::GetTextureGroupNames();
        TexGroupMemoryMap.Empty();
        TexGroupMemoryMap.Reserve(TextureGroupNames.Num());
        TotalTextureMemoryMB = 0.f;

        for(FString GroupName : TextureGroupNames)
        {
          TexGroupMemoryMap.FindOrAdd(GroupName);
        }

        for (TObjectIterator<UTexture> It; It; ++It)
        {
            UTexture* Texture = *It;
            if (!Texture || !Texture->GetResource())
            {
                continue;
            }

            if (!Texture->GetOuter() || Texture->HasAnyFlags(RF_Transient))
            {
                continue;
            }

            TextureGroup Group = Texture->LODGroup;
        
            float MemoryMB = static_cast<float>(Texture->CalcTextureMemorySizeEnum(TMC_ResidentMips)) / (1024.0f * 1024.0f);

            TexGroupMemoryMap[TextureGroupNames[Group]] += MemoryMB;
            TotalTextureMemoryMB += MemoryMB;
        }
    }

    {//SystemMemory
       SystemMemoryMap.Empty();
       SystemMemoryMap.Reserve(static_cast<int>(EWcwSystemGroup::Max));

       const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
       TArray<FString> SystemMemoryGroups = UWcwMemoryMonitorSettings::GetWcwSystemMemoryGroupNames();
        for(FString GroupName : SystemMemoryGroups)
        {
          SystemMemoryMap.FindOrAdd(GroupName);
        }

       FWcwSystemMemInfo EntryMem;
       {//PhysicalMemory
           float RamMaxBytes = static_cast<float>(MemStats.UsedPhysical + MemStats.AvailablePhysical);
           EntryMem.UseMemory = MemStats.UsedPhysical/(1024.0f*1024.0f);
           EntryMem.MaxMemory = RamMaxBytes/(1024.0f*1024.0f);
           SystemMemoryMap[SystemMemoryGroups[ static_cast<int>(EWcwSystemGroup::WcwSystemPhysicalMemory)]] = EntryMem;
       }
       {//VRAM
           EntryMem.UseMemory = GetUsedVRAM_MB();
           EntryMem.MaxMemory = GetTotalVRAM_MB();
           SystemMemoryMap[SystemMemoryGroups[ static_cast<int>(EWcwSystemGroup::WcwSystemVRAM)]] = EntryMem;
       }
       const IConsoleVariable* CVarTextureStreaming = IConsoleManager::Get().FindConsoleVariable(TEXT("r.TextureStreaming"));
	   if (CVarTextureStreaming && CVarTextureStreaming->GetInt() != 0)
       {//TexturePool
           FStreamingManagerCollection& StreamingManagerCollection = IStreamingManager::Get();
           IRenderAssetStreamingManager& TextureStreamingManager = StreamingManagerCollection.GetTextureStreamingManager();

           int64 CurrentPoolBytes = TextureStreamingManager.GetPoolSize();
           float CurrentPoolMB = static_cast<float>(CurrentPoolBytes) / (1024.0f * 1024.0f);
           float PoolRequiredMB = static_cast<float>(TextureStreamingManager.GetRequiredPoolSize()) / (1024.0f * 1024.0f);
           EntryMem.UseMemory = PoolRequiredMB;
           EntryMem.MaxMemory = CurrentPoolMB;
           SystemMemoryMap[SystemMemoryGroups[ static_cast<int>(EWcwSystemGroup::WcwSystemTexturePool)]] = EntryMem;
       }
    }
}

float UWcwMemoryAccessSubsystem::GetLLMMemoryMB(EWcwLLMTag Tag) const
{
#if ENABLE_LOW_LEVEL_MEM_TRACKER
    if (!FLowLevelMemTracker::Get().IsEnabled() || Tag == EWcwLLMTag::Untracked)
    {
        return 0.f;
    }

    ELLMTag NativeTag = ELLMTag::Untagged;
    
    switch (Tag)
    {
        case EWcwLLMTag::Textures:        NativeTag = ELLMTag::Textures; break;
        case EWcwLLMTag::RenderTargets:   NativeTag = ELLMTag::RenderTargets; break;
        case EWcwLLMTag::StaticMeshes:    NativeTag = ELLMTag::StaticMesh; break;
        case EWcwLLMTag::SkeletalMeshes:  NativeTag = ELLMTag::SkeletalMesh; break;
        case EWcwLLMTag::Materials:       NativeTag = ELLMTag::Materials; break;
        
        case EWcwLLMTag::Animation:       NativeTag = ELLMTag::Animation; break;
        case EWcwLLMTag::Audio:           NativeTag = ELLMTag::Audio; break;
        case EWcwLLMTag::Physics:         NativeTag = ELLMTag::Physics; break;
        case EWcwLLMTag::Niagara:         NativeTag = ELLMTag::Niagara; break;
        case EWcwLLMTag::UI:              NativeTag = ELLMTag::UI; break;
        
        case EWcwLLMTag::UObject:         NativeTag = ELLMTag::UObject; break;
        case EWcwLLMTag::EngineMisc:      NativeTag = ELLMTag::EngineMisc; break;
        case EWcwLLMTag::Shaders:         NativeTag = ELLMTag::Shaders; break;
        case EWcwLLMTag::NavigationRecast:      NativeTag = ELLMTag::NavigationRecast; break;

        default: break;
    }

    const int64 Bytes = FLowLevelMemTracker::Get().GetTagAmountForTracker(ELLMTracker::Default, NativeTag, UE::LLM::ESizeParams::Default);
    return static_cast<float>(Bytes) / (1024.0f * 1024.0f);
#else
    return 0.f;
#endif
}
bool UWcwMemoryAccessSubsystem::GetSystemMemoryMB(const FString& SystemName,FWcwSystemMemInfo& OutSystemMemInfo)
{
    const FWcwSystemMemInfo* Found = SystemMemoryMap.Find(SystemName);
    if(Found)
    {
        OutSystemMemInfo = *Found;
    }
    return Found ? true : false;

}
float UWcwMemoryAccessSubsystem::GetTotalVRAM_MB() const
{
    FTextureMemoryStats Stats;
    RHIGetTextureMemoryStats(Stats);

    return static_cast<float>(Stats.DedicatedVideoMemory) / (1024.0f * 1024.0f);
}

float UWcwMemoryAccessSubsystem::GetUsedVRAM_MB() const
{
#if PLATFORM_WINDOWS || PLATFORM_MICROSOFT
    LUID TargetLuid{};
    bool bHasLuid = false;

    {
        IDXGIFactory1* Factory = nullptr;
        if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&Factory))) && Factory)
        {
            IDXGIAdapter1* Adapter = nullptr;
            SIZE_T BestDedicated = 0;

            for (UINT i = 0; Factory->EnumAdapters1(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 Desc{};
                Adapter->GetDesc1(&Desc);
                Adapter->Release();

                const bool bSoftware = (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
                if (!bSoftware && Desc.DedicatedVideoMemory > BestDedicated)
                {
                    BestDedicated = Desc.DedicatedVideoMemory;
                    TargetLuid = Desc.AdapterLuid;
                    bHasLuid = true;
                }
            }
            Factory->Release();
        }
    }

    PDH_HQUERY Query = nullptr;
    if (PdhOpenQuery(nullptr, 0, &Query) != ERROR_SUCCESS)
    {
        return 0.0f;
    }

    PDH_HCOUNTER Counter = nullptr;
    const PDH_STATUS AddStatus = PdhAddEnglishCounter(
        Query,
        TEXT("\\GPU Adapter Memory(*)\\Dedicated Usage"),
        0,
        &Counter);

    if (AddStatus != ERROR_SUCCESS)
    {
        PdhCloseQuery(Query);
        return 0.0f;
    }

    if (PdhCollectQueryData(Query) != ERROR_SUCCESS)
    {
        PdhCloseQuery(Query);
        return 0.0f;
    }

    DWORD BufferSize = 0;
    DWORD ItemCount = 0;
    PdhGetFormattedCounterArray(Counter, PDH_FMT_LARGE, &BufferSize, &ItemCount, nullptr);

    TArray<uint8> Buffer;
    Buffer.SetNumZeroed(BufferSize);
    auto* Items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM*>(Buffer.GetData());

    const PDH_STATUS ArrStatus = PdhGetFormattedCounterArray(
        Counter, PDH_FMT_LARGE, &BufferSize, &ItemCount, Items);

    float UsedMB = 0.0f;
    if (ArrStatus == ERROR_SUCCESS)
    {
        FString TargetToken;
        if (bHasLuid)
        {
            TargetToken = FString::Printf(
                TEXT("luid_0x%08x_0x%08x"),
                static_cast<uint32>(TargetLuid.HighPart),
                static_cast<uint32>(TargetLuid.LowPart));
        }

        int64 BestBytes = 0;
        int64 MatchedBytes = -1;

        for (DWORD i = 0; i < ItemCount; ++i)
        {
            const int64 Bytes = Items[i].FmtValue.largeValue;
            if (Bytes > BestBytes)
            {
                BestBytes = Bytes;
            }

            if (bHasLuid && Items[i].szName)
            {
                const FString Name(Items[i].szName);
                if (Name.Contains(TargetToken, ESearchCase::IgnoreCase))
                {
                    MatchedBytes = Bytes;
                }
            }
        }

        const int64 ResultBytes = (MatchedBytes >= 0) ? MatchedBytes : BestBytes;
        UsedMB = static_cast<float>(ResultBytes) / (1024.0f * 1024.0f);
    }

    PdhCloseQuery(Query);
    return UsedMB;
#else
    FTextureMemoryStats Stats;
    RHIGetTextureMemoryStats(Stats);

    uint64 TotalUsedBytes = Stats.StreamingMemorySize + Stats.NonStreamingMemorySize;
    return static_cast<float>(TotalUsedBytes) / (1024.0f * 1024.0f);
#endif
}

float UWcwMemoryAccessSubsystem::GetTextureGroupMemoryMB(TextureGroup Group) const
{
    TArray<FString> TextureGroupNames = UTextureLODSettings::GetTextureGroupNames();
    const float* Found = TexGroupMemoryMap.Find(TextureGroupNames[Group]);
    return Found ? *Found : 0.f;
}

float UWcwMemoryAccessSubsystem::GetTotalTextureMemoryMB() const
{
    return TotalTextureMemoryMB;
}

