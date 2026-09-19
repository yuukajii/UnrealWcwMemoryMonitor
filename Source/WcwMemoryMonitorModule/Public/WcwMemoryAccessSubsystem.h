#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Engine/TextureLODSettings.h"
#include "WcwMemoryMonitorTypes.h"
#include "WcwMemoryAccessSubsystem.generated.h"

class SWcwMemoryBudgetWidget;

UCLASS()
class WCWMEMORYMONITORMODULE_API UWcwMemoryAccessSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void FetchMemoryStats();
    float GetTotalVRAM_MB() const;
    float GetUsedVRAM_MB() const;
    float GetTextureGroupMemoryMB(TextureGroup Group) const;
    float GetTotalTextureMemoryMB() const;

    bool GetSystemMemoryMB(const FString& SystemName,FWcwSystemMemInfo& OutSystemMemInfo);
    float GetLLMMemoryMB(EWcwLLMTag Tag) const;
    void ToggleWidget();
    void ForceCloseWidget();
	float GetRhiResourceMemory(EWcwRhiResourceGroup Group);

private:
    TMap<FString, FWcwSystemMemInfo> SystemMemoryMap;
    TMap<FString, float> TexGroupMemoryMap;
    float TotalTextureMemoryMB = 0.f;

    TSharedPtr<SWcwMemoryBudgetWidget> BudgetWidgetInstance;
	TMap<EWcwRhiResourceGroup, FWcwRhiResourceStatsInfo>  RhiResourceStats;
#if WITH_EDITOR
    void HandleEndPIE(bool bIsSimulating);
#endif
};
