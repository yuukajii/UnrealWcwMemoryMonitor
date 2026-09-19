// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureDefines.h"
#include "WcwMemoryMonitorTypes.generated.h"

UENUM(BlueprintType)
enum class EMonitorContentType : uint8
{
    SystemInfo     UMETA(DisplayName = "System Info"),
    TextureGroup   UMETA(DisplayName = "Texture Group"),
    LLMMetrics     UMETA(DisplayName = "LLM Metrics"),
    RHIResource     UMETA(DisplayName = "RHI Resouce"),
    Max,
};

UENUM(BlueprintType)
enum class EWcwSystemGroup : uint8
{
    WcwSystemPhysicalMemory UMETA(DisplayName = "System:PhysicalMemory"),
    WcwSystemVRAM UMETA(DisplayName = "System:VRAM"),
    WcwSystemTexturePool    UMETA(DisplayName = "System:TexturePool"),
    Max
};

UENUM(BlueprintType)
enum class EWcwLLMTag : uint8
{
    Untracked       UMETA(DisplayName = "LLM:Untracked"),
    
    Textures        UMETA(DisplayName = "LLM:Textures"),     
    RenderTargets   UMETA(DisplayName = "LLM:RenderTargets"),
    StaticMeshes    UMETA(DisplayName = "LLM:StaticMeshes"), 
    SkeletalMeshes  UMETA(DisplayName = "LLM:SkeletalMeshes"), 
    Materials       UMETA(DisplayName = "LLM:Materials"),      
    Shaders         UMETA(DisplayName = "LLM:Shaders"),      

    NavigationRecast    UMETA(DisplayName = "LLM:NavigationRecast"),      
    Animation       UMETA(DisplayName = "LLM:Animation"),      
    Audio           UMETA(DisplayName = "LLM:Audio"),         
    Physics         UMETA(DisplayName = "LLM:Physics"),       
    Niagara         UMETA(DisplayName = "LLM:Niagara"),       
    UI              UMETA(DisplayName = "LLM:UI"),            
    
    UObject         UMETA(DisplayName = "LLM:UObject"),       
    EngineMisc      UMETA(DisplayName = "LLM:EngineMisc"),    
    Max,
};

UENUM(BlueprintType)
enum class EWcwRhiResourceGroup : uint8
{
    WcwRhiLumen				UMETA(DisplayName = "RHI:Lumen"),
    WcwRhiNanite			UMETA(DisplayName = "RHI:Nanite"),
    WcwRhiShadow			UMETA(DisplayName = "RHI:Shadow"),
    WcwRhiDistanceFields    UMETA(DisplayName = "RHI:DistanceFields"),
	WcwRhiIndexBuffer		UMETA(DisplayName = "RHI:IndexBuffer"),
	WcwRhiVertexBuffer      UMETA(DisplayName = "RHI:VertexBuffer"),
	WcwRhiVirtualTexture    UMETA(DisplayName = "RHI:VirtualTexture"),
	WcwRhiHair				UMETA(DisplayName = "RHI:Hair"),
    Max
};

struct FWcwSystemMemInfo
{
    float UseMemory;
    float MaxMemory;
};

struct FWcwRhiResourceStatsInfo
{
    int64 NonTransientSize = 0;
    int64 TransientSize = 0;
    int64 GetTotalSize() const { return NonTransientSize + TransientSize; }
};

