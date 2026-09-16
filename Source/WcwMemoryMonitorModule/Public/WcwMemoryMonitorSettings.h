// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/TextureDefines.h"
#include "WcwMemoryMonitorTypes.h"
#include "WcwMemoryMonitorSettings.generated.h"

USTRUCT(BlueprintType)
struct FWcwSystemBudgetConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    EWcwSystemGroup SystemGroupEnum = EWcwSystemGroup::WcwSystemPhysicalMemory;
};

USTRUCT(BlueprintType)
struct FWcwTextureGroupBudgetConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    TEnumAsByte<TextureGroup> TextureGroupEnum = TEXTUREGROUP_World;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget", meta = (ClampMin = "0.0"))
    float BudgetMB = 512.f;
};

USTRUCT(BlueprintType)
struct FWcwLLMBudgetConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    EWcwLLMTag WcwLLMTag = EWcwLLMTag::Untracked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget", meta = (ClampMin = "0.0"))
    float BudgetMB = 512.f;
};


UCLASS(Config = Engine, defaultconfig, meta = (DisplayName = "WCW Memory Monitor Settings"))
class UWcwMemoryMonitorSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UWcwMemoryMonitorSettings();
	virtual FName GetCategoryName() const override;
    virtual FName GetSectionName() const override;
    static TArray<FString> GetWcwSystemMemoryGroupNames();
    static TArray<FString> GetWcwLLMMemoryGroupNames();

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Font", meta = (ClampMin = "6.0", ClampMax = "48.0"))
    float FontSize = 14.f;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "System Groups", meta = (TitleProperty = "SystemGroupEnum"))
    TArray<FWcwSystemBudgetConfig> SystemGroups;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Texture Groups", meta = (TitleProperty = "TextureGroupEnum"))
    TArray<FWcwTextureGroupBudgetConfig> TargetGroups;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "LLM Groups", meta = (TitleProperty = "WcwLLMTag",ToolTip = "Note: The tag specified here must include '-LLM'."))
    TArray<FWcwLLMBudgetConfig> LLMGroups;


    static const UWcwMemoryMonitorSettings* Get() { return GetDefault<UWcwMemoryMonitorSettings>(); }
};
