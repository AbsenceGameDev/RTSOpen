/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "SaveEditor/SRTSOSaveEditor_EntityData.h"
#include "SaveEditor/SRTSOSaveEditor.h"

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

#define LOCTEXT_NAMESPACE "SRTSOSaveEditor_EntityData"

typedef SNumericVectorInputBox<int32, UE::Math::TVector<int32>, 3> SNumericV3i;
typedef SNumericVectorInputBox<double, FVector, 3> SNumericV3d;
typedef SNumericEntryBox<int32> SNumericS1i;
typedef SNumericEntryBox<double> SNumericS1d;


//
// SAVE EDITOR MAIN
void SRTSOSaveEditor_EntityData::Construct(const FArguments& InArgs, FRTSSaveData* InLinkedData)
{
	LinkedSaveDataCopy = InLinkedData;
	UpdateChildSlot(nullptr);
}

void SRTSOSaveEditor_EntityData::UpdateChildSlot(void* OpaqueData)
{
	// Covers representing below fields
	// CopiedSaveData.EntityUnits;

	if (OpaqueData != nullptr)
	{
		EntitiesAsSharedArray = static_cast<TArray<TSharedPtr<FRTSSavedWorldUnits>>*>(OpaqueData);
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
				SNew(STextBlock).Text(FText::FromString("ENTITY DATA: "))
			]
			
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(SListView<TSharedPtr<FRTSSavedWorldUnits>>)
					.ListItemsSource(EntitiesAsSharedArray)
					.OnGenerateRow( this, &SRTSOSaveEditor_EntityData::MakeListViewWidget_EntityData )
					.OnSelectionChanged( this, &SRTSOSaveEditor_EntityData::OnComponentSelected_EntityData )
			]			
		]
	];	
}

TSharedRef<ITableRow> SRTSOSaveEditor_EntityData::MakeListViewWidget_EntityData(TSharedPtr<FRTSSavedWorldUnits> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(this);
	check(MutableThis != nullptr)

	const FRTSSavedWorldUnits& SavedEntityDatum = *InItem.Get();

	// ENTITY BASE DATA
	// @done SavedEntityDatum.InstanceIndex;
	// @done SavedEntityDatum.EntityUnitTag;

	// ENTITY STATE DATA
	// @done SavedEntityDatum.Location;
	// @done SavedEntityDatum.Health;
	
	// ENTITY OWNER DATA
	// @done SavedEntityDatum.OwnerID;
	// @done SavedEntityDatum.SelectionIndex;
	
	// ENTITY ACTION DATA
	// @done SavedEntityDatum.CurrentAction;

	// @todo IMPLEMENT CALLBACKS
	const FOnClicked OnEntityTypeClicked =
		FOnClicked::CreateLambda(
			[&]() -> FReply
			{
				// Create the window to pick the class
				if (TagPicker.IsValid() == false)
				{
					TSharedRef<SRTSOTagPicker> InstancedTagPicker = SNew(SRTSOTagPicker);
					(TSharedPtr<SRTSOTagPicker>)TagPicker = (TSharedPtr<SRTSOTagPicker>)InstancedTagPicker.ToSharedPtr();
				}
		
				TagPicker->SetVisibility(TagPicker->GetVisibility().IsVisible() ? EVisibility::Collapsed : EVisibility::Visible);
				TagPicker->RequestScrollToView(SavedEntityDatum.EntityUnitTag);
				
				return FReply::Handled();
			});

	const FOnClicked OnEntityActionTagChanged =
		FOnClicked::CreateLambda(
			[&]() -> FReply
			{
				// Create the window to pick the class
				if (TagPicker.IsValid() == false)
				{
					TSharedRef<SRTSOTagPicker> InstancedTagPicker = SNew(SRTSOTagPicker);
					(TSharedPtr<SRTSOTagPicker>)TagPicker = (TSharedPtr<SRTSOTagPicker>)InstancedTagPicker.ToSharedPtr();
				}
		
				TagPicker->SetVisibility(TagPicker->GetVisibility().IsVisible() ? EVisibility::Collapsed : EVisibility::Visible);
				TagPicker->RequestScrollToView(SavedEntityDatum.CurrentAction.ActionTag);
				
				return FReply::Handled();
			});

	const FOnClicked OnOptRewardItemTypeChanged =
		FOnClicked::CreateLambda(
			[&]() -> FReply
			{
				// Create the window to pick the class
				if (TagPicker.IsValid() == false)
				{
					TSharedRef<SRTSOTagPicker> InstancedTagPicker = SNew(SRTSOTagPicker);
					(TSharedPtr<SRTSOTagPicker>)TagPicker = (TSharedPtr<SRTSOTagPicker>)InstancedTagPicker.ToSharedPtr();
				}
		
				TagPicker->SetVisibility(TagPicker->GetVisibility().IsVisible() ? EVisibility::Collapsed : EVisibility::Visible);
				TagPicker->RequestScrollToView(SavedEntityDatum.CurrentAction.Reward);
				
				return FReply::Handled();
			});		
	

	SNumericV3d::FOnVectorValueChanged OnEntityLocationChanged;
	OnEntityLocationChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](const FVector& UpdatedVector)
		{
			SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			
			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableSaveData->EntityUnits)
			{
				if (CurrentUnit.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentUnit.Location = UpdatedVector;
				break;
			}
		});

	
	SNumericS1d::FOnValueChanged OnEntityHealthChanged;
	OnEntityHealthChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](double UpdatedValue)
		{
			const SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableSaveData->EntityUnits)
			{
				if (CurrentInteractable.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentInteractable.Health = UpdatedValue;
				break;
			}
		});

	
	SNumericS1i::FOnValueChanged OnEntityOwnerIDChanged;
	OnEntityOwnerIDChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](int32 NewOwnerID)
		{
			const SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableSaveData->EntityUnits)
			{
				if (CurrentInteractable.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentInteractable.OwnerID = NewOwnerID;
				break;
			}
		});	

	
	SNumericS1i::FOnValueChanged OnEntityOwnerSelectionGroupChanged;
	OnEntityOwnerSelectionGroupChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](int32 NewOwnerSelectionGroup)
		{
			const SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableSaveData->EntityUnits)
			{
				if (CurrentInteractable.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentInteractable.SelectionIndex = NewOwnerSelectionGroup;
				break;
			}
		});
	
	
	SNumericS1i::FOnValueChanged OnOptionalRewardAmountChanged;
	OnOptionalRewardAmountChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](int32 NewOptRewardAmount)
		{
			const SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableSaveData->EntityUnits)
			{
				if (CurrentInteractable.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentInteractable.CurrentAction.RewardAmount = NewOptRewardAmount;
				break;
			}
		});


	SNumericS1i::FOnValueChanged OnSavedEntityIndexChanged;
	OnSavedEntityIndexChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](const int32& NewSavedEntityIndex)
		{
			const SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableSaveData->EntityUnits)
			{
				if (CurrentUnit.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentUnit.InstanceIndex.Index = NewSavedEntityIndex;
				break;
			}
		});	
	
	SNumericS1i::FOnValueChanged OnOptTargetEntityIndexChanged;
	OnOptTargetEntityIndexChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](const int32& NewTargetEntityIndex)
		{
			const SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableSaveData->EntityUnits)
			{
				if (CurrentUnit.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentUnit.CurrentAction.OptTargets.ActionTargetAsEntity.Index = NewTargetEntityIndex;
				break;
			}
		});	
	
	SNumericV3d::FOnVectorValueChanged OnEntityTargetLocationChanged;
	OnEntityTargetLocationChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](const FVector& UpdatedVector)
		{
			const SRTSOSaveEditor_EntityData* MutableThis = const_cast<SRTSOSaveEditor_EntityData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableSaveData->EntityUnits)
			{
				if (CurrentUnit.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentUnit.CurrentAction.OptTargets.ActionTargetAsLocation = UpdatedVector;
				break;
			}
		});	
	
	MutableThis->EntityTable = SNew( STableRow< TSharedPtr<FRTSSavedWorldUnits> >, OwnerTable )
		[
			SNew(SVerticalBox)
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(STextBlock)
					.Text(FText::FromString("ENTITY"))
			]
			+ VERTICAL_SEPARATOR(5.0f)


			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("ENTITY BASE DATA"))
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
							.Text(FText::FromString("Index: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericEntryBox<int32>)
							.Value(SavedEntityDatum.InstanceIndex.Index)
							.OnValueChanged(OnSavedEntityIndexChanged)							
					]
				]
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
							.Text(FText::FromString(SavedEntityDatum.EntityUnitTag.ToString()))
							.OnClicked(OnEntityTypeClicked)
					]			
				]
			]
			+ VERTICAL_SEPARATOR(5.0f)
			
			
			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("ENTITY STATE DATA"))
			]				
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Location: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SNumericV3d)
						.Vector(SavedEntityDatum.Location)
						.OnVectorChanged(OnEntityLocationChanged)
				]
				+ HORIZONTAL_SEPARATOR(5.0f)
				
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SHorizontalBox)
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Health: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericEntryBox<double>)
							.Value(SavedEntityDatum.Health)
							.OnValueChanged(OnEntityHealthChanged)	 
					]			
				]					
			]
			+ VERTICAL_SEPARATOR(5.0f) 
			

			
			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("ENTITY OWNER DATA"))
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
							.Text(FText::FromString("OwnerID: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericS1i)
							.Value(SavedEntityDatum.OwnerID)
							.OnValueChanged(OnEntityOwnerIDChanged)
					]
				]
				+ HORIZONTAL_SEPARATOR(5.0f)

				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SHorizontalBox)
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("SelectionGroup: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericS1i)
							.Value(SavedEntityDatum.SelectionIndex)
							.OnValueChanged(OnEntityOwnerSelectionGroupChanged)
					]
				]
			]
			+ VERTICAL_SEPARATOR(5.0f) 


			
			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("ACTION DATA"))
			]
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(STextBlock)
					.Text(FText::FromString("BASE"))
			]					
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Action Type: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					// @todo implement SLightGameplayTagPicker then finish impl of 
					SNew(SButton)
						.Text(FText::FromString(SavedEntityDatum.CurrentAction.ActionTag.ToString()))
						.OnClicked(OnEntityActionTagChanged)						
					// SNew(STextBlock)
					// 	.Text(FText::FromString(SavedEntityDatum.CurrentAction.ActionTag.ToString()))
				]
			]
			+ VERTICAL_SEPARATOR(5.0f) 
			
			
			+ INSET_VERTICAL_SLOT(40)
			[
					SNew(STextBlock)
						.Text(FText::FromString("OPTIONAL REWARD"))
			]			
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Type: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					// @todo implement SLightGameplayTagPicker then finish impl of 
					SNew(SButton)
						.Text(FText::FromString(SavedEntityDatum.CurrentAction.Reward.ToString()))
						.OnClicked(OnOptRewardItemTypeChanged)
				]
				+ HORIZONTAL_SEPARATOR(5.0f) 

				
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Amount: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SNumericS1i)
						.Value(SavedEntityDatum.CurrentAction.RewardAmount)
						.OnValueChanged(OnOptionalRewardAmountChanged)
				]				
			]
			+ VERTICAL_SEPARATOR(2.0f) 

			
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(STextBlock)
					.Text(FText::FromString("OPTIONAL TARGETS"))
			]			
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("As Entity: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SNumericEntryBox<int32>)
						.Value(SavedEntityDatum.CurrentAction.OptTargets.ActionTargetAsEntity.Index)
						.OnValueChanged(OnOptTargetEntityIndexChanged)
				]
				+ HORIZONTAL_SEPARATOR(5.0f) 

				
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("As Actor: "))

				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					// @todo need a custom slate to allow selecting arbitrary target
					// @todo need to ActionTargetAsActor from being a pointer to being the ActorID to even be able to represent it in the save editor and even in the save game for that matter
					SNew(STextBlock)
						.Text(FText::FromString(SavedEntityDatum.CurrentAction.OptTargets.ActionTargetAsActor != nullptr ? SavedEntityDatum.CurrentAction.OptTargets.ActionTargetAsActor->GetName() : "N/A"))
				]
				+ HORIZONTAL_SEPARATOR(5.0f) 
				
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("As Pure Location: "))

				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SNumericV3d)
						.Vector(SavedEntityDatum.CurrentAction.OptTargets.ActionTargetAsLocation.Get())
						.OnVectorChanged(OnEntityTargetLocationChanged)
				]			
			]				
			
		];

	return MutableThis->EntityTable.ToSharedRef();
}

void SRTSOSaveEditor_EntityData::OnComponentSelected_EntityData(TSharedPtr<FRTSSavedWorldUnits> InItem, ESelectInfo::Type InSelectInfo)
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
	
	if(OnEntityDataChosen.IsBound())
	{
		OnEntityDataChosen.Execute(*InItem.Get());
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

