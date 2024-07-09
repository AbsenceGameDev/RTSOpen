/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "SaveEditor/SRTSOSaveEditor_PlayerInventoryData.h"

#include "Textures/SlateIcon.h"
#include "Framework/Commands/UIAction.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Views/SListView.h"
#include "Styling/AppStyle.h"

// Class picker 
#include "Kismet2/SClassPickerDialog.h"


#include "GameplayTagContainer.h"
#include "Interfaces/PDInteractInterface.h"
#include "SaveEditor/SRTSOTagPicker.h"

#define LOCTEXT_NAMESPACE "SRTSOSaveEditor_PlayerInventoryData"

class SRTSOTagPicker;
typedef SNumericVectorInputBox<int32, UE::Math::TVector<int32>, 3> SNumericV3i;
typedef SNumericVectorInputBox<double, FVector, 3> SNumericV3d;
typedef SNumericEntryBox<int32> SNumericS1i;
typedef SNumericEntryBox<double> SNumericS1d;


//
// SAVE EDITOR MAIN
void SRTSOSaveEditor_PlayerInventoryData::Construct(const FArguments& InArgs, FRTSSaveData* InLinkedData)
{
	LinkedSaveDataCopy = InLinkedData;
	UpdateChildSlot(nullptr);
}

void SRTSOSaveEditor_PlayerInventoryData::UpdateChildSlot(void* OpaqueData)
{
	// Covers representing below fields
	// CopiedSaveData.Inventories;

	if (OpaqueData != nullptr)
	{
		AllUserInventoriesAsSharedTupleArray = static_cast<TArray<TSharedPtr<FUserInventoriesStruct>>*>(OpaqueData);
	}
	
	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SHorizontalBox)
		+ INSET_HORIZONTAL_SLOT(0)
		[
			SNew(SVerticalBox)
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(STextBlock).Text(FText::FromString("PLAYER INVENTORY DATA: "))
			]
			
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(SListView<TSharedPtr<FUserInventoriesStruct>>)
					.ListItemsSource(AllUserInventoriesAsSharedTupleArray)
					.OnGenerateRow( this, &SRTSOSaveEditor_PlayerInventoryData::MakeListViewWidget_InventoryOverviewData )
					.OnSelectionChanged( this, &SRTSOSaveEditor_PlayerInventoryData::OnComponentSelected_InventoryOverviewData )
			]
		]
	];	
}

TSharedRef<ITableRow> SRTSOSaveEditor_PlayerInventoryData::MakeListViewWidget_InventoryOverviewData(TSharedPtr<FUserInventoriesStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor_PlayerInventoryData* MutableThis = const_cast<SRTSOSaveEditor_PlayerInventoryData*>(this);
	check(MutableThis != nullptr)

	FUserInventoriesStruct& SavedInventoryDatum = *InItem.Get();

	const int32 UserID = SavedInventoryDatum.Key; // 
	const TArray<FPDItemNetDatum>& UserItems = SavedInventoryDatum.Value.Items; //

	const SNumericS1i::FOnValueChanged OnInventoryOwningUserIDChanged =
		SNumericS1i::FOnValueChanged::CreateLambda(
			[&](int32 NewUserID)
			{
				// Slow, @todo come back to this
				for (TTuple<int32, FRTSSavedItems>& CurrentInventory : MutableThis->LinkedSaveDataCopy->Inventories)
				{
					if (CurrentInventory.Key != UserID) { continue;}
				
					CurrentInventory.Key = NewUserID;
					SavedInventoryDatum.Key = UserID;
					break;
				}
				
			});

	TArray<TSharedPtr<FPDItemNetDatum>> CurrentInventoriesAsSharedTupleArray;
	for (const FPDItemNetDatum& InventoryItem : UserItems)
	{
		CurrentInventoriesAsSharedTupleArray.Emplace(MakeShared<FPDItemNetDatum>(InventoryItem).ToSharedPtr());
	}	
	
	
	MutableThis->InventoryTable = SNew( STableRow< TSharedPtr<FUserInventoriesStruct> >, OwnerTable )
		[
			SNew(SVerticalBox)
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(STextBlock)
					.Text(FText::FromString("INVENTORY"))
			]
			+ VERTICAL_SEPARATOR(5.0f)


			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("INVENTORY BASE DATA"))
			]	
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SHorizontalBox)
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Owning UserID: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericEntryBox<int32>)
							.Value(UserID)
							.OnValueChanged(OnInventoryOwningUserIDChanged)
					]
				]
			]
			+ VERTICAL_SEPARATOR(5.0f)
			
			
			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("INVENTORY STATE DATA"))
			]				
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Items: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					// Optimize with TSharedPointer array
					SNew(SListView<TSharedPtr<FPDItemNetDatum>>)
						.ListItemsSource( &CurrentInventoriesAsSharedTupleArray)
						.OnGenerateRow( this, &SRTSOSaveEditor_PlayerInventoryData::MakeListViewWidget_ItemData )
						.OnSelectionChanged( this, &SRTSOSaveEditor_PlayerInventoryData::OnComponentSelected_ItemData )
				]
				+ HORIZONTAL_SEPARATOR(5.0f)
			]
			+ VERTICAL_SEPARATOR(5.0f) 
			
		];

	return MutableThis->InventoryTable.ToSharedRef();	
}

void SRTSOSaveEditor_PlayerInventoryData::OnComponentSelected_InventoryOverviewData(TSharedPtr<FUserInventoriesStruct> InItem, ESelectInfo::Type InSelectInfo)
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
	
	if(OnInventoryOverviewDataChosen.IsBound())
	{
		OnInventoryOverviewDataChosen.Execute(*InItem.Get());
	}			
}

TSharedRef<ITableRow> SRTSOSaveEditor_PlayerInventoryData::MakeListViewWidget_ItemData(TSharedPtr<FPDItemNetDatum> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	const SRTSOSaveEditor_PlayerInventoryData* MutableThis = const_cast<SRTSOSaveEditor_PlayerInventoryData*>(this);
	check(MutableThis != nullptr)

	FPDItemNetDatum& SavedItemDatum = *InItem.Get();

	TArray<TSharedPtr<FStacksStruct>> CurrentStacksAsSharedTupleArray;
	for (const TTuple<int32, int32>& SelectedStack : SavedItemDatum.Stacks)
	{
		FStacksStruct StackDatum{SelectedStack.Key, SelectedStack.Value};
		CurrentStacksAsSharedTupleArray.Emplace(MakeShared<FStacksStruct>(StackDatum).ToSharedPtr());
	}

	// Callback delegates
	const FOnClicked OnItemTypeClicked =
		FOnClicked::CreateLambda(
			[&]() -> FReply
			{
				if (TagPicker.IsValid() == false)
				{
					TSharedRef<SRTSOTagPicker> InstancedTagPicker = SNew(SRTSOTagPicker);
					(TSharedPtr<SRTSOTagPicker>)TagPicker = (TSharedPtr<SRTSOTagPicker>)InstancedTagPicker.ToSharedPtr();
				}
		
				TagPicker->SetVisibility(TagPicker->GetVisibility().IsVisible() ? EVisibility::Collapsed : EVisibility::Visible);
				TagPicker->RequestScrollToView(SavedItemDatum.ItemTag);
				
				return FReply::Handled();
			});

	const SNumericEntryBox<int32>::FOnValueChanged OnTotalItemCountChanged = SNumericEntryBox<int32>::FOnValueChanged::CreateLambda(
		[ImmutableItemDatum = SavedItemDatum](const int32 UpdatedTotalCount)
		{
			FPDItemNetDatum& MutableItemDatum = const_cast<FPDItemNetDatum&>(ImmutableItemDatum);
			MutableItemDatum.TotalItemCount = UpdatedTotalCount;
		});

	const SNumericEntryBox<int32>::FOnValueChanged OnLastEditedStackChanged = SNumericEntryBox<int32>::FOnValueChanged::CreateLambda(
		[ImmutableItemDatum = SavedItemDatum](const int32 UpdatedLastStackIndex)
		{
			FPDItemNetDatum& MutableItemDatum = const_cast<FPDItemNetDatum&>(ImmutableItemDatum);
			MutableItemDatum.LastEditedStackIndex = UpdatedLastStackIndex;
		});
	

	const SListView<TSharedPtr<FStacksStruct>>::FOnGenerateRow OnGenerateStacksElement =
		SListView<TSharedPtr<FStacksStruct>>::FOnGenerateRow::CreateLambda(
			[] (TSharedPtr<FStacksStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
	{
		// @todo
		return SNew( STableRow< TSharedPtr<FName> >, OwnerTable );
	});

	const SListView<TSharedPtr<FStacksStruct>>::FOnSelectionChanged OnSelectedStackElement =
	SListView<TSharedPtr<FStacksStruct>>::FOnSelectionChanged::CreateLambda(
	[](TSharedPtr<FStacksStruct> InItem, ESelectInfo::Type InSelectInfo)
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

		// @todo come back to this
		// if(OnItemStackChosen.IsBound())
		// {
		// 	OnItemStackChosen.Execute(*InItem.Get());
		// }		
	});
	
	
	TSharedRef<STableRow<TSharedPtr<FPDItemNetDatum>>> ItemTable = SNew(STableRow<TSharedPtr<FPDItemNetDatum>>, OwnerTable )
	[
		SNew(SVerticalBox)
		+ INSET_VERTICAL_SLOT(0)
		[
			SNew(STextBlock)
				.Text(FText::FromString("ITEM"))
		]
		+ VERTICAL_SEPARATOR(5.0f)
		
		+ INSET_VERTICAL_SLOT(20)
		[
			SNew(STextBlock)
				.Text(FText::FromString("BASE DATA"))
		]	
		+ INSET_VERTICAL_SLOT(40)
		[
			SNew(SHorizontalBox)
			+ INSET_HORIZONTAL_SLOT(0)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Type: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SButton)
						.Text(FText::FromString(SavedItemDatum.ItemTag.ToString()))
						.OnClicked(OnItemTypeClicked)
				]
				+ INSET_HORIZONTAL_SLOT(10)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Total Count: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SNumericEntryBox<int32>)
						.Value(SavedItemDatum.TotalItemCount)
						.OnValueChanged(OnTotalItemCountChanged) 
				]	
				
			]
		]
		+ VERTICAL_SEPARATOR(5.0f)
		
		
		+ INSET_VERTICAL_SLOT(20)
		[
			SNew(STextBlock)
				.Text(FText::FromString("ENTITY STACK DATA"))
		]				
		+ INSET_VERTICAL_SLOT(40)
		[
			SNew(SHorizontalBox)
			+ INSET_HORIZONTAL_SLOT(0)
			[
				SNew(STextBlock)
					.Text(FText::FromString("Last Edited Stack: "))
			]
			+ INSET_HORIZONTAL_SLOT(0)
			[
				SNew(SNumericEntryBox<int32>)
					.Value(SavedItemDatum.LastEditedStackIndex)
					.OnValueChanged(OnLastEditedStackChanged)
			]
			+ HORIZONTAL_SEPARATOR(5.0f)
			
			+ INSET_HORIZONTAL_SLOT(0)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Stacks: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SListView<TSharedPtr<FStacksStruct>>)
						.ListItemsSource(&CurrentStacksAsSharedTupleArray)
						.OnGenerateRow(OnGenerateStacksElement)
						.OnSelectionChanged(OnSelectedStackElement)
				]			
			]					
		]

	];
	
	return ItemTable;
}

void SRTSOSaveEditor_PlayerInventoryData::OnComponentSelected_ItemData(TSharedPtr<FPDItemNetDatum> InItem, ESelectInfo::Type InSelectInfo) const
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
	
	if(OnItemDataChosen.IsBound())
	{
		OnItemDataChosen.Execute(*InItem.Get());
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

