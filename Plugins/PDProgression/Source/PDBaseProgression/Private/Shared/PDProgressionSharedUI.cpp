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
#include "Fonts/FontMeasure.h"
#include "Widgets/Notifications/SProgressBar.h"

// Class picker 
#include "GameplayTags.h"
#include "Components/PDProgressionComponent.h"
#include "Subsystems/PDProgressionSubsystem.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "SPDStatList"


FText SPDStatList::StatBase_TitleText                 = LOCTEXT("HeaderRow_Field", "STATS");
FText SPDStatList::StatProgress_Header_Name           = LOCTEXT("Stat_HeaderRow_Field_Name", "NAME");
FText SPDStatList::StatProgress_Header_Category       = LOCTEXT("Stat_HeaderRow_Field_Category", "CATEGORY");
FText SPDStatList::StatProgress_Header_CurrentValue   = LOCTEXT("Stat_HeaderRow_Field_Value", "CURR. VALUE");
FText SPDStatList::StatProgress_Header_Level          = LOCTEXT("Stat_HeaderRow_Field_Level", "LEVEL");
FText SPDStatList::StatProgress_Header_Experience     = LOCTEXT("Stat_HeaderRow_Field_Experience", "EXPERIENCE");
FText SPDStatList::StatProgress_Header_ModifiedOffset = LOCTEXT("Stat_HeaderRow_Field_ModifiedOffset", "OFFSET");


FText SPDSelectedStat_LevelData::SelectedStatLevelLabel = LOCTEXT("Stat_LevelData_Title", "LEVEL DATA");;
FText SPDSelectedStat_LevelData::Token_ColumnLabel = LOCTEXT("Stat_LevelData_TokensToGrant", "TOKENS");;
FText SPDSelectedStat_LevelData::TokenName_ColumnLabel = LOCTEXT("Stat_LevelData_TokensToGrant", "NAME");;
FText SPDSelectedStat_LevelData::TokenCount_ColumnLabel = LOCTEXT("Stat_LevelData_TokensToGrant", "COUNT");;
FText SPDSelectedStat_LevelData::OtherStats_ColumnLabel = LOCTEXT("Stat_LevelData_OtherAffected", "OTHER STATS");;
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
	OwnerID = InOwnerID;
	SelectedStatTag = InSelectedStatTag;
	TokenArrayPtr = &TokenArrayRef;
	AffectedStatsPtr = &AffectedStatsRef;

	PrepareData();
	UpdateChildSlot();
}

void SPDSelectedStat_LevelData::UpdateChildSlot()
{
	if (TitleFont.TypefaceFontName.IsNone())
	{
		// @todo Set up a custom slate styleset for the saveeditors fonts and icons 
		TitleFont = FAppStyle::GetFontStyle( TEXT("PropertyWindow.NormalFont"));
		TitleFont.Size *= 8;
		HalfSizedTitleFont = TitleFont;
		HalfSizedTitleFont.Size /= 2;
	}
	
	if (TokenHeader.IsValid() == false)
	{
		TokenHeader = SNew(SHeaderRow);
	}
	TokenHeader->ClearColumns();
	
	SHeaderRow::FColumn::FArguments ColumnArguments =
		SHeaderRow::FColumn::FArguments{}
		.DefaultLabel(TokenName_ColumnLabel)
		.FixedWidth(SectionWidth)
		.ColumnId("0");
	TokenHeader->AddColumn(ColumnArguments);
	
	ColumnArguments =
		SHeaderRow::FColumn::FArguments{}
	.DefaultLabel(TokenCount_ColumnLabel)
	.FixedWidth(SectionWidth)
	.ColumnId("1");
	TokenHeader->AddColumn(ColumnArguments);	

	if (AffectedHeader.IsValid() == false)
	{
		AffectedHeader = SNew(SHeaderRow);
	}
	AffectedHeader->ClearColumns();

	ColumnArguments =
		SHeaderRow::FColumn::FArguments{}
	.DefaultLabel(OtherStatsAffectedName_ColumnLabel)
	.FixedWidth(SectionWidth)
	.ColumnId("0");
	AffectedHeader->AddColumn(ColumnArguments);
	
	ColumnArguments =
		SHeaderRow::FColumn::FArguments{}
	.DefaultLabel(OtherStatsAffectedValue_ColumnLabel)
	.FixedWidth(SectionWidth)
	.ColumnId("1");
	AffectedHeader->AddColumn(ColumnArguments);
	
	

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
		SNew(SVerticalBox)
		+ SVerticalBox::Slot() 
			.Padding(FMargin{0, 0})
			.AutoHeight()
		[
			SNew(STextBlock)
				.Font(TitleFont)
				.Justification(ETextJustify::Center)
				.Text(SelectedStatLevelLabel)
		]

		+ SVerticalBox::Slot() 
			.Padding(FMargin{0, 0})
			.AutoHeight()
		[
			SNew(STextBlock)
				.Font(HalfSizedTitleFont)
				.Justification(ETextJustify::Center)
				.Text(Token_ColumnLabel)
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
				TokenArrayListView.ToSharedRef()
			]
		]

		+ SVerticalBox::Slot() 
			.Padding(FMargin{0, 0})
			.AutoHeight()
		[
			SNew(STextBlock)
				.Font(HalfSizedTitleFont)
				.Justification(ETextJustify::Center)
				.Text(OtherStats_ColumnLabel)
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
				AffectedStatsListView.ToSharedRef()
			]
		]

		+ SVerticalBox::Slot() 
			.Padding(FMargin{0, 0})
			.AutoHeight()
		[
			SNew(STextBlock)
				.Font(HalfSizedTitleFont)
				.Justification(ETextJustify::Center)
				.Text(ExperienceBar_Title)
		]			
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
				// @inprogress replace with bar representing experience
				SNew(STextBlock)
				.Text(ExperienceBar_Title)
			]			
		]		
	]
	];
}

void SPDSelectedStat_LevelData::PrepareData()
{
	if (TokenArrayPtr == nullptr || TokenArrayListView.IsValid() == false)
	{
		return;
	}
	
	if (AffectedStatsPtr == nullptr || AffectedStatsListView.IsValid() == false)
	{
		return;
	}
	
	static UPDStatSubsystem* StatSubsystem = UPDStatSubsystem::Get();
	const FPDStatsRow& SelectedStat = StatSubsystem->GetStatTypeData(SelectedStatTag);
	const TArray<FGameplayTag>& StatsThatWeAffect = StatSubsystem->StatCrossBehaviourMap.FindRef(SelectedStatTag);

	UPDStatHandler** OwnersStatHandler = StatSubsystem->StatHandlers.Find(OwnerID);
	if (OwnersStatHandler == nullptr || *OwnersStatHandler == nullptr)
	{
		return;
	}

	const FPDStatMapping* StatMapping = (*OwnersStatHandler)->LocalStatMappings.Find(SelectedStatTag);
	int32 SelectedStatNextLevel = 1 + (StatMapping == nullptr
		? 0
		: SelectedStatNextLevel = (*OwnersStatHandler)->StatList.Items[StatMapping->Index].CurrentLevel);

	
	for (const FGameplayTag& StatTargetTag : StatsThatWeAffect)
	{
		const FPDStatsRow* StatTargetDefaultDataPtr = StatSubsystem->GetStatTypeDataPtr(StatTargetTag);
		
		if (StatTargetDefaultDataPtr == nullptr) { continue; }

		// Find out our (SelectedStatTag) effect on target stat (Stats that are affected by us)
		float CrossBehaviourMultiplier = 1.0;
		float NextCrossBehaviourMultiplier = 1.0;
		const FPDStatsCrossBehaviourRules& CrossBehaviourRules = StatTargetDefaultDataPtr->RulesAffectedBy.FindRef(SelectedStatTag);
		if (CrossBehaviourRules.RuleSetLevelCurveMultiplier != nullptr)
		{
			CrossBehaviourMultiplier = 
				CrossBehaviourRules.RuleSetLevelCurveMultiplier->GetFloatValue(SelectedStatNextLevel - 1);
			NextCrossBehaviourMultiplier = 
				CrossBehaviourRules.RuleSetLevelCurveMultiplier->GetFloatValue(SelectedStatNextLevel);
		}

		const int32 StatTargetBaseDivisor = StatTargetDefaultDataPtr->Representation.BaseDivisor;
		const int32 CurrentCrossBehaviourOffset_NotNormalized = CrossBehaviourRules.CrossBehaviourBaseValue * CrossBehaviourMultiplier;
		const double CurrentCrossBehaviourOffset_Normalized =
			static_cast<double>(CurrentCrossBehaviourOffset_NotNormalized) / static_cast<double>(StatTargetBaseDivisor);
		const int32 NextCrossBehaviourOffset_NotNormalized = CrossBehaviourRules.CrossBehaviourBaseValue * NextCrossBehaviourMultiplier;
		const double NextCrossBehaviourOffset_Normalized =
			static_cast<double>(NextCrossBehaviourOffset_NotNormalized) / static_cast<double>(StatTargetBaseDivisor);
		
		const double DeltaNewLevelOffset = NextCrossBehaviourOffset_Normalized - CurrentCrossBehaviourOffset_Normalized; 

		
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
	const FGameplayTag& TokenTag = StatViewTokensToGrant->TokenType;
	const FString& ParentTagString = TokenTag.RequestDirectParent().GetTagName().ToString();
	const int32 ParentCutoffIndex = 1 + ParentTagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagCategoryString = ParentTagString.RightChop(ParentCutoffIndex);

	const FString& TagString = TokenTag.GetTagName().ToString();
	const int32 CutoffIndex = 1 + TagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagStatString = TagString.RightChop(CutoffIndex);

	const FText TokenAndCategoryText = FText::FromString(TagCategoryString + "." + TagStatString );
	
	constexpr double WidthDiscrepancy = (1.015);
	const float TrueSectionWidth = SectionWidth * WidthDiscrepancy;
	StatTable = SNew( STableRow< TSharedPtr<FPDSkillTokenBase> >, OwnerTable )
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(TokenAndCategoryText)
			.MinDesiredWidth(TrueSectionWidth)
		]
		+ SHorizontalBox::Slot()
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::FromInt(StatViewTokensToGrant->TokenValue)))
			.MinDesiredWidth(TrueSectionWidth)
		]
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
	const FGameplayTag& AffectedStatTag = CurrentAffectedStat->AffectedStat;
	const FString& ParentTagString = AffectedStatTag.RequestDirectParent().GetTagName().ToString();
	const int32 ParentCutoffIndex = 1 + ParentTagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagCategoryString = ParentTagString.RightChop(ParentCutoffIndex);

	const FString& TagString = AffectedStatTag.GetTagName().ToString();
	const int32 CutoffIndex = 1 + TagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagStatString = TagString.RightChop(CutoffIndex);

	const FText AffectedStatAndCategoryText = FText::FromString(TagCategoryString + "." + TagStatString );

	double AbsoluteTotalAffectedDeltaPercentage = FMath::Abs(CurrentAffectedStat->TotalAffectedDelta);
	FString Sign = CurrentAffectedStat->TotalAffectedDelta > 0.0 ? "+" : "-";
	
	constexpr double WidthDiscrepancy = (1.015);
	const float TrueSectionWidth = SectionWidth * WidthDiscrepancy;
	StatTable = SNew( STableRow< TSharedPtr<FPDStatViewAffectedStat> >, OwnerTable )
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(AffectedStatAndCategoryText)
			.MinDesiredWidth(TrueSectionWidth)
		]
		+ SHorizontalBox::Slot()
		.MaxWidth(TrueSectionWidth)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Sign + FString::FromInt(AbsoluteTotalAffectedDeltaPercentage)))
			.MinDesiredWidth(TrueSectionWidth)
		]
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
	OwnerID = InOwnerID;
	
	SelectedStatTag = InSelectedStatTag;
	SelectedStatModifierSources = &ArrayRef;

	PrepareData();
	UpdateChildSlot();
}

void SPDSelectedStat_OffsetData::PrepareData()
{
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
		const FPDStatsRow* StatSourceDefaultDataPtr = StatSubsystem->GetStatTypeDataPtr(StatSourceTag);

		FPDStatMapping* StatMapping = (*OwnersStatHandler)->LocalStatMappings.Find(StatSourceTag);
		if (StatSourceDefaultDataPtr == nullptr || StatMapping == nullptr) { continue; }

		const FPDStatNetDatum& StatSourceActiveDatum = (*OwnersStatHandler)->StatList.Items[StatMapping->Index];

		const FPDStatsCrossBehaviourRules& CrossBehaviourRules = SelectedStat.RulesAffectedBy.FindRef(StatSourceTag);

		float CrossBehaviourMultiplier = 1.0;
		if (CrossBehaviourRules.RuleSetLevelCurveMultiplier != nullptr)
		{
			CrossBehaviourMultiplier = 
				CrossBehaviourRules.RuleSetLevelCurveMultiplier->GetFloatValue(StatSourceActiveDatum.CurrentLevel);
		}

		const int32 StatSourceBaseDivisor = StatSourceDefaultDataPtr->Representation.BaseDivisor;
		const int32 CrossBehaviourOffset_NotNormalized = CrossBehaviourRules.CrossBehaviourBaseValue * CrossBehaviourMultiplier;
		const double CrossBehaviourOffset_Normalized =
			static_cast<double>(CrossBehaviourOffset_NotNormalized) / static_cast<double>(StatSourceBaseDivisor);

		
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
	OwnerID = InOwnerID;
	SelectedStatModifierSources = &DataViewRef;
	SectionWidth = NewSectionWidth;
	
	if (ModifySourceListView.IsValid() == false) { return; }

	PrepareData();
	ModifySourceListView->RebuildList();
}

void SPDSelectedStat_OffsetData::UpdateChildSlot()
{
	if (Header.IsValid() == false)
	{
		Header = SNew(SHeaderRow);
	}
	Header->ClearColumns();
	
	SHeaderRow::FColumn::FArguments ColumnArgs;
	ColumnArgs
		.DefaultLabel(StatSources_Header_Name)
		.FixedWidth(SectionWidth)
		.ColumnId(FName("0")) ;
	Header->AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(StatSources_Header_Category)
		.FixedWidth(SectionWidth)
		.ColumnId(FName("1")) ;
	Header->AddColumn(ColumnArgs);
	
	
	ColumnArgs
		.DefaultLabel(StatSources_Header_AppliedOffset)
		.FixedWidth(SectionWidth)
		.ColumnId(FName("2")) ;
	Header->AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(StatSources_Header_Curves)
		.FixedWidth(SectionWidth)
		.ColumnId(FName("3")) ;
	Header->AddColumn(ColumnArgs);

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
			SNew(SVerticalBox)
			+ SVerticalBox::Slot() 
				.Padding(FMargin{0, 0})
				.AutoHeight()
			[
				SNew(STextBlock)
					.Font(TitleFont)
					.Text(StatSources_Header_Title)
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
					ModifySourceListView.ToSharedRef()
				]
			]
		]
	];	
	
}

TSharedRef<ITableRow> SPDSelectedStat_OffsetData::MakeListViewWidget_LinkedStat(
	TSharedPtr<FPDStatViewModifySource> StatViewModifySource,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedPtr<STableRow< TSharedPtr<FPDStatViewModifySource>>> StatTable = nullptr;

	// @done 1. StatViewModifySource->StatTag
	const FGameplayTag& SourceStatTag = StatViewModifySource->StatTag;
	const FString& ParentTagString = SourceStatTag.RequestDirectParent().GetTagName().ToString();
	const int32 ParentCutoffIndex = 1 + ParentTagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagCategoryString = ParentTagString.RightChop(ParentCutoffIndex);

	const FString& TagString = SourceStatTag.GetTagName().ToString();
	const int32 CutoffIndex = 1 + TagString.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString TagStatString = TagString.RightChop(CutoffIndex);

	const FText StatNameAsText     = FText::FromString(TagStatString);
	const FText StatCategoryAsText = FText::FromString(TagCategoryString);
	const FText ModifySourceTagAndCategoryText = FText::FromString(TagCategoryString + "." + TagStatString );

	
	// @done 2. StatViewModifySource->AppliedStatOffset;
	double AppliedStatOffset = StatViewModifySource->AppliedStatOffset;
	double AbsoluteAppliedStatOffset = FMath::Abs(AppliedStatOffset);
	FString Sign = AppliedStatOffset > 0.0 ? "+" : "-";

	
	// @todo 3. StatViewModifySource->StatOffsetCurveSource;
	FString PlaceholderCurveString = "TODO - IMPLEMENT CURVE DISPLAY/VIEW";
	
	
	constexpr double WidthDiscrepancy = (1.015);
	const float TrueSectionWidth = SectionWidth * WidthDiscrepancy;
	StatTable =
		SNew( STableRow< TSharedPtr<FPDStatViewModifySource> >, OwnerTable )
		[
			SNew(SHorizontalBox)

			// Name Field			
			+ SHorizontalBox::Slot()
			.MaxWidth(TrueSectionWidth)
			[
				SNew(STextBlock)
				.Text(ModifySourceTagAndCategoryText)
				.MinDesiredWidth(TrueSectionWidth)
			]

			// Applied Stat Offset Field			
			+ SHorizontalBox::Slot()
			.MaxWidth(TrueSectionWidth)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Sign + FString::FromInt(AbsoluteAppliedStatOffset)))
				.MinDesiredWidth(TrueSectionWidth)
			]


			// @todo 3. StatViewModifySource->StatOffsetCurveSource;
			// Curve display, Showing the Applied offset			
			+ SHorizontalBox::Slot()
			.MaxWidth(TrueSectionWidth)
			[
				SNew(STextBlock)
				.Text(FText::FromString(PlaceholderCurveString))
				.MinDesiredWidth(TrueSectionWidth)
			]
		];


	return StatTable.ToSharedRef();
}

void SPDSelectedStat_OffsetData::OnComponentSelected_LinkedStat(TSharedPtr<FPDStatViewModifySource> FpdStatViewModifySource,
	ESelectInfo::Type Arg) const
{
}

//
// STAT-LIST MAIN
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
		PropertyName.IsEqual(GET_MEMBER_NAME_CHECKED(UPDStatListInnerWidget, EditorTestEntries_BaseList))
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
	NetDataView.Empty();
	if (InnerStatList.IsValid())
	{
#if WITH_EDITOR
		if (IsDesignTime())
		{
			for (const FPDStatNetDatum& EditorEntry :  EditorTestEntries_BaseList)
			{
				TSharedRef<FPDStatNetDatum> SharedNetDatum = MakeShared<FPDStatNetDatum>(EditorEntry);
				NetDataView.Emplace(SharedNetDatum);			
			}
		}
#endif // WITH_EDITOR
		
		InnerStatList->Refresh(INDEX_NONE, NetDataView, SectionWidth);
	}
}

void UPDStatListInnerWidget::RefreshStatLevel_Popup()
{
	TokenDataView.Empty();
	AffectedStatsDataView.Empty();
	if (SelectedStatLevelData_PopUp.IsValid())
	{
#if WITH_EDITOR
		if (IsDesignTime())
		{
			for (const FPDSkillTokenBase& EditorEntry :  EditorTestEntries_LevelPopup_TokenData)
			{
				TSharedRef<FPDSkillTokenBase> SharedDatum = MakeShared<FPDSkillTokenBase>(EditorEntry);
				TokenDataView.Emplace(SharedDatum);			
			}

			for (const FPDStatViewAffectedStat& EditorEntry :  EditorTestEntries_LevelPopup_AffectedStatsData)
			{
				TSharedRef<FPDStatViewAffectedStat> SharedDatum = MakeShared<FPDStatViewAffectedStat>(EditorEntry);
				AffectedStatsDataView.Emplace(SharedDatum);			
			}			
		}
#endif // WITH_EDITOR
		
		SelectedStatLevelData_PopUp->Refresh(INDEX_NONE, TokenDataView, AffectedStatsDataView, SectionWidth);
	}
}

void UPDStatListInnerWidget::RefreshStatOffset_Popup()
{
	ModifyingSourcesDataView.Empty();
	if (SelectedStatOffsetData_PopUp.IsValid())
	{
#if WITH_EDITOR
		if (IsDesignTime())
		{
			for (const FPDStatViewModifySource& EditorEntry :  EditorTestEntries_OffsetPopup_ModifySources)
			{
				TSharedRef<FPDStatViewModifySource> SharedDatum = MakeShared<FPDStatViewModifySource>(EditorEntry);
				ModifyingSourcesDataView.Emplace(SharedDatum);			
			}
		}
#endif // WITH_EDITOR
		
		SelectedStatOffsetData_PopUp->Refresh(INDEX_NONE, ModifyingSourcesDataView, SectionWidth);
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
			for (const FPDStatNetDatum& EditorEntry :  EditorTestEntries_BaseList)
			{
				TSharedRef<FPDStatNetDatum> SharedNetDatum = MakeShared<FPDStatNetDatum>(EditorEntry);
				NetDataView.Emplace(SharedNetDatum);			
			}
		}
#endif // WITH_EDITOR
		
		InnerStatList =
			SNew(SPDStatList, OwnerID, NetDataView, SectionWidth);
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


void SStatCurve::Construct( const FArguments& InArgs )
{
	MinValue = InArgs._MinValue;
	MaxValue = InArgs._MaxValue;
	FixedLabelSpacing = InArgs._FixedLabelSpacing;

	// FAppStyle::GetBrush( TEXT("PropertyWindow.Background"));
	BackgroundImage = FAppStyle::GetBrush( TEXT("ProfileVisualizer.Background"));


	Zoom = 1.0f;
	Offset = 0.0f;
}

int32 SStatCurve::OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	// Used to track the layer ID we will return.
	int32 RetLayerId = LayerId;

	
	const TSharedRef< FSlateFontMeasure > FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	

	// Paint inside the border only. 
	const FVector2D BorderPadding = FAppStyle::GetVector( TEXT("ProfileVisualizer.ProgressBar.BorderPadding"));

	const float OffsetX = DrawingOffsetX; // BorderPadding.X
	const float Width = DrawingGeometry.Size.X; // AllottedGeometry.Size.X - - 2.0f * BorderPadding.X

	FSlateFontInfo MyFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);

	// Create line points
	const float ValueScale = MaxValue - MinValue;
	const int32 NumValues = FMath::FloorToInt( AllottedGeometry.Size.X * Zoom / FixedLabelSpacing );

	TArray< FVector2D > LinePoints;
	LinePoints.AddUninitialized( 2 );

	// Scoped
	{
		LinePoints[0] = FVector2D( OffsetX, BorderPadding.Y + 1.0f );
		LinePoints[1] = FVector2D( OffsetX + Width, BorderPadding.Y + 1.0f );

		// Draw lines
		FSlateDrawElement::MakeLines( 
			OutDrawElements,
			RetLayerId++,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			FLinearColor::White
			);
	}

	const FVector2D TextDrawSize = FontMeasureService->Measure(TEXT("0.00"), MyFont);
	const float LineHeight = AllottedGeometry.Size.Y - BorderPadding.Y - TextDrawSize.Y - 2.0f;
	
	for( int32 LineIndex = 0; LineIndex <= NumValues; LineIndex++ )
	{
		const float NormalizedX = (float)LineIndex / NumValues;
		const float LineX = Offset + NormalizedX * Zoom;
		if( LineX < 0.0f || LineX > 1.0f )
		{
			continue;
		}

		const float LineXPos =  OffsetX + Width * LineX;
		LinePoints[0] = FVector2D( LineXPos, BorderPadding.Y );
		LinePoints[1] = FVector2D( LineXPos, LineHeight );

		// Draw lines
		FSlateDrawElement::MakeLines( 
			OutDrawElements,
			RetLayerId++,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			FLinearColor::White
			);

		FString ValueText( FString::Printf( TEXT("%.2f"), MinValue + NormalizedX * ValueScale ) );
		FVector2D DrawSize = FontMeasureService->Measure(ValueText, MyFont);
		FVector2D TextPos( LineXPos - DrawSize.X * 0.5f, LineHeight );

		if( TextPos.X < 0.0f )
		{
			TextPos.X = 0.0f;
		}
		else if( (TextPos.X + DrawSize.X) > AllottedGeometry.Size.X )
		{
			TextPos.X = OffsetX + Width - DrawSize.X;
		}

		FSlateDrawElement::MakeText( 
			OutDrawElements,
			RetLayerId,
			AllottedGeometry.ToOffsetPaintGeometry( TextPos ),
			ValueText,
			MyFont,
			ESlateDrawEffect::None,
			FLinearColor::White
			);
	}

	// Always draw lines at the start and at the end of the timeline
	{
		LinePoints[0] = FVector2D( OffsetX, BorderPadding.Y );
		LinePoints[1] = FVector2D( OffsetX, LineHeight );

		// Draw lines
		FSlateDrawElement::MakeLines( 
			OutDrawElements,
			RetLayerId++,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			FLinearColor::White
			);
	}

	{
		LinePoints[0] = FVector2D( OffsetX + Width, BorderPadding.Y );
		LinePoints[1] = FVector2D( OffsetX + Width, LineHeight );

		// Draw lines
		FSlateDrawElement::MakeLines( 
			OutDrawElements,
			RetLayerId++,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			FLinearColor::White
			);
	}

	return RetLayerId - 1;
}

void SStatCurve::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	DrawingOffsetX = DrawingGeometry.AbsolutePosition.X - AllottedGeometry.AbsolutePosition.X;
}

FReply SStatCurve::OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	return SCompoundWidget::OnMouseButtonDown( MyGeometry, MouseEvent );
}

FReply SStatCurve::OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	return SCompoundWidget::OnMouseMove( MyGeometry, MouseEvent );
}

FVector2D SStatCurve::ComputeDesiredSize( float ) const
{
	return FVector2D( 8.0f, 8.0f );
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

