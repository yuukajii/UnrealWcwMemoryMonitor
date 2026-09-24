#include "SWcwMemoryBudgetWidget.h"
#include "WcwMemoryAccessSubsystem.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/CoreStyle.h"
#include "HAL/IConsoleManager.h"
#include "Widgets/SToolTip.h"
#include "WcwMemoryMonitorSettings.h"
#include "WcwMemoryMonitorTypes.h"
#if WITH_EDITOR
#include "DetailLayoutBuilder.h"
#endif

#define MEMORY_WIDGET_WIDTH  420.0f

static TAutoConsoleVariable<float> CVarWcwMemoryMonitorFontScale(
	TEXT("Wcw.MemoryMonitor.FontScale"), 1.0f,
	TEXT(""),
	ECVF_Default);


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
    IConsoleVariable* FontScaleCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("wcw.MemoryMonitor.FontScale"));
    if (FontScaleCVar)
    {
//        FontScaleCVar->OnChangedDelegate().AddSP(this, &SWcwMemoryBudgetWidget::OnFontScaleCVarChanged);
//        OnFontScaleCVarChanged(FontScaleCVar);
		LastFontScale = FontScaleCVar->GetFloat();
    }

    SetVisibility(EVisibility::HitTestInvisible);

    const UWcwMemoryMonitorSettings* Settings = UWcwMemoryMonitorSettings::Get();

    TSharedPtr<SVerticalBox> MainVerticalBox;
    FSlateFontInfo CustomFont = FAppStyle::GetFontStyle("NormalFont");
	CustomFont.Size = Settings->FontSize; 

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .Padding(15.f)
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
                    SNew(STextBlock)
                    .Text(NSLOCTEXT("WcwMemoryBudget", "Title", "--- MEMORY BUDGET STATS ---"))
                    .Font(CustomFont)
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)))
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
   UpdateMemoryData();
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

void SWcwMemoryBudgetWidget::UpdateMemoryData()
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

    MemorySubsystem->FetchMemoryStats();

    for (int32 i = 0; i < GroupElements.Num(); ++i)
    {
        FGroupUIElement& Elem = GroupElements[i];
        FText Result;
        FLinearColor TextColor;
        bool bApply=false;
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
                    Result = FText::FromString(FString::Printf(TEXT("Use: %5.1f / Max:%5.1f MB"), Elem.CurrentMB, Elem.MaxMB));
                }
            }
            break;
        case EMonitorContentType::TextureGroup:
            {
                Elem.CurrentMB = MemorySubsystem->GetTextureGroupMemoryMB(Elem.GroupEnum);
                const bool bIsOver = Elem.CurrentMB > Elem.MaxMB;
                TextColor = bIsOver ? FLinearColor(1.0f, 0.2f, 0.2f, 1.0f) : FLinearColor::White;
                bApply=true;
                if (Elem.NameTextBlock.IsValid())
                {
                    Elem.NameTextBlock->SetText(GetTextureGroupDisplayName(Elem.GroupEnum));
                }
                Result = FText::FromString(FString::Printf(TEXT("Use: %5.1f / Budget:%5.1f MB"), Elem.CurrentMB, Elem.MaxMB));
            }
            break;
        case EMonitorContentType::LLMMetrics:
            {
                  Elem.CurrentMB = MemorySubsystem->GetLLMMemoryMB(Elem.WcwLLMTag);
                  const bool bIsOver = Elem.CurrentMB > Elem.MaxMB;
                  TextColor = bIsOver ? FLinearColor(1.0f, 0.2f, 0.2f, 1.0f) : FLinearColor::White;
                  bApply=true;
                  Result = FText::FromString(FString::Printf(TEXT("Use:%5.1f / Budget:%5.1f MB"), Elem.CurrentMB, Elem.MaxMB));
            }
            break;
		case EMonitorContentType::RHIResource:
			{
                  Elem.CurrentMB = MemorySubsystem->GetRhiResourceMemory(Elem.RhiGroup);
                  TextColor = FLinearColor::White;
                  bApply=true;
                  Result = FText::FromString(FString::Printf(TEXT("Use:%5.1f MB"), Elem.CurrentMB));				
			}
			break;
        default:break;
        }
        if(bApply==false)continue;

        if (Elem.ValueTextBlock.IsValid())
        {
            Elem.ValueTextBlock->SetText(Result);
            Elem.ValueTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
        }
    }
}
