#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"      
#include "Widgets/Notifications/SProgressBar.h" 
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Engine/TextureDefines.h"
#include "WcwMemoryMonitorTypes.h"


class STextBlock;


class SWcwMemoryBudgetWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SWcwMemoryBudgetWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
private:
    virtual bool SupportsKeyboardFocus() const override { return false; }
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    struct FGroupUIElement
    {
        EMonitorContentType Type;
        EWcwLLMTag WcwLLMTag = EWcwLLMTag::Untracked;
        TextureGroup GroupEnum=TEXTUREGROUP_World;
        FString GroupName;
        float CurrentMB = 0.f;
        float MaxMB = 512.f;
        
        TSharedPtr<STextBlock> NameTextBlock;
        TSharedPtr<STextBlock> ValueTextBlock;
    };

    TArray<FGroupUIElement> GroupElements;
    
    void AddElement(const FGroupUIElement& Element,TSharedPtr<SVerticalBox> MainVerticalBox);
    void UpdateMemoryData();
};