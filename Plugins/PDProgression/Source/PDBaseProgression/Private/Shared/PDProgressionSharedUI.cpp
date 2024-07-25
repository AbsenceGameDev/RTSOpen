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

#define LOCTEXT_NAMESPACE "SPDStatList"


FText SPDStatList::StatBase_TitleText = LOCTEXT("HeaderRow_Field", "STATS");
FText SPDStatList::StatProgress_Header_Name = LOCTEXT("Stat_HeaderRow_Field_Level", "NAME");
FText SPDStatList::StatProgress_Header_Level = LOCTEXT("Stat_HeaderRow_Field_Level", "LEVEL");
FText SPDStatList::StatProgress_Header_Experience = LOCTEXT("Stat_HeaderRow_Field_Experience", "EXPERIENCE");
FText SPDStatList::StatProgress_Header_ModifiedOffset = LOCTEXT("Stat_HeaderRow_Field_ModifiedOffset", "OFFSET");

//
// SAVE EDITOR MAIN
void SPDStatList::Construct(const FArguments& InArgs, int32 InOwnerID, TArray<TSharedPtr<FPDStatNetDatum>>& ArrayRef)
{
	UPDStatHandler* SelectedStatHandler = UPDStatSubsystem::Get()->StatHandlers.FindRef(InOwnerID);
	if (SelectedStatHandler != nullptr)
	{
		for (const FPDStatNetDatum& Item : SelectedStatHandler->StatList.Items)
		{
			TSharedRef<FPDStatNetDatum> SharedNetDatum = MakeShared<FPDStatNetDatum>(Item);
			StatsAsSharedArray->Emplace(SharedNetDatum);
		}
	}
	
	UpdateChildSlot(&ArrayRef);
}

void SPDStatList::UpdateChildSlot(void* OpaqueData)
{
	if (TitleFont.TypefaceFontName.IsNone())
	{
		// @todo Set up a custom slate styleset for the saveeditors fonts and icons 
		TitleFont = FAppStyle::GetFontStyle( TEXT("PropertyWindow.NormalFont"));
		TitleFont.Size *= 8;
	}

	
	// Covers representing below fields
	// CopiedSaveData.ConversationActorState;

	if (OpaqueData != nullptr)
	{
		StatsAsSharedArray = static_cast<TArray<TSharedPtr<FPDStatNetDatum>>*>(OpaqueData);
	}

	const TSharedRef<SHeaderRow> Header = SNew(SHeaderRow);
	SHeaderRow::FColumn::FArguments ColumnArgs;
	
	ColumnArgs
		.DefaultLabel(StatProgress_Header_Name)
		.FixedWidth(50)
		.ColumnId(FName("0")) ;
	Header.Get().AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(FText::FromString("|"))
		.FixedWidth(15)
		.ColumnId(FName("1")) ;
	Header.Get().AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(StatProgress_Header_Level)
		.FixedWidth(50)
		.ColumnId(FName("2")) ;
	Header.Get().AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(FText::FromString("|"))
		.FixedWidth(15)
		.ColumnId(FName("3")) ;
	Header.Get().AddColumn(ColumnArgs);	

	ColumnArgs
		.DefaultLabel(StatProgress_Header_Experience)
		.FixedWidth(50)
		.ColumnId(FName("4")) ;
	Header.Get().AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(FText::FromString("|"))
		.FixedWidth(15)
		.ColumnId(FName("5")) ;
	Header.Get().AddColumn(ColumnArgs);	
	
	ColumnArgs
		.DefaultLabel(StatProgress_Header_ModifiedOffset)
		.FixedWidth(50)
		.ColumnId(FName("4")) ;
	Header.Get().AddColumn(ColumnArgs);

	ColumnArgs
		.DefaultLabel(FText::FromString("|"))
		.FixedWidth(15)
		.ColumnId(FName("5")) ;
	Header.Get().AddColumn(ColumnArgs);		
	
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
				[
					SNew(SListView<TSharedPtr<FPDStatNetDatum>>)
						.HeaderRow(Header.ToSharedPtr())
						.ListItemsSource(StatsAsSharedArray)
						.OnGenerateRow( this, &SPDStatList::MakeListViewWidget_AllStatData )
						.OnSelectionChanged( this, &SPDStatList::OnComponentSelected_AllStatData )
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
	const FText StatNameAsText   = FText::FromName(CurrentStatNetDatum.ProgressionTag.GetTagName());
	const FText LevelAsText      = FText::FromString(FString::FromInt(CurrentStatNetDatum.CurrentLevel));
	const FText ExperienceAsText = FText::FromString(FString::FromInt(CurrentStatNetDatum.CurrentExperience));
	const FText ModifiersAsText  = FText::FromString(FString::FromInt(CurrentStatNetDatum.CrossBehaviourValue));
	
	//
	// Widget layout
	SPDStatList* MutableThis = const_cast<SPDStatList*>(this);
	check(MutableThis != nullptr)
	
	MutableThis->StatTable =
		SNew( STableRow< TSharedPtr<FPDStatNetDatum> >, OwnerTable )
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(StatNameAsText)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SSeparator)
			.Orientation(Orient_Vertical)
			.Thickness(10)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(LevelAsText)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SSeparator)
			.Orientation(Orient_Vertical)
			.Thickness(10)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(ExperienceAsText)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SSeparator)
			.Orientation(Orient_Vertical)
			.Thickness(10)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(ModifiersAsText)
		]			
	]
	;

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
	
	if(OnStatDataChosen.IsBound())
	{
		OnStatDataChosen.Execute(*InItem.Get());
	}			
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

