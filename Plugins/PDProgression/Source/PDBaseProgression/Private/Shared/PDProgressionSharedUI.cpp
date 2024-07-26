/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "PDProgressionSharedUI.h"

#include "Textures/SlateIcon.h"
#include "Framework/Commands/UIAction.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Views/SListView.h"
#include "Styling/AppStyle.h"

// Class picker 
#include "GameplayTags.h"
#include "Components/PDProgressionComponent.h"
#include "Subsystems/PDProgressionSubsystem.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "SPDStatList"


FText SPDStatList::StatBase_TitleText                 = LOCTEXT("HeaderRow_Field", "STATS");
FText SPDStatList::StatProgress_Header_Name           = LOCTEXT("Stat_HeaderRow_Field_Name", "NAME");
FText SPDStatList::StatProgress_Header_Category       = LOCTEXT("Stat_HeaderRow_Field_Category", "CATEGORY");
FText SPDStatList::StatProgress_Header_CurrentValue   = LOCTEXT("Stat_HeaderRow_Field_Value", "CURR. VALUE");
FText SPDStatList::StatProgress_Header_Level          = LOCTEXT("Stat_HeaderRow_Field_Level", "LEVEL");
FText SPDStatList::StatProgress_Header_Experience     = LOCTEXT("Stat_HeaderRow_Field_Experience", "EXPERIENCE");
FText SPDStatList::StatProgress_Header_ModifiedOffset = LOCTEXT("Stat_HeaderRow_Field_ModifiedOffset", "OFFSET");

//
// SAVE EDITOR MAIN
void SPDStatList::Construct(const FArguments& InArgs, int32 InOwnerID, TArray<TSharedPtr<FPDStatNetDatum>>& DataViewRef, const int32 InSectionWidth)
{
	SectionWidth = InSectionWidth;
	Refresh(InOwnerID, DataViewRef, SectionWidth);
}

// void SPDStatList::Refresh(int32 InOwnerID, TArray<TSharedPtr<FPDStatNetDatum>>& DataViewRef, const TArray<int32>& NewSectionWidths)
void SPDStatList::Refresh(int32 InOwnerID, TArray<TSharedPtr<FPDStatNetDatum>>& DataViewRef, const int32 NewSectionWidth)
{
	OwnerID = InOwnerID;
	StatsAsSharedArray = &DataViewRef;

	// Potentially lazy construction
	if (ActualList.IsValid() == false || SectionWidth != NewSectionWidth)
	{
		UpdateChildSlot();
	}
	else
	{
		PrepareData();
		ActualList->SetItemsSource(StatsAsSharedArray);
		ActualList->RebuildList();
	}
}

void SPDStatList::PrepareData()
{
	// // @todo investigate why it acts like the horizontal boxex width only are correct when they are uniform,
	// // @todo then resolve below and uncomment code
	// constexpr int32 NewSize = EPDStatListSections::EValueOffset + 1;
	//
	// // Ensure that SectionWidths contains 4 entries, resolve and send warning in case it is not!
	// if (SectionWidths.IsValidIndex(EPDStatListSections::EValueOffset))
	// {
	// 	if (SectionWidths.IsValidIndex(NewSize))
	// 	{
	// 		UE_LOG(PDLog_Progression, Warning, TEXT("SPDStatList::PrepareData -- 'SectionWidths' has too many entries, set to 4 total entries."))
	// 		SectionWidths.SetNum(NewSize);
	// 	}
	// }
	// else // Fallback, set our own size
	// {
	// 	UE_LOG(PDLog_Progression, Warning, TEXT("SPDStatList::PrepareData -- 'SectionWidths' has too few entries, increasing to 4 total entries."))
	//
	// 	
	// 	int32 IndexToStartEdit = SectionWidths.Num();
	// 	SectionWidths.SetNum(NewSize);
	// 	for (; IndexToStartEdit < NewSize; IndexToStartEdit++)
	// 	{
	// 		if (IndexToStartEdit == 0)
	// 		{
	// 			SectionWidths[IndexToStartEdit] = 50;
	// 		}
	// 		else
	// 		{
	// 			SectionWidths[IndexToStartEdit] = 20;
	// 		}
	// 	}
	// }
	
	
	UPDStatHandler* SelectedStatHandler = UPDStatSubsystem::Get()->StatHandlers.FindRef(OwnerID);
	if (SelectedStatHandler != nullptr)
	{
		for (const FPDStatNetDatum& Item : SelectedStatHandler->StatList.Items)
		{
			TSharedRef<FPDStatNetDatum> SharedNetDatum = MakeShared<FPDStatNetDatum>(Item);
			StatsAsSharedArray->Emplace(SharedNetDatum);
		}
	}
	
	if (TitleFont.TypefaceFontName.IsNone())
	{
		// @todo Set up a custom slate styleset for the saveeditors fonts and icons 
		TitleFont = FAppStyle::GetFontStyle( TEXT("PropertyWindow.NormalFont"));
		TitleFont.Size *= 8;
	}
}

void SPDStatList::UpdateChildSlot()
{
	PrepareData();
	if (Header.IsValid() == false)
	{
		Header = SNew(SHeaderRow);
	}
	Header->ClearColumns();
	
	SHeaderRow::FColumn::FArguments ColumnArgs;
	ColumnArgs
		.DefaultLabel(StatProgress_Header_Name)
		// .FixedWidth(SectionWidths[EPDStatListSections::EName])
		.FixedWidth(SectionWidth)
		.ColumnId(FName("0")) ;
	Header->AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(StatProgress_Header_Category)
		// .FixedWidth(SectionWidths[EPDStatListSections::ECategory])
		.FixedWidth(SectionWidth)
		.ColumnId(FName("1")) ;
	Header->AddColumn(ColumnArgs);
	
	
	ColumnArgs
		.DefaultLabel(StatProgress_Header_Level)
		// .FixedWidth(SectionWidths[EPDStatListSections::ELevel])
		.FixedWidth(SectionWidth)
		.ColumnId(FName("2")) ;
	Header->AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(StatProgress_Header_Experience)
		// .FixedWidth(SectionWidths[EPDStatListSections::EExperience])
		.FixedWidth(SectionWidth)
		.ColumnId(FName("3")) ;
	Header->AddColumn(ColumnArgs);
	
	ColumnArgs
		.DefaultLabel(StatProgress_Header_CurrentValue)
		// .FixedWidth(SectionWidths[EPDStatListSections::EValue])
		.FixedWidth(SectionWidth)
		.ColumnId(FName("4")) ;
	Header->AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(StatProgress_Header_ModifiedOffset)
		// .FixedWidth(SectionWidths[EPDStatListSections::EValueOffset])
		.FixedWidth(SectionWidth)
		.ColumnId(FName("5")) ;
	Header->AddColumn(ColumnArgs);		

	
	ActualList = SNew(SListView<TSharedPtr<FPDStatNetDatum>>)
		.HeaderRow(Header)
		.ListItemsSource(StatsAsSharedArray)
		.OnGenerateRow( this, &SPDStatList::MakeListViewWidget_AllStatData )
		.OnSelectionChanged( this, &SPDStatList::OnComponentSelected_AllStatData );	
	
	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(10)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot() 
				.Padding(FMargin{0, 0})
				.AutoHeight()
			[
				SNew(STextBlock)
					.Font(TitleFont)
					.Text(StatBase_TitleText)
			]
			
			+ SVerticalBox::Slot() 
				.Padding(FMargin{0, 0})
				.FillHeight(10)
			[
				SNew(SScrollBox)
				.ScrollBarAlwaysVisible(true)
				.ScrollBarVisibility(EVisibility::Visible)
				.ScrollBarThickness(UE::Slate::FDeprecateVector2DParameter(10))
				.Orientation(EOrientation::Orient_Vertical)
				+SScrollBox::Slot()
					.AutoSize()
					.MaxSize(800)
				[
					ActualList.ToSharedRef()
				]
			]
			+ SVerticalBox::Slot() 
				.Padding(FMargin{0, FMath::Clamp(StatsAsSharedArray->Num() * 4.f, 0.f, 30.f)})
				.FillHeight(10)
		]
	];	
}


TSharedRef<ITableRow> SPDStatList::MakeListViewWidget_AllStatData(TSharedPtr<FPDStatNetDatum> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	const FPDStatNetDatum& CurrentStatNetDatum = *InItem.Get();

	const FGameplayTag& StatTag = CurrentStatNetDatum.ProgressionTag;

	const FString& ParentTagString = StatTag.RequestDirectParent().GetTagName().ToString();
	const int32 ParentCutoffIndex = 1 + ParentTagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagCategoryString = ParentTagString.RightChop(ParentCutoffIndex);

	const FString& TagString = StatTag.GetTagName().ToString();
	const int32 CutoffIndex = 1 + TagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagStatString = TagString.RightChop(CutoffIndex);

	const FText StatNameAsText     = FText::FromString(TagStatString);
	const FText StatCategoryAsText = FText::FromString(TagCategoryString);
	const FText LevelAsText        = FText::FromString(FString::FromInt(CurrentStatNetDatum.CurrentLevel));
	const FText ExperienceAsText   = FText::FromString(FString::FromInt(CurrentStatNetDatum.CurrentExperience));

	const FText AppliedValueAsText = FText::FromString(FString::Printf(TEXT("%lf"), CurrentStatNetDatum.GetAppliedValue()));
	const FText ModifiersAsText    = FText::FromString(FString::Printf(TEXT("%lf"), CurrentStatNetDatum.GetProcessedCrossBehaviour()));

	
	//
	// Widget layout
	SPDStatList* MutableThis = const_cast<SPDStatList*>(this);
	check(MutableThis != nullptr)

	// There is still some discrepancy between the content scale and our absolute scale,
	// offsetting by a tiny amount as a workaround for now
	constexpr double WidthDiscrepancy = (1.015);

	 const float TrueSectionWidth = SectionWidth * WidthDiscrepancy;
	// const float TrueSectionWidthName         = SectionWidths[EPDStatListSections::EName] * WidthDiscrepancy;
	// const float TrueSectionWidthCategory     = SectionWidths[EPDStatListSections::ECategory] * WidthDiscrepancy;
	// const float TrueSectionWidthLevel        = SectionWidths[EPDStatListSections::ELevel] * WidthDiscrepancy;
	// const float TrueSectionWidthExperience   = SectionWidths[EPDStatListSections::EExperience] * WidthDiscrepancy;
	// const float TrueSectionWidthValueRep     = SectionWidths[EPDStatListSections::EValue] * WidthDiscrepancy;
	// const float TrueSectionWidthValueOffsets = SectionWidths[EPDStatListSections::EValueOffset] * WidthDiscrepancy;
	
	MutableThis->StatTable =
		SNew( STableRow< TSharedPtr<FPDStatNetDatum> >, OwnerTable )
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		// .MaxWidth(TrueSectionWidthName)
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(StatNameAsText)
			// .MinDesiredWidth(TrueSectionWidthName)
			.MinDesiredWidth(TrueSectionWidth)
		]
		+ SHorizontalBox::Slot()
		// .MaxWidth(TrueSectionWidthCategory)
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(StatCategoryAsText)
			// .MinDesiredWidth(TrueSectionWidthCategory)
			.MinDesiredWidth(TrueSectionWidth)
		]		
		+ SHorizontalBox::Slot()
		// .MaxWidth(TrueSectionWidthLevel)
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(LevelAsText)
			// .MinDesiredWidth(TrueSectionWidthLevel)
			.MinDesiredWidth(TrueSectionWidth)
		]
		+ SHorizontalBox::Slot()
		// .MaxWidth(TrueSectionWidthExperience)
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(ExperienceAsText)
			// .MinDesiredWidth(TrueSectionWidthExperience)
			.MinDesiredWidth(TrueSectionWidth)
		]
		+ SHorizontalBox::Slot()
		// .MaxWidth(TrueSectionWidthValueRep)
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(AppliedValueAsText)
			// .MinDesiredWidth(TrueSectionWidthValueRep)
			.MinDesiredWidth(TrueSectionWidth)
		]
		+ SHorizontalBox::Slot()
		// .MaxWidth(TrueSectionWidthValueOffsets)
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(ModifiersAsText)
			// .MinDesiredWidth(TrueSectionWidthValueOffsets)
			.MinDesiredWidth(TrueSectionWidth)
		]				
	];

	// ...
	// ...
	
	return MutableThis->StatTable.ToSharedRef();	
}

void SPDStatList::OnComponentSelected_AllStatData(TSharedPtr<FPDStatNetDatum> InItem, ESelectInfo::Type InSelectInfo)
{
	FSlateApplication::Get().DismissAllMenus();

	switch (InSelectInfo)
	{
	case ESelectInfo::OnKeyPress:
		break;
	case ESelectInfo::OnNavigation:
		break;
	case ESelectInfo::OnMouseClick:
		break;
	case ESelectInfo::Direct:
		break;
	}
	
	if (OnStatDataChosen.IsBound())
	{
		OnStatDataChosen.Execute(*InItem.Get());
	}			
}

void UPDStatListInnerWidget::OnBindingChanged(const FName& Property)
{
	Super::OnBindingChanged(Property);
}

void UPDStatListInnerWidget::RefreshStatListOnChangedProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FProperty* Property = PropertyChangedEvent.Property;
	if (Property == nullptr) { return; }

	const FName PropertyName = Property->GetFName();
	const bool bDoesPropertyHaveCorrectName =
		PropertyName.IsEqual(GET_MEMBER_NAME_CHECKED(UPDStatListInnerWidget, EditorTestEntries))
		|| PropertyName.IsEqual(GET_MEMBER_NAME_CHECKED(UPDStatListInnerWidget, SectionWidth));
	if (bDoesPropertyHaveCorrectName == false) { return; }
	
	RefreshInnerStatList();
}

void UPDStatListInnerWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	RefreshStatListOnChangedProperty(PropertyChangedEvent);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UPDStatListInnerWidget::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	RefreshStatListOnChangedProperty(PropertyChangedEvent);
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UPDStatListInnerWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

void UPDStatListInnerWidget::RefreshInnerStatList()
{
	DataView.Empty();
	if (InnerStatList.IsValid())
	{
#if WITH_EDITOR
		if (IsDesignTime())
		{
			for (const FPDStatNetDatum& EditorEntry :  EditorTestEntries)
			{
				TSharedRef<FPDStatNetDatum> SharedNetDatum = MakeShared<FPDStatNetDatum>(EditorEntry);
				DataView.Emplace(SharedNetDatum);			
			}
		}
#endif // WITH_EDITOR
		
		InnerStatList->Refresh(INDEX_NONE, DataView, SectionWidth);
	}
}

TSharedRef<SWidget> UPDStatListInnerWidget::RebuildWidget()
{
	if (InnerSlateWrapbox.IsValid() == false)
	{
		InnerSlateWrapbox = SNew(SWrapBox);
	}
	if (InnerStatList.IsValid() == false)
	{
#if WITH_EDITOR
		if (IsDesignTime())
		{
			for (const FPDStatNetDatum& EditorEntry :  EditorTestEntries)
			{
				TSharedRef<FPDStatNetDatum> SharedNetDatum = MakeShared<FPDStatNetDatum>(EditorEntry);
				DataView.Emplace(SharedNetDatum);			
			}
		}
#endif // WITH_EDITOR
		
		InnerStatList =
			SNew(SPDStatList, OwnerID, DataView, SectionWidth);
	}

	InnerSlateWrapbox->ClearChildren();
	SWrapBox::FScopedWidgetSlotArguments WrapboxSlot = InnerSlateWrapbox->AddSlot();
	WrapboxSlot.AttachWidget(InnerStatList.ToSharedRef());
	
	return InnerSlateWrapbox.ToSharedRef();
}

void UPDStatListInnerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	InnerSlateWrapbox.Reset();
	InnerStatList.Reset();
	
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UPDStatListInnerWidget::UpdateOwner(int32 NewOwner)
{
	OwnerID = NewOwner;
}

void UPDStatListUserWidget::NativePreConstruct()
{
	if (InnerStatList != nullptr)
	{
		InnerStatList->UpdateOwner(GetOwnerID());
	}
	
	Super::NativePreConstruct();
}

int32 UPDStatListUserWidget::GetOwnerID()
{
	if (OwnerIDDelegate.IsBound() == false)
	{
		UE_LOG(PDLog_Progression, Warning, TEXT("UPDStatListUserWidget(%s)::GetOwnerID -- Failed -- OwnerIDDelegate is not bound!"), *GetName())

		return 0;
	}
	
	return OwnerIDDelegate.Execute();
}


#undef LOCTEXT_NAMESPACE

/**
Business Source License 1.1

Parameters

Licensor:             Ario Amin (@ Permafrost Development)
Licensed Work:        RTSOpen (Source available on github)
                      The Licensed Work is (c) 2024 Ario Amin (@ Permafrost Development)
Additional Use Grant: You may make free use of the Licensed Work in a commercial product or service provided these three additional conditions as met; 
                      1. Must give attributions to the original author of the Licensed Work, in 'Credits' if that is applicable.
                      2. The Licensed Work must be Compiled before being redistributed.
                      3. The Licensed Work Source may be linked but may not be packaged into the product or service being sold
                      4. Must not be resold or repackaged or redistributed as another product, is only allowed to be used within a commercial or non-commercial game project.
                      5. Teams with yearly budgets larger than 100000 USD must contact the owner for a custom license or buy the framework from a marketplace it has been made available on.

                      "Credits" indicate a scrolling screen with attributions. This is usually in a products end-state

                      "Package" means the collection of files distributed by the Licensor, and derivatives of that collection
                      and/or of those files..   

                      "Source" form means the source code, documentation source, and configuration files for the Package, usually in human-readable format.

                      "Compiled" form means the compiled bytecode, object code, binary, or any other
                      form resulting from mechanical transformation or translation of the Source form.

Change Date:          2028-04-17

Change License:       Apache License, Version 2.0

For information about alternative licensing arrangements for the Software,
please visit: https://permadev.se/

Notice

The Business Source License (this document, or the “License”) is not an Open
Source license. However, the Licensed Work will eventually be made available
under an Open Source License, as stated in this License.

License text copyright (c) 2017 MariaDB Corporation Ab, All Rights Reserved.
“Business Source License” is a trademark of MariaDB Corporation Ab.

-----------------------------------------------------------------------------

Business Source License 1.1

Terms

The Licensor hereby grants you the right to copy, modify, create derivative
works, redistribute, and make non-production use of the Licensed Work. The
Licensor may make an Additional Use Grant, above, permitting limited
production use.

Effective on the Change Date, or the fourth anniversary of the first publicly
available distribution of a specific version of the Licensed Work under this
License, whichever comes first, the Licensor hereby grants you rights under
the terms of the Change License, and the rights granted in the paragraph
above terminate.

If your use of the Licensed Work does not comply with the requirements
currently in effect as described in this License, you must purchase a
commercial license from the Licensor, its affiliated entities, or authorized
resellers, or you must refrain from using the Licensed Work.

All copies of the original and modified Licensed Work, and derivative works
of the Licensed Work, are subject to this License. This License applies
separately for each version of the Licensed Work and the Change Date may vary
for each version of the Licensed Work released by Licensor.

You must conspicuously display this License on each original or modified copy
of the Licensed Work. If you receive the Licensed Work in original or
modified form from a third party, the terms and conditions set forth in this
License apply to your use of that work.

Any use of the Licensed Work in violation of this License will automatically
terminate your rights under this License for the current and all other
versions of the Licensed Work.

This License does not grant you any right in any trademark or logo of
Licensor or its affiliates (provided that you may use a trademark or logo of
Licensor as expressly required by this License).

TO THE EXTENT PERMITTED BY APPLICABLE LAW, THE LICENSED WORK IS PROVIDED ON
AN “AS IS” BASIS. LICENSOR HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS,
EXPRESS OR IMPLIED, INCLUDING (WITHOUT LIMITATION) WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT, AND
TITLE.

MariaDB hereby grants you permission to use this License’s text to license
your works, and to refer to it using the trademark “Business Source License”,
as long as you comply with the Covenants of Licensor below.

Covenants of Licensor

In consideration of the right to use this License’s text and the “Business
Source License” name and trademark, Licensor covenants to MariaDB, and to all
other recipients of the licensed work to be provided by Licensor:

1. To specify as the Change License the GPL Version 2.0 or any later version,
   or a license that is compatible with GPL Version 2.0 or a later version,
   where “compatible” means that software provided under the Change License can
   be included in a program with software provided under GPL Version 2.0 or a
   later version. Licensor may specify additional Change Licenses without
   limitation.

2. To either: (a) specify an additional grant of rights to use that does not
   impose any additional restriction on the right granted in this License, as
   the Additional Use Grant; or (b) insert the text “None”.

3. To specify a Change Date.

4. Not to modify this License in any other way.
 **/

