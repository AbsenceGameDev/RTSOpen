/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "Net/PDProgressionNetDatum.h"
#include "Subsystems/EngineSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Layout/Geometry.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

#include "PDProgressionSharedUI.generated.h"

class FPaintArgs;
class FSlateWindowElementList;
struct FSlateBrush;

namespace EPDStatListSections
{
	enum
	{
		EName = 0,
		ECategory,
		ELevel,
		EExperience,
		EValue,
		EValueOffset
	};
}

USTRUCT(Blueprintable)
struct FPDStatViewModifySource
{
	GENERATED_BODY()

	/** @brief Tag of source stat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StatTag{};
	
	/** @brief Offset applied from source stat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double AppliedStatOffset = 0.0;

	/** @brief Offset applied from source stat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* StatOffsetCurveSource = nullptr;		
};

USTRUCT(Blueprintable)
struct FPDStatViewTokenToGrant
{
	GENERATED_BODY()

	/** @brief Tag of source stat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag TokenTag;

	/** @brief @todo  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TokensToGrantForNextLevel;
	
};

USTRUCT(Blueprintable)
struct FPDStatViewAffectedStat
{
	GENERATED_BODY()

	/** @brief Tag of source stat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag AffectedStat;

	/** @brief @todo  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TotalAffectedDelta;
	
};

// @todo (PRIO 1) Need a widget for a button to upgrade a stat and a developer setting to tell if the button should be visible or not, this is be able to cater to different players // Todo also make the inventory system
// @todo Cont. This would need to allow selection of type of token to be used, in case a upgrade allows for different types of tokens

// @todo (PRIO 3/BACKLOG) Need a widget for selecting the Category : Create a view that shows other, unlocked, stats in the same category



/** @brief @todo/in-progress Widget for data regarding the selected stats level and experience
 * @todo Need to have an experience-bar which also may display numbers
 * @details Show amount of tokens, and their types that will be earned upon gaining next level, and any relevant stat changes that will occur */
class PDBASEPROGRESSION_API SPDSelectedStat_LevelData : public  SCompoundWidget
{
public:
	FSlateFontInfo TitleFont;
	FSlateFontInfo HalfSizedTitleFont;

	SLATE_BEGIN_ARGS(SPDSelectedStat_LevelData){}
	SLATE_END_ARGS()

	void Construct(
		const FArguments& InArgs,
		int32 InOwnerID,
		const FGameplayTag& InSelectedStatTag,
		TArray<TSharedPtr<FPDStatViewTokenToGrant>>& TokenArrayRef,
		TArray<TSharedPtr<FPDStatViewAffectedStat>>& AffectedStatsRef);

	TSharedRef<ITableRow> MakeListViewWidget_LinkedStat_TokensToGrant(TSharedPtr<FPDStatViewTokenToGrant> FpdStatViewTokensToGrant, const TSharedRef<STableViewBase>& TableViewBase) const;
	void OnComponentSelected_LinkedStat_TokensToGrant(TSharedPtr<FPDStatViewTokenToGrant> StatViewTokensToGrant, ESelectInfo::Type Arg) const;
	TSharedRef<ITableRow> MakeListViewWidget_LinkedStat_AffectedStats(TSharedPtr<FPDStatViewAffectedStat> StatViewAffectedStats, const TSharedRef<STableViewBase>& TableViewBase) const;
	void OnComponentSelected_LinkedStat_AffectedStats(TSharedPtr<FPDStatViewAffectedStat> FpdStatViewAffectedStats, ESelectInfo::Type Arg) const;
	void UpdateChildSlot();

	void PrepareData();


	int32 OwnerID;
	FGameplayTag SelectedStatTag;
	int32 SectionWidth = 50;

	TSharedPtr<SListView<TSharedPtr<FPDStatViewTokenToGrant>>> TokenArrayListView = nullptr;
	TArray<TSharedPtr<FPDStatViewTokenToGrant>>* TokenArrayPtr;

	TSharedPtr<SListView<TSharedPtr<FPDStatViewAffectedStat>>> AffectedStatsListView = nullptr;
	TArray<TSharedPtr<FPDStatViewAffectedStat>>* AffectedStatsPtr;

	TSharedPtr<SHeaderRow> TokenHeader = nullptr;
	TSharedPtr<SHeaderRow> AffectedHeader = nullptr;

	static FText SelectedStatLevelLabel;
	static FText Token_ColumnLabel;
	static FText TokenName_ColumnLabel;
	static FText TokenCount_ColumnLabel;
	
	static FText OtherStats_ColumnLabel;
	static FText OtherStatsAffectedName_ColumnLabel;
	static FText OtherStatsAffectedValue_ColumnLabel;
	
	static FText TokenEntryLabel;
	static FText OtherStatsAffectedEntryLabel;

	static FText ExperienceBar_Title;

	
};

// @todo A SPDSelectedStat is meant to be displayed when selecting the stats offset value, consider renaming it to be more clear
class PDBASEPROGRESSION_API SPDSelectedStat_OffsetData : public SCompoundWidget
{
public:

	FSlateFontInfo TitleFont;

	SLATE_BEGIN_ARGS(SPDSelectedStat_OffsetData){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, int32 InOwnerID, const FGameplayTag& InSelectedStatTag, TArray<TSharedPtr<FPDStatViewModifySource>>& ArrayRef);

	void PrepareData();
	void Refresh();

	TSharedRef<ITableRow> MakeListViewWidget_LinkedStat(TSharedPtr<FPDStatViewModifySource> StatViewModifySource, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_LinkedStat(TSharedPtr<FPDStatViewModifySource> StatViewModifySource, ESelectInfo::Type Arg) const;
	virtual void UpdateChildSlot();


	int32 SectionWidth = 50;

	int32 OwnerID = 0;	
	FGameplayTag SelectedStatTag{};
	TSharedPtr<SHeaderRow> Header = nullptr;
	TSharedPtr<SListView<TSharedPtr<FPDStatViewModifySource>>> ModifySourceListView = nullptr;
	TArray<TSharedPtr<FPDStatViewModifySource>>* SelectedStatModifierSources;
	
	// Labels
	static FText StatSources_Header_Title;
	
	static FText StatSources_Header_Name;
	static FText StatSources_Header_Category;
	static FText StatSources_Header_AppliedOffset;
	static FText StatSources_Header_Curves;
};

class PDBASEPROGRESSION_API SPDStatList : public SCompoundWidget
{
public:

	/** @brief Font we want to use for titles in teh save editor */
	FSlateFontInfo TitleFont;	

	DECLARE_DELEGATE_OneParam( FOnStatDataChosen, const FPDStatNetDatum&);
	
	SLATE_BEGIN_ARGS(SPDStatList) { }
 		SLATE_EVENT(FOnUserScrolled, OnUserScrolled)
 		SLATE_EVENT(FOnClicked, OnUserClicked)
	SLATE_END_ARGS()
	
	/** @brief Stores a pointer to the copied save data and then Calls UpdateChildSlot, passing ArrayRef as the opaquedata parameter */
	void Construct(const FArguments& InArgs, int32 InOwnerID, TArray<TSharedPtr<FPDStatNetDatum>>& DataViewRef, const int32 InSectionWidth);
	void Refresh(int32 InOwnerID, TArray<TSharedPtr<FPDStatNetDatum>>& DataViewRef, const int32 NewSectionWidth);
	void PrepareData();
	/** @brief Base call, ensures we have a title-font loaded, Sets up the child slot, and passes in the data view array to an slistview wrapped in a scrollbox */
	virtual void UpdateChildSlot();
	
	/** @brief Displays the actual list item for each entry in ConversationStatesAsSharedArray, which in this case is the states in 'FPDStatNetDatum' */
	TSharedRef<ITableRow> MakeListViewWidget_AllStatData(TSharedPtr<FPDStatNetDatum> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_AllStatData(TSharedPtr<FPDStatNetDatum> InItem, ESelectInfo::Type InSelectInfo);
	
	/** @brief Array 'View' that is used to display the data related to this editor widget */
	TArray<TSharedPtr<FPDStatNetDatum>>* StatsAsSharedArray;

	// Callbacks
	FOnStatDataChosen OnStatDataChosen{};

	UClass* SelectedClass = nullptr;

	// View Tables
	TSharedPtr<STableRow< TSharedPtr<FPDStatNetDatum>>> StatTable = nullptr;
	TSharedPtr<SListView<TSharedPtr<FPDStatNetDatum>>> ActualList = nullptr;
	TSharedPtr<SHeaderRow> Header = nullptr;

	
	int32 OwnerID = INDEX_NONE;
	// TArray<int32> SectionWidths = {50, 50, 50, 50, 50, 50};
	int32 SectionWidth = 50;		

	
	// Localized text
	static FText StatBase_TitleText;
	static FText StatProgress_Header_Name;
	static FText StatProgress_Header_Category;
	static FText StatProgress_Header_CurrentValue;
	static FText StatProgress_Header_Level;
	static FText StatProgress_Header_Experience;
	static FText StatProgress_Header_ModifiedOffset;
};

DECLARE_DYNAMIC_DELEGATE_RetVal(int, FOwnerIDDelegate);



UCLASS(Blueprintable)
class PDBASEPROGRESSION_API UPDStatListInnerWidget : public UWidget
{
	GENERATED_BODY()
public:
	
	/* @brief @todo */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/* @brief @todo */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/* @brief @todo */
	virtual void SynchronizeProperties() override;
	/* @brief @todo */
	virtual void OnBindingChanged(const FName& Property) override;
	/* @brief @todo */
	void RefreshStatListOnChangedProperty(FPropertyChangedEvent& PropertyChangedEvent);

	/* @brief @todo */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	/* @brief @todo */
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	
	/* @brief @todo */
	void RefreshInnerStatList();
	
	/* @brief @todo */
	UFUNCTION(BlueprintCallable)
	virtual void UpdateOwner(int32 NewOwner);
	
	/** @brief Wrapbox that wraps our SPDStatList derived widgets */
	/* @brief @todo */
	TSharedPtr<class SWrapBox> InnerSlateWrapbox;
	
	/* @brief @todo */
	/** @brief Base ptr to a SPDStatList widget */
	TSharedPtr<SPDStatList> InnerStatList;

	/* @brief @todo Write Supporting code to actually open this as an interactable window */
	TSharedPtr<SPDSelectedStat_LevelData> SelectedStatLevelData_PopUp;

	/* @brief @todo Write Supporting code to actually open this as an interactable window */
	TSharedPtr<SPDSelectedStat_OffsetData> SelectedStatOffsetData_PopUp;	

	/* @brief @todo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OwnerID = 0;

	/* @brief @todo */
	UPROPERTY()
	FOwnerIDDelegate OwnerIDDelegate;

	/* @brief @todo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPDStatNetDatum> EditorTestEntries{};

	/* @brief @todo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SectionWidth = 50;		
	// TArray<int32> SectionWidths = {50, 50, 50, 50, 50};		

	/* @brief @todo */
	PROPERTY_BINDING_IMPLEMENTATION(int32, OwnerID);
	
	//
	// Dataviews
	
	/* @brief @todo */
	TArray<TSharedPtr<FPDStatNetDatum>> DataView{};

	/* @brief @todo */
	TArray<TSharedPtr<FPDStatViewTokenToGrant>> TokenDataView;
};

/* @brief @todo */
UCLASS(Blueprintable)
class PDBASEPROGRESSION_API UPDStatListUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_RetVal(int, FOwnerIDDelegate)
	
	/* @brief @todo */
	UFUNCTION()
	virtual void NativePreConstruct() override;

	// @todo must set up
	/* @brief @todo */
	UFUNCTION()
	virtual int32 GetOwnerID();

	/* @brief @todo */
	FOwnerIDDelegate OwnerIDDelegate;
	
	/* @brief @todo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UPDStatListInnerWidget* InnerStatList;
};

/** A timeline widget.*/
class PDBASEPROGRESSION_API SStatCurve : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS( SStatCurve )
		: _MinValue( 0.0f )
		, _MaxValue( 1.0f )
		, _FixedLabelSpacing(1.0)
		{}

		/** Min. value on the curve */
		SLATE_ARGUMENT( float, MinValue )

		/** Max. value on the curve */
		SLATE_ARGUMENT( float, MaxValue )

		/** fixed pixel spacing between centers of labels */
		SLATE_ARGUMENT( float, FixedLabelSpacing )

	SLATE_END_ARGS()

	/* @brief @todo */
	void Construct( const FArguments& InArgs );

	// SWidget interface
	virtual int32 OnPaint( const FPaintArgs& Args,  const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	virtual FReply OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	FVector2D ComputeDesiredSize(float) const override;
	// End of SWidget interface

	/* @brief @todo */
	void SetZoom(float InZoom) { Zoom = FMath::Max(InZoom, 1.0f); }

	/* @brief @todo */
	float GetZoom() const { return Zoom; }
	
	/* @brief @todo */
	void SetOffset(float InOffset) { Offset = InOffset; } 

	/* @brief @todo */
	float GetOffset() const { return Offset; }

	/* @brief @todo */
	void SetMinMaxValues(float InMin, float InMax)
	{
		MinValue = InMin;
		MaxValue = InMax;
	}

	/* @brief @todo */
	void GetMinMaxValues(float &InMin, float &InMax)
	{
		InMin = MinValue;
		InMax = MaxValue;
	}

	/* @brief @todo */
	void SetDrawingGeometry(const FGeometry& Geometry) { DrawingGeometry = Geometry; }

	/* @brief @todo */
	FGeometry GetDrawingGeometry() const { return DrawingGeometry; }

private:

	/** Background image to use for the graph bar */
	const FSlateBrush* BackgroundImage = nullptr;

	/** Minimum value on the timeline */
	float MinValue = 0;

	/** Maximum value on the timeline */
	float MaxValue = 1;

	/** fixed pixel spacing between centers of labels */
	float FixedLabelSpacing = 1;

	/** Current zoom of the graph */
	float Zoom = 1;

	/** Current offset of the graph */
	float Offset = 0;

	float DrawingOffsetX = 0;

	FGeometry DrawingGeometry;
};


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
