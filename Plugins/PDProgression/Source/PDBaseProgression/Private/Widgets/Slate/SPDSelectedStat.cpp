/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "Widgets/Slate/SPDSelectedStat.h"

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
#include "Widgets/Notifications/SProgressBar.h"

// Class picker 
#include "Components/PDProgressionComponent.h"
#include "Subsystems/PDProgressionSubsystem.h"

#define LOCTEXT_NAMESPACE "SPDStatList"


FText SPDSelectedStat_LevelData::SelectedStatLevelLabel = LOCTEXT("Stat_LevelData_Title", "LEVEL DATA");;
FText SPDSelectedStat_LevelData::Token_SectionTitle = LOCTEXT("Stat_LevelData_TokensToGrant", "TOKENS");;
FText SPDSelectedStat_LevelData::TokenName_ColumnLabel = LOCTEXT("Stat_LevelData_TokensToGrant", "NAME");;
FText SPDSelectedStat_LevelData::TokenCount_ColumnLabel = LOCTEXT("Stat_LevelData_TokensToGrant", "COUNT");;
FText SPDSelectedStat_LevelData::OtherStats_SectionTitle = LOCTEXT("Stat_LevelData_OtherAffected", "OTHER STATS");;
FText SPDSelectedStat_LevelData::OtherStatsAffectedName_ColumnLabel = LOCTEXT("Stat_LevelData_OtherAffected", "NAME");;
FText SPDSelectedStat_LevelData::OtherStatsAffectedValue_ColumnLabel = LOCTEXT("Stat_LevelData_OtherAffected", "VALUE");;

FText SPDSelectedStat_LevelData::TokenEntryLabel = LOCTEXT("Stat_LevelData_TokensToGrant_Entry", "Token: ");;
FText SPDSelectedStat_LevelData::OtherStatsAffectedEntryLabel = LOCTEXT("Stat_LevelData_OtherAffected_Entry", "Affected Stats:");;
FText SPDSelectedStat_LevelData::ExperienceBar_Title = LOCTEXT("Stat_LevelData_ExperienceBar_Title", "EXPERIENCE");;


FText SPDSelectedStat_OffsetData::StatSources_Header_Title = LOCTEXT("StatSource_HeaderRow_Name", "STAT SOURCE LIST");
FText SPDSelectedStat_OffsetData::StatSources_Header_Name = LOCTEXT("StatSource_HeaderRow_Name", "NAME");
FText SPDSelectedStat_OffsetData::StatSources_Header_Category = LOCTEXT("StatSource_HeaderRow_Name", "CATEGORY");
FText SPDSelectedStat_OffsetData::StatSources_Header_AppliedOffset = LOCTEXT("StatSource_HeaderRow_Name", "VALUE OFFSET");
FText SPDSelectedStat_OffsetData::StatSources_Header_Curves = LOCTEXT("StatSource_HeaderRow_Name", "CURVE");


//
// Selected stat - Level data


void SPDSelectedStat_LevelData::Construct(
	const FArguments& InArgs,
	int32 InOwnerID,
	const FGameplayTag& InSelectedStatTag,
	TArray<TSharedPtr<FPDSkillTokenBase>>& TokenArrayRef,
	TArray<TSharedPtr<FPDStatViewAffectedStat>>& AffectedStatsRef)
{
	TArray<TSharedPtr<FPDSkillTokenBase>>*& TokenArrayPtr = HeaderDataViews.Key.DataViewPtr;
	TArray<TSharedPtr<FPDStatViewAffectedStat>>*& AffectedStatsPtr = HeaderDataViews.Value.DataViewPtr;

	OwnerID = InOwnerID;
	SelectedStatTag = InSelectedStatTag;
	TokenArrayPtr = &TokenArrayRef;
	AffectedStatsPtr = &AffectedStatsRef;

	PrepareData();
	UpdateChildSlot();
}

void SPDSelectedStat_LevelData::UpdateChildSlot()
{
	InitializeFonts();

	TArray<TSharedPtr<FPDSkillTokenBase>>*& TokenArrayPtr = HeaderDataViews.Key.DataViewPtr;
	TSharedPtr<SListView<TSharedPtr<FPDSkillTokenBase>>>& TokenArrayListView = HeaderDataViews.Key.ListView;
	TSharedPtr<SHeaderRow>& TokenHeader = HeaderDataViews.Key.Header;
	FPDStatStatics::CreateHeaderRow(TokenHeader, SectionWidth, 2,
		"0", *TokenName_ColumnLabel.ToString(),
		"1", *TokenCount_ColumnLabel.ToString());

	TArray<TSharedPtr<FPDStatViewAffectedStat>>*& AffectedStatsPtr = HeaderDataViews.Value.DataViewPtr;
	TSharedPtr<SListView<TSharedPtr<FPDStatViewAffectedStat>>>& AffectedStatsListView = HeaderDataViews.Value.ListView;
	TSharedPtr<SHeaderRow>& AffectedHeader = HeaderDataViews.Value.Header;	
	FPDStatStatics::CreateHeaderRow(AffectedHeader, SectionWidth, 2,
		"0", *OtherStatsAffectedName_ColumnLabel.ToString(),
		"1", *OtherStatsAffectedValue_ColumnLabel.ToString());	
	
	TokenArrayListView =
		SNew(SListView<TSharedPtr<FPDSkillTokenBase>>)
		.HeaderRow(TokenHeader)
		.ListItemsSource(TokenArrayPtr)
		.OnGenerateRow( this, &SPDSelectedStat_LevelData::MakeListViewWidget_LinkedStat_TokensToGrant)
		.OnSelectionChanged( this, &SPDSelectedStat_LevelData::OnComponentSelected_LinkedStat_TokensToGrant );
	
	AffectedStatsListView =
		SNew(SListView<TSharedPtr<FPDStatViewAffectedStat>>)
		.HeaderRow(AffectedHeader)
		.ListItemsSource(AffectedStatsPtr)
		.OnGenerateRow( this, &SPDSelectedStat_LevelData::MakeListViewWidget_LinkedStat_AffectedStats)
		.OnSelectionChanged( this, &SPDSelectedStat_LevelData::OnComponentSelected_LinkedStat_AffectedStats );

	static FSlateBrush ProgressBarBrush;
	ProgressBarBrush.ImageType = ESlateBrushImageType::NoImage;
	ProgressBarBrush.TintColor = FSlateColor(FLinearColor::Transparent);
	ProgressBarBrush.ImageSize.X = 200.0f;
	ProgressBarBrush.ImageSize.Y = 20.0f;
	static FProgressBarStyle ProgressBarStyle;
	ProgressBarStyle.BackgroundImage = ProgressBarBrush;
	
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(10)
		[
			SetWidgetTitle(SVerticalBox, SelectedStatLevelLabel, TitleFont)
			+ SetTitleSlot(SVerticalBox, Token_SectionTitle, HalfSizedTitleFont)
				+ SetListViewSlot(SVerticalBox, 800, TokenArrayListView.ToSharedRef())
				+ SetListViewSlot(SVerticalBox, 800, AffectedStatsListView.ToSharedRef())
			
			+ SetTitleSlot(SVerticalBox, ExperienceBar_Title, HalfSizedTitleFont)
			+ SVerticalBox::Slot() 
				.Padding(FMargin{0, 0})
				.FillHeight(10)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.ZOrder(9)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					// @inprogress replace with bar representing experience
	
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.MaxWidth(200)
					[
						SNew(SProgressBar)
						.Percent(0.5)
						.Style(&ProgressBarStyle)
					]
				]
				+ SOverlay::Slot()
				.ZOrder(10)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)			
				[
					SNew(STextBlock)
					.Text(ExperienceBar_Title)
					// @inprogress actual text indicating numerical value of experience
				]			
			]		
		]
	];
}

void SPDSelectedStat_LevelData::PrepareData()
{
	TArray<TSharedPtr<FPDSkillTokenBase>>*& TokenArrayPtr = HeaderDataViews.Key.DataViewPtr;
	const TSharedPtr<SListView<TSharedPtr<FPDSkillTokenBase>>>& TokenArrayListView = HeaderDataViews.Key.ListView;
	TArray<TSharedPtr<FPDStatViewAffectedStat>>*& AffectedStatsPtr = HeaderDataViews.Value.DataViewPtr;
	const TSharedPtr<SListView<TSharedPtr<FPDStatViewAffectedStat>>>& AffectedStatsListView = HeaderDataViews.Value.ListView;
	
	if (TokenArrayPtr == nullptr || TokenArrayListView.IsValid() == false) { return; }
	if (AffectedStatsPtr == nullptr || AffectedStatsListView.IsValid() == false) { return; }
	
	static UPDStatSubsystem* StatSubsystem = UPDStatSubsystem::Get();
	const FPDStatsRow& SelectedStat = StatSubsystem->GetStatTypeData(SelectedStatTag);
	const TArray<FGameplayTag>& StatsThatWeAffect = StatSubsystem->StatCrossBehaviourMap.FindRef(SelectedStatTag);

	UPDStatHandler** OwnersStatHandler = StatSubsystem->StatHandlers.Find(OwnerID);
	if (OwnersStatHandler == nullptr || *OwnersStatHandler == nullptr) { return; }

	constexpr int32 DeltaLevel = 1;
	const FPDStatMapping* StatMapping = (*OwnersStatHandler)->LocalStatMappings.Find(SelectedStatTag);
	int32 SelectedStatNextLevel = DeltaLevel + (StatMapping == nullptr
		? 0
		: SelectedStatNextLevel = (*OwnersStatHandler)->StatList.Items[StatMapping->Index].CurrentLevel);

	
	for (const FGameplayTag& StatTargetTag : StatsThatWeAffect)
	{
		const FPDStatsRow* StatTargetDefaultDataPtr = StatSubsystem->GetStatTypeDataPtr(StatTargetTag);
		
		if (StatTargetDefaultDataPtr == nullptr) { continue; }

		// Find out our (SelectedStatTag) effect on target stat (Stats that are affected by us)
		double DeltaNewLevelOffset; 
		StatSubsystem->ResolveCrossBehaviourDelta(
			SelectedStatNextLevel - DeltaLevel,
			SelectedStatNextLevel,
			*StatTargetDefaultDataPtr, 
			SelectedStatTag,
			DeltaNewLevelOffset);
		
		
		TSharedPtr<FPDStatViewAffectedStat> ConstructedAffectedStatView = MakeShared<FPDStatViewAffectedStat>();
		ConstructedAffectedStatView->AffectedStat = StatTargetTag;
		ConstructedAffectedStatView->TotalAffectedDelta = DeltaNewLevelOffset;

		AffectedStatsPtr->Emplace(ConstructedAffectedStatView);
	}
	
	for (const TTuple<FGameplayTag, UCurveFloat*>& TokenCategoryCompound : SelectedStat.TokensToGrantPerLevel)
	{
		const UCurveFloat* TokenProgressCurve = TokenCategoryCompound.Value;
		if (TokenProgressCurve == nullptr)
		{
			UE_LOG(PDLog_Progression, Error,
				   TEXT("UPDStatHandler::GrantTokens "
					   "-- Iterating the token list, entry(%s) has not valid curve applied ot it"), *TokenCategoryCompound.Key.GetTagName().ToString());
			continue;
		}

		TSharedPtr<FPDSkillTokenBase> ConstructedTokenView = MakeShared<FPDSkillTokenBase>();
		ConstructedTokenView->TokenType = TokenCategoryCompound.Key;
		ConstructedTokenView->TokenValue += (TokenProgressCurve->GetFloatValue(SelectedStatNextLevel) + 0.5f);
		
		TokenArrayPtr->Emplace(ConstructedTokenView);
	}
}

TSharedRef<ITableRow> SPDSelectedStat_LevelData::MakeListViewWidget_LinkedStat_TokensToGrant(
	TSharedPtr<FPDSkillTokenBase> StatViewTokensToGrant,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedPtr<STableRow< TSharedPtr<FPDSkillTokenBase>>> StatTable = nullptr;

	// (DONE) 1. StatViewModifySource->StatTag
	const FPDStatDataViewSlotParams& HeaderSlotParam =
		FPDStatStatics::CreateDataViewSlotParam(StatViewTokensToGrant->TokenType, StatViewTokensToGrant->TokenValue);
	
	constexpr double WidthDiscrepancy = (1.015);
	const float TrueSectionWidth = SectionWidth * WidthDiscrepancy;
	StatTable = SNew( STableRow< TSharedPtr<FPDSkillTokenBase> >, OwnerTable )
	[
		SNew(SHorizontalBox)
		+ SetListHeaderSlot(SHorizontalBox, HeaderSlotParam.SectionText, TrueSectionWidth )
		+ SetListHeaderSlot(SHorizontalBox, HeaderSlotParam.ValueText, TrueSectionWidth )
	];

	
	return StatTable.ToSharedRef();
}

void SPDSelectedStat_LevelData::Refresh(
	int32 InOwnerID,
	TArray<TSharedPtr<FPDSkillTokenBase>>& TokenArrayRef,
	TArray<TSharedPtr<FPDStatViewAffectedStat>>& AffectedStatsRef,
	int32 InSectionWidth)
{
	OwnerID = InOwnerID;
	SectionWidth = InSectionWidth;

	TArray<TSharedPtr<FPDSkillTokenBase>>*& TokenArrayPtr = HeaderDataViews.Key.DataViewPtr;
	const TSharedPtr<SListView<TSharedPtr<FPDSkillTokenBase>>>& TokenArrayListView = HeaderDataViews.Key.ListView;
	TArray<TSharedPtr<FPDStatViewAffectedStat>>*& AffectedStatsPtr = HeaderDataViews.Value.DataViewPtr;
	const TSharedPtr<SListView<TSharedPtr<FPDStatViewAffectedStat>>>& AffectedStatsListView = HeaderDataViews.Value.ListView;
	
	TokenArrayPtr = &TokenArrayRef;
	AffectedStatsPtr = &AffectedStatsRef;
	
	PrepareData();
	if (TokenArrayListView.IsValid())
	{
		TokenArrayListView->SetItemsSource(TokenArrayPtr);
		TokenArrayListView->RebuildList();
	}
	if (AffectedStatsListView.IsValid())
	{
		AffectedStatsListView->SetItemsSource(AffectedStatsPtr);
		AffectedStatsListView->RebuildList();
	}

}

void SPDSelectedStat_LevelData::OnComponentSelected_LinkedStat_TokensToGrant(
	TSharedPtr<FPDSkillTokenBase> StatViewTokensToGrant,
	ESelectInfo::Type Arg) const
{
}

TSharedRef<ITableRow> SPDSelectedStat_LevelData::MakeListViewWidget_LinkedStat_AffectedStats(
	TSharedPtr<FPDStatViewAffectedStat> CurrentAffectedStat,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedPtr<STableRow< TSharedPtr<FPDStatViewAffectedStat>>> StatTable = nullptr;

	// (DONE) 1. StatViewModifySource->StatTag
	const FPDStatDataViewSlotParams& HeaderSlotParam =
		FPDStatStatics::CreateDataViewSlotParam(CurrentAffectedStat->AffectedStat, CurrentAffectedStat->TotalAffectedDelta);

	constexpr double WidthDiscrepancy = (1.015);
	const float TrueSectionWidth = SectionWidth * WidthDiscrepancy;
	StatTable = SNew( STableRow< TSharedPtr<FPDStatViewAffectedStat> >, OwnerTable )
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		+ SetListHeaderSlot(SHorizontalBox, HeaderSlotParam.SectionText, TrueSectionWidth )
		+ SetListHeaderSlot(SHorizontalBox, HeaderSlotParam.ValueText, TrueSectionWidth )
	];

	return StatTable.ToSharedRef();
	
}
void SPDSelectedStat_LevelData::OnComponentSelected_LinkedStat_AffectedStats(
	TSharedPtr<FPDStatViewAffectedStat> StatViewAffectedStats,
	ESelectInfo::Type Arg) const
{
	// @todo
}



//
// Selected Stat - Offset modify sources view
void SPDSelectedStat_OffsetData::Construct(const FArguments& InArgs, int32 InOwnerID, const FGameplayTag& InSelectedStatTag, TArray<TSharedPtr<FPDStatViewModifySource>>& ArrayRef)
{
	TArray<TSharedPtr<FPDStatViewModifySource>>*& SelectedStatModifierSources = HeaderDataViews.Value.DataViewPtr;
	OwnerID = InOwnerID;
	
	SelectedStatTag = InSelectedStatTag;
	SelectedStatModifierSources = &ArrayRef;

	PrepareData();
	UpdateChildSlot();
}

void SPDSelectedStat_OffsetData::PrepareData()
{
	TArray<TSharedPtr<FPDStatViewModifySource>>*& SelectedStatModifierSources = HeaderDataViews.Value.DataViewPtr;
	const TSharedPtr<SListView<TSharedPtr<FPDStatViewModifySource>>>& ModifySourceListView = HeaderDataViews.Value.ListView;
	if (SelectedStatModifierSources == nullptr || ModifySourceListView.IsValid() == false)
	{
		return;
	}
	
	static UPDStatSubsystem* StatSubsystem = UPDStatSubsystem::Get();
	const FPDStatsRow& SelectedStat = StatSubsystem->GetStatTypeData(SelectedStatTag);
	const TArray<FGameplayTag>& StatsThatAffectUs = StatSubsystem->StatCrossBehaviourBackMapped.FindRef(SelectedStatTag);

	UPDStatHandler** OwnersStatHandler = StatSubsystem->StatHandlers.Find(OwnerID);
	if (OwnersStatHandler == nullptr || *OwnersStatHandler == nullptr)
	{
		return;
	}
	
	for (const FGameplayTag& StatSourceTag : StatsThatAffectUs)
	{
		FPDStatsCrossBehaviourRules CrossBehaviourRules;

		const FPDStatMapping* StatMapping = (*OwnersStatHandler)->LocalStatMappings.Find(StatSourceTag);
		int32 StatSourceLevel = (StatMapping == nullptr
			? 0
			: StatSourceLevel = (*OwnersStatHandler)->StatList.Items[StatMapping->Index].CurrentLevel);		
		
		double CrossBehaviourOffset_Normalized;
		StatSubsystem->ResolveCrossBehaviours(
			StatSourceLevel,
			SelectedStat, 
			StatSourceTag,
			CrossBehaviourRules,
			CrossBehaviourOffset_Normalized);

		TSharedPtr<FPDStatViewModifySource> ConstructedModifySourceView = MakeShared<FPDStatViewModifySource>();
		ConstructedModifySourceView->StatTag = StatSourceTag;
		ConstructedModifySourceView->AppliedStatOffset = CrossBehaviourOffset_Normalized;
		ConstructedModifySourceView->StatOffsetCurveSource = CrossBehaviourRules.RuleSetLevelCurveMultiplier; 

		// @todo, need to write a shader that visualizes this curve for us, and where on the curve we are at
		
		SelectedStatModifierSources->Emplace(ConstructedModifySourceView);
	}
	
}

void SPDSelectedStat_OffsetData::Refresh(
	int32 InOwnerID,
	TArray<TSharedPtr<FPDStatViewModifySource>>& DataViewRef,
	const int32 NewSectionWidth)
{
	TArray<TSharedPtr<FPDStatViewModifySource>>*& SelectedStatModifierSources = HeaderDataViews.Value.DataViewPtr;
	const TSharedPtr<SListView<TSharedPtr<FPDStatViewModifySource>>>& ModifySourceListView = HeaderDataViews.Value.ListView;
	
	OwnerID = InOwnerID;
	SelectedStatModifierSources = &DataViewRef;
	SectionWidth = NewSectionWidth;
	
	if (ModifySourceListView.IsValid() == false) { return; }

	PrepareData();
	ModifySourceListView->RebuildList();
}

void SPDSelectedStat_OffsetData::UpdateChildSlot()
{
	InitializeFonts();
	TArray<TSharedPtr<FPDStatViewModifySource>>*& SelectedStatModifierSources = HeaderDataViews.Value.DataViewPtr;
	TSharedPtr<SListView<TSharedPtr<FPDStatViewModifySource>>>& ModifySourceListView = HeaderDataViews.Value.ListView;

	TSharedPtr<SHeaderRow>& Header = HeaderDataViews.Value.Header;
	FPDStatStatics::CreateHeaderRow(Header, SectionWidth, 4,
		"0", *StatSources_Header_Name.ToString(),
		"1", *StatSources_Header_Category.ToString(),
		"2", *StatSources_Header_AppliedOffset.ToString(),
		"3", *StatSources_Header_Curves.ToString());	
	
	ModifySourceListView = SNew(SListView<TSharedPtr<FPDStatViewModifySource>>)
		.HeaderRow(Header)
		.ListItemsSource(SelectedStatModifierSources)
		.OnSelectionChanged( this, &SPDSelectedStat_OffsetData::OnComponentSelected_LinkedStat )	
		.OnGenerateRow( this, &SPDSelectedStat_OffsetData::MakeListViewWidget_LinkedStat);
	
	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(10)
		[
			SetWidgetTitle(SVerticalBox, StatSources_Header_Title, TitleFont)
			
			+ SetListViewSlot(SVerticalBox, 800, ModifySourceListView.ToSharedRef())
		]
	];	
	
}

TSharedRef<ITableRow> SPDSelectedStat_OffsetData::MakeListViewWidget_LinkedStat(
	TSharedPtr<FPDStatViewModifySource> StatViewModifySource,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedPtr<STableRow< TSharedPtr<FPDStatViewModifySource>>> StatTable = nullptr;

	const FPDStatDataViewSlotParams& HeaderSlotParam =
		FPDStatStatics::CreateDataViewSlotParam(StatViewModifySource->StatTag, StatViewModifySource->AppliedStatOffset);
	
	// @todo 3. StatViewModifySource->StatOffsetCurveSource;
	const FText PlaceholderCurveString = LOCTEXT("Placeholder93u490803","TODO - IMPLEMENT CURVE DISPLAY/VIEW");
	
	constexpr double WidthDiscrepancy = (1.015);
	const float TrueSectionWidth = SectionWidth * WidthDiscrepancy;
	StatTable =
		SNew( STableRow< TSharedPtr<FPDStatViewModifySource> >, OwnerTable )
		[
			SNew(SHorizontalBox)

			// Name Field			
			+ SHorizontalBox::Slot()

			+ SetListHeaderSlot(SHorizontalBox, HeaderSlotParam.SectionText, TrueSectionWidth )
			+ SetListHeaderSlot(SHorizontalBox, HeaderSlotParam.ValueText, TrueSectionWidth )
			
			// @todo 3. StatViewModifySource->StatOffsetCurveSource;
			// Curve display, Showing the Applied offset			
			+ SetListHeaderSlot(SHorizontalBox, PlaceholderCurveString, TrueSectionWidth )
		];


	return StatTable.ToSharedRef();
}

void SPDSelectedStat_OffsetData::OnComponentSelected_LinkedStat(TSharedPtr<FPDStatViewModifySource> FpdStatViewModifySource,
	ESelectInfo::Type Arg) const
{
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

