#include "SWcwMemoryBudgetWidget.h"
#include "WcwMemoryAccessSubsystem.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Scalability.h"
#include "Styling/CoreStyle.h"
#include "HAL/IConsoleManager.h"
#include "Widgets/SToolTip.h"
#include "WcwMemoryMonitorSettings.h"
#include "WcwMemoryMonitorTypes.h"
#if WITH_EDITOR
#include "DetailLayoutBuilder.h"
#endif
#include "GameFramework/GameUserSettings.h"

#define MEMORY_WIDGET_WIDTH  500.0f

static TAutoConsoleVariable<float> CVarWcwMemoryMonitorFontScale(
	TEXT("Wcw.MemoryMonitor.FontScale"), 1.0f,
	TEXT(""),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarWcwMemoryMonitorEnableRHI(
    TEXT("Wcw.MemoryMonitor.EnableRHI"), true,
    TEXT("false: Disable heavy RHI resource tracking, true: Enable tracking"),
    ECVF_Default);

static const TCHAR* WcwIni()
{
	static FString Path = FPaths::ProjectConfigDir() / TEXT("Wcw.ini");
	return *Path;
}

static bool GetBuildValue(const TCHAR* Key, FString& Out)
{
	Out.Reset();
	GConfig->LoadFile(WcwIni());
	return GConfig->GetString(TEXT("Wcw.Build"), Key, Out, WcwIni()) && !Out.IsEmpty();
}

static FString GetProjectVersionString()
{
	FString ProjectVersion;
	if (GetBuildValue(TEXT("TitleVersion"), ProjectVersion)
		|| GetBuildValue(TEXT("ProjectVersion"), ProjectVersion))
	{
		return ProjectVersion;
	}

	if (GConfig)
	{
		GConfig->GetString(
			TEXT("/Script/EngineSettings.GeneralProjectSettings"),
			TEXT("ProjectVersion"),
			ProjectVersion,
			GGameIni);
	}
	if (ProjectVersion.IsEmpty())
	{
		ProjectVersion = TEXT("1.0.0.0");
	}
	return ProjectVersion;
}
static FString MakeBuildLabel()
{
	const FString ProjectVersion = GetProjectVersionString();
	const FString EngineVer = FEngineVersion::Current().ToString(EVersionComponent::Patch);

	FString Changelist;
	if (GetBuildValue(TEXT("Changelist"), Changelist))
	{
		return FString::Printf(TEXT("Build: %s  |  CL %s  |  UE %s"), *ProjectVersion, *Changelist, *EngineVer);
	}

	return FString::Printf(TEXT("Build: %s  |  UE %s"), *ProjectVersion, *EngineVer);
}

static FText GetQualityName(int32 QualityLevel)
{
    switch (QualityLevel)
    {
        case 0: return FText::FromString(TEXT("Low"));
        case 1: return FText::FromString(TEXT("Medium"));
        case 2: return FText::FromString(TEXT("High"));
        case 3: return FText::FromString(TEXT("Epic"));
        case 4: return FText::FromString(TEXT("Cinematic"));
        default: return FText::FromString(TEXT("Auto/Unknown"));
    }
}

static FString TextureGroupToString(TextureGroup Group)
{
    if (const UEnum* EnumPtr = StaticEnum<TextureGroup>())
    {
        return EnumPtr->GetNameStringByValue(static_cast<int64>(Group));
    }
    return TEXT("TEXTUREGROUP_Unknown");
}

static FText GetTextureGroupDisplayName(TextureGroup GroupEnum)
{
    const TCHAR* GroupNameTChar = UTexture::GetTextureGroupString(GroupEnum);
    FString DisplayNameOut;

    if (GConfig)
    {
        const UWcwMemoryMonitorSettings* Settings = UWcwMemoryMonitorSettings::Get();

        FString IniSec = TEXT("EnumRemap");
        FString KeyName = FString::Printf(TEXT("%s.DisplayName"), GroupNameTChar);

        if (GConfig->GetString(*IniSec, *KeyName, DisplayNameOut, GEngineIni))
        {
            return FText::FromString(DisplayNameOut);
        }
    }

    return FText::FromString(GroupNameTChar);
}


void SWcwMemoryBudgetWidget::Construct(const FArguments& InArgs)
{
	FString BuildVersion = MakeBuildLabel();
	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();

    IConsoleVariable* FontScaleCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("wcw.MemoryMonitor.FontScale"));
    if (FontScaleCVar)
    {
//        FontScaleCVar->OnChangedDelegate().AddSP(this, &SWcwMemoryBudgetWidget::OnFontScaleCVarChanged);
//        OnFontScaleCVarChanged(FontScaleCVar);
		LastFontScale = FontScaleCVar->GetFloat();
    }

    SetVisibility(EVisibility::HitTestInvisible);

    const UWcwMemoryMonitorSettings* Settings = UWcwMemoryMonitorSettings::Get();
	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
    TSharedPtr<SVerticalBox> MainVerticalBox;
    TSharedPtr<SVerticalBox> QualityVerticalBox;
    FSlateFontInfo CustomFont = FAppStyle::GetFontStyle("NormalFont");
	CustomFont.Size = Settings->FontSize; 

    ChildSlot
    [
        SNew(SVerticalBox)
		// Build:ID
		+ SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [
            SAssignNew(BuildIdTextBlock,STextBlock)
            .Text(FText::FromString(FString::Printf(TEXT("%s"), *BuildVersion)))
			.Font(CustomFont)
            .ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
        ]
		// Main Contents
        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .Padding(15.f)
        [
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 30, 0) // 30px
            [
				SNew(SBorder)
	            .BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
	            .BorderImage(FAppStyle::Get().GetBrush("WhiteBrush")) 
	            .Padding(8.f)
	            [
					SAssignNew(QualityVerticalBox,SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[0],STextBlock).Text(FText::FromString(TEXT("[ Quality Presets ]"))).Font(CustomFont).ColorAndOpacity(FSlateColor(FLinearColor::Gray)) ]
                
					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[1],STextBlock).Text(FText::Format(FText::FromString(TEXT("AntiAliasing : {0}")), GetQualityName(QualityLevels.AntiAliasingQuality))) ]
                
					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[2],STextBlock).Text(FText::Format(FText::FromString(TEXT("ViewDistance : {0}")), GetQualityName(QualityLevels.ViewDistanceQuality))) ]
                
					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[3],STextBlock).Text(FText::Format(FText::FromString(TEXT("Shadow       : {0}")), GetQualityName(QualityLevels.ShadowQuality))) ]
                
					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[4],STextBlock).Text(FText::Format(FText::FromString(TEXT("GlobalIllum  : {0}")), GetQualityName(QualityLevels.GlobalIlluminationQuality))) ]
                
					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[5],STextBlock).Text(FText::Format(FText::FromString(TEXT("Reflection   : {0}")), GetQualityName(QualityLevels.ReflectionQuality))) ]

					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[6],STextBlock).Text(FText::Format(FText::FromString(TEXT("PostProcess  : {0}")), GetQualityName(QualityLevels.PostProcessQuality))) ]

					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[7],STextBlock).Text(FText::Format(FText::FromString(TEXT("Texture      : {0}")), GetQualityName(QualityLevels.TextureQuality))) ]

					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[8],STextBlock).Text(FText::Format(FText::FromString(TEXT("Effects      : {0}")), GetQualityName(QualityLevels.EffectsQuality))) ]

					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[9],STextBlock).Text(FText::Format(FText::FromString(TEXT("Foliage      : {0}")), GetQualityName(QualityLevels.FoliageQuality))) ]

					+ SVerticalBox::Slot().AutoHeight()
					[ SAssignNew(PresetTextBlock[10],STextBlock).Text(FText::Format(FText::FromString(TEXT("Shading      : {0}")), GetQualityName(QualityLevels.ShadingQuality))) ]
				]
			]
			+ SHorizontalBox::Slot()
            .AutoWidth()
            [
				SNew(SBorder)
	            .BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
	            .BorderImage(FAppStyle::Get().GetBrush("WhiteBrush")) 
	            .Padding(8.f)
	            [
	                SAssignNew(MainVerticalBox, SVerticalBox)           
	                + SVerticalBox::Slot()
	                .AutoHeight()
	                .Padding(0.f, 0.f, 0.f, 4.f)
	                [
		                SAssignNew(MemoryBudgetTextBlock,STextBlock)
		                .Text(NSLOCTEXT("WcwMemoryBudget", "Title", "--- MEMORY BUDGET STATS ---"))
		                .Font(CustomFont)
		                .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)))
		            ]
				]
            ]
        ]
    ];

    //SystemMemory
    {
        TArray<FString> SystemMemoryGroups = UWcwMemoryMonitorSettings::GetWcwSystemMemoryGroupNames();
        
        for (FWcwSystemBudgetConfig SystemGroup : Settings->SystemGroups)
        {
            FGroupUIElement Element;
            Element.Type      = EMonitorContentType::SystemInfo;
            Element.GroupName = SystemMemoryGroups[static_cast<int>(SystemGroup.SystemGroupEnum)];
            Element.MaxMB = 0; 
            AddElement(Element,MainVerticalBox);
        }
    }

    //TextureGroup
    {
        TArray<FString> TextureGroupNames = UTextureLODSettings::GetTextureGroupNames();
        for (FWcwTextureGroupBudgetConfig TargetGroup : Settings->TargetGroups)
        {
            FGroupUIElement Element;
            Element.Type      = EMonitorContentType::TextureGroup;
            Element.GroupEnum = TargetGroup.TextureGroupEnum;
            Element.GroupName = TextureGroupNames[TargetGroup.TextureGroupEnum];
            Element.MaxMB = TargetGroup.BudgetMB; 
            AddElement(Element,MainVerticalBox);
        }
    }
    {//LLM
        TArray<FString> WcwLLMMemoryGroups = UWcwMemoryMonitorSettings::GetWcwLLMMemoryGroupNames();
        for (FWcwLLMBudgetConfig LLMGroup : Settings->LLMGroups)
        {
            FGroupUIElement Element;
            Element.Type      = EMonitorContentType::LLMMetrics;
            Element.WcwLLMTag = LLMGroup.WcwLLMTag;
            Element.GroupName = WcwLLMMemoryGroups[static_cast<int>(LLMGroup.WcwLLMTag)];
            Element.MaxMB = LLMGroup.BudgetMB; 
            AddElement(Element,MainVerticalBox);
        }
    }

    {//RHI
        TArray<FString> WcwRHIMemoryGroups = UWcwMemoryMonitorSettings::GetWcwRhiMemoryGroupNames();
        for (FWcwRhiResourceConfig RhiResourceConfig : Settings->RhiResourceGroups)
        {
            FGroupUIElement Element;
            Element.Type      = EMonitorContentType::RHIResource;
            Element.RhiGroup  = RhiResourceConfig.RhiResourceGroup;
            Element.GroupName = WcwRHIMemoryGroups[static_cast<int>(RhiResourceConfig.RhiResourceGroup)];
            AddElement(Element,MainVerticalBox);
        }
    }
}

void SWcwMemoryBudgetWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
   SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
   static IConsoleVariable* FontScaleCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("wcw.MemoryMonitor.FontScale"));
   if (FontScaleCVar)
   {
	   float CurrentScale = FontScaleCVar->GetFloat();
	   if (FMath::IsNearlyEqual(CurrentScale , LastFontScale)==false)
        {
            LastFontScale = CurrentScale;
            ApplyFontScale(CurrentScale); 
        }
   }
   UpdateMemoryData(InDeltaTime);
}

FReply SWcwMemoryBudgetWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) //override
{
    return FReply::Unhandled();
}
FReply SWcwMemoryBudgetWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) //override
{
    return FReply::Unhandled();
}

void SWcwMemoryBudgetWidget::AddElement(const FGroupUIElement& Element,TSharedPtr<SVerticalBox> MainVerticalBox)
{
   const UWcwMemoryMonitorSettings* Settings = UWcwMemoryMonitorSettings::Get();
   FSlateFontInfo CustomFont = FAppStyle::GetFontStyle("NormalFont");
   CustomFont.Size = Settings->FontSize; 

   TSharedPtr<STextBlock> NameText;
   TSharedPtr<STextBlock> ValText;
   MainVerticalBox->AddSlot()
   .AutoHeight()
   .Padding(1.f, 2.f)
   [
       SNew(SHorizontalBox)
       + SHorizontalBox::Slot()
       .AutoWidth()
       .Padding(2.f, 0.f)
       [
           SAssignNew(NameText, STextBlock)
           .Text(FText::FromString(Element.GroupName))
           .Font(CustomFont)
           .MinDesiredWidth(MEMORY_WIDGET_WIDTH)
       ]
       + SHorizontalBox::Slot()
       .AutoWidth()
       .Padding(2.f, 0.f)
       [
           SAssignNew(ValText, STextBlock)
           .Text(FText::FromString(TEXT("0.0 / 512.0 MB")))
           .Font(CustomFont)
       ]
   ];
   FGroupUIElement EntryElement = Element;
   EntryElement.NameTextBlock = NameText;
   EntryElement.ValueTextBlock = ValText;
   GroupElements.Add(EntryElement);
}

void SWcwMemoryBudgetWidget::ApplyFontScale(float CurrentScale)
{
	{
		const UWcwMemoryMonitorSettings* Settings = UWcwMemoryMonitorSettings::Get();
		FSlateFontInfo CustomFont = FAppStyle::GetFontStyle("NormalFont");
		float BaseSize = Settings->FontSize; 
		{
			if(BuildIdTextBlock.IsValid())
			{
				FSlateFontInfo CurrentFont = BuildIdTextBlock->GetFont();
				CurrentFont.Size = FMath::RoundToInt(BaseSize * CurrentScale); 
				BuildIdTextBlock->SetFont(CurrentFont);
			}
			if(MemoryBudgetTextBlock.IsValid())
			{
				FSlateFontInfo CurrentFont = MemoryBudgetTextBlock->GetFont();
				CurrentFont.Size = FMath::RoundToInt(BaseSize * CurrentScale); 
				MemoryBudgetTextBlock->SetFont(CurrentFont);
			}			

			for(TSharedPtr<STextBlock> TextBlock : PresetTextBlock)
			{
				if(TextBlock.IsValid())
				{
					FSlateFontInfo CurrentFont = TextBlock->GetFont();
					CurrentFont.Size = FMath::RoundToInt(BaseSize * CurrentScale); 
					TextBlock->SetFont(CurrentFont);
				}
			}
			for (int32 i = 0; i < GroupElements.Num(); ++i)
			{
				FGroupUIElement& Elem = GroupElements[i];

				if (Elem.ValueTextBlock.IsValid())
				{
					FSlateFontInfo CurrentFont = Elem.ValueTextBlock->GetFont();
					CurrentFont.Size = FMath::RoundToInt(BaseSize * CurrentScale); 
					Elem.ValueTextBlock->SetFont(CurrentFont);
				}
				if (Elem.NameTextBlock.IsValid())
				{
					FSlateFontInfo CurrentFont = Elem.NameTextBlock->GetFont();
					CurrentFont.Size = FMath::RoundToInt(BaseSize * CurrentScale); 
					Elem.NameTextBlock->SetFont(CurrentFont);
				}
			}
		}
	}
}

void SWcwMemoryBudgetWidget::UpdateMemoryData(float InDeltaTime)
{
    if (!GEngine || !GEngine->GameViewport)
    {
        return;
    }

    UWcwMemoryAccessSubsystem* MemorySubsystem = GEngine ? GEngine->GetEngineSubsystem<UWcwMemoryAccessSubsystem>() : nullptr;

    if (!MemorySubsystem)
    {
        return;
    }
	//Preset
	{
		Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
		if(PresetTextBlock[1].IsValid())
		{
           PresetTextBlock[1]->SetText(FText::Format(FText::FromString(TEXT("AntiAliasing : {0}")), GetQualityName(QualityLevels.AntiAliasingQuality)));
		}
		if(PresetTextBlock[2].IsValid())
		{
           PresetTextBlock[2]->SetText(FText::Format(FText::FromString(TEXT("ViewDistance : {0}")), GetQualityName(QualityLevels.ViewDistanceQuality)));
		}
		if(PresetTextBlock[3].IsValid())
		{
           PresetTextBlock[3]->SetText(FText::Format(FText::FromString(TEXT("Shadow : {0}")), GetQualityName(QualityLevels.ShadowQuality)));
		}
		if(PresetTextBlock[4].IsValid())
		{
           PresetTextBlock[4]->SetText(FText::Format(FText::FromString(TEXT("GlobalIllum : {0}")), GetQualityName(QualityLevels.GlobalIlluminationQuality)));
		}
		if(PresetTextBlock[5].IsValid())
		{
           PresetTextBlock[5]->SetText(FText::Format(FText::FromString(TEXT("Reflection : {0}")), GetQualityName(QualityLevels.ReflectionQuality)));
		}
		if(PresetTextBlock[6].IsValid())
		{
           PresetTextBlock[6]->SetText(FText::Format(FText::FromString(TEXT("PostProcess : {0}")), GetQualityName(QualityLevels.PostProcessQuality)));
		}
		if(PresetTextBlock[7].IsValid())
		{
           PresetTextBlock[7]->SetText(FText::Format(FText::FromString(TEXT("Texture : {0}")), GetQualityName(QualityLevels.TextureQuality)));
		}
		if(PresetTextBlock[8].IsValid())
		{
           PresetTextBlock[8]->SetText(FText::Format(FText::FromString(TEXT("Effects : {0}")), GetQualityName(QualityLevels.EffectsQuality)));
		}
		if(PresetTextBlock[9].IsValid())
		{
           PresetTextBlock[9]->SetText(FText::Format(FText::FromString(TEXT("Foliage : {0}")), GetQualityName(QualityLevels.FoliageQuality)));
		}
		if(PresetTextBlock[10].IsValid())
		{
           PresetTextBlock[10]->SetText(FText::Format(FText::FromString(TEXT("Shading : {0}")), GetQualityName(QualityLevels.ShadingQuality)));
		}
	}

    MemorySubsystem->FetchMemoryStats();

    for (int32 i = 0; i < GroupElements.Num(); ++i)
    {
        FGroupUIElement& Elem = GroupElements[i];
		FString BaseText;
        FLinearColor TextColor;
        bool bApply=false;
		bool bPeakDisp=false;
        switch(Elem.Type)
        {
        case EMonitorContentType::SystemInfo:
            {
                FWcwSystemMemInfo Info;
                if(MemorySubsystem->GetSystemMemoryMB(Elem.GroupName,Info)!=false)
                {
                    Elem.CurrentMB = Info.UseMemory;
                    Elem.MaxMB = Info.MaxMemory;
                    TextColor = FLinearColor::White;

                    bApply=true;
					BaseText = FString::Printf(TEXT("Use: %5.1f / Max: %5.1f MB"), Elem.CurrentMB, Elem.MaxMB);
                }
            }
            break;
        case EMonitorContentType::TextureGroup:
            {
				bPeakDisp=true;
				Elem.CurrentMB = MemorySubsystem->GetTextureGroupMemoryMB(Elem.GroupEnum);
				const bool bIsOver = Elem.CurrentMB > Elem.MaxMB;
				TextColor = bIsOver ? FLinearColor(1.0f, 0.2f, 0.2f, 1.0f) : FLinearColor::White;
				bApply=true;
				if (Elem.NameTextBlock.IsValid())
				{
					Elem.NameTextBlock->SetText(GetTextureGroupDisplayName(Elem.GroupEnum));
				}
				BaseText = FString::Printf(TEXT("Use: %5.1f / Budget: %5.1f MB"), Elem.CurrentMB, Elem.MaxMB);
            }
            break;
        case EMonitorContentType::LLMMetrics:
            {
				bPeakDisp=true;
				Elem.CurrentMB = MemorySubsystem->GetLLMMemoryMB(Elem.WcwLLMTag);
				const bool bIsOver = Elem.CurrentMB > Elem.MaxMB;
				TextColor = bIsOver ? FLinearColor(1.0f, 0.2f, 0.2f, 1.0f) : FLinearColor::White;
				bApply=true;
				BaseText = FString::Printf(TEXT("Use: %5.1f / Budget: %5.1f MB"), Elem.CurrentMB, Elem.MaxMB);
            }
            break;
		case EMonitorContentType::RHIResource:
			{
				if (CVarWcwMemoryMonitorEnableRHI.GetValueOnAnyThread() != false)
                {
                  Elem.CurrentMB = MemorySubsystem->GetRhiResourceMemory(Elem.RhiGroup);
                  TextColor = FLinearColor::White;
                  bApply=true;
    			  BaseText = FString::Printf(TEXT("Use: %5.1f MB"), Elem.CurrentMB);
                }
                else
                {
					bApply = true;
                    BaseText = TEXT("Use: --- MB (Disabled)");
                    TextColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
                    
                    Elem.CurrentMB = 0.0f;
                    Elem.PeakValueMB = 0.0f;
                    Elem.PeakHoldTimer = 0.0f;
				}
			}
			break;
        default:break;
        }
        if(bApply==false)continue;

		// Peak-Hold
        if (Elem.CurrentMB > Elem.PeakValueMB && bPeakDisp!=false)
        {
            Elem.PeakValueMB = Elem.CurrentMB;
            Elem.PeakTime = FDateTime::Now();
			if(Elem.CurrentMB > Elem.MaxMB)
            Elem.PeakHoldTimer = 5.0f; 
        }
        else
        {
            Elem.PeakHoldTimer -= InDeltaTime;
            if (Elem.PeakHoldTimer <= 0.0f)
            {
                Elem.PeakValueMB = Elem.CurrentMB;
            }
        }
		FString FinalText = BaseText;
		if (Elem.PeakHoldTimer > 0.0f && Elem.PeakValueMB > Elem.CurrentMB)
        {
            FString TimeStr = Elem.PeakTime.ToString(TEXT("%H:%M:%S"));
            FinalText += FString::Printf(TEXT("   (Peak: %5.1f MB @ %s)"), Elem.PeakValueMB, *TimeStr);
            //TextColor = FLinearColor::Yellow;
        }

        if (Elem.ValueTextBlock.IsValid())
        {
            Elem.ValueTextBlock->SetText(FText::FromString(FinalText));
            Elem.ValueTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
        }
    }
}
