/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "SaveEditor/SRTSOSaveEditor_ConversationsData.h"
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

#define LOCTEXT_NAMESPACE "SRTSOSaveEditor_ConversationsData"

typedef SNumericVectorInputBox<int32, UE::Math::TVector<int32>, 3> SNumericV3i;
typedef SNumericVectorInputBox<double, FVector, 3> SNumericV3d;
typedef SNumericEntryBox<int32> SNumericS1i;
typedef SNumericEntryBox<double> SNumericS1d;


//
// SAVE EDITOR MAIN
void SRTSOSaveEditor_ConversationsData::Construct(const FArguments& InArgs)
{
	UpdateChildSlot(nullptr);
}

void SRTSOSaveEditor_ConversationsData::UpdateChildSlot(void* OpaqueData)
{
	// Covers representing below fields
	// CopiedSaveData.ConversationActorState;

	ConversationStatesAsSharedArray = static_cast<TArray<TSharedPtr<FConversationStateStruct>>*>(OpaqueData);
	
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
				SNew(STextBlock).Text(FText::FromString("CONVERSATIONS STATE DATA: "))
			]
			
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(SListView<TSharedPtr<FConversationStateStruct>>)
					.ListItemsSource(ConversationStatesAsSharedArray)
					.OnGenerateRow( this, &SRTSOSaveEditor_ConversationsData::MakeListViewWidget_ConversationStateData )
					.OnSelectionChanged( this, &SRTSOSaveEditor_ConversationsData::OnComponentSelected_ConversationStateData )
			]
		]
	];	
}


TSharedRef<ITableRow> SRTSOSaveEditor_ConversationsData::MakeListViewWidget_ConversationStateData(TSharedPtr<FConversationStateStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor_ConversationsData* MutableThis = const_cast<SRTSOSaveEditor_ConversationsData*>(this);
	check(MutableThis != nullptr)

	FConversationStateStruct& SavedConversationStateDatum = *InItem.Get();	

	//
	// Gather shared ptrs
	TArray<TSharedPtr<FName>> CurrentMissionsAsSharedTupleArray;
	for (const FName& SelectedMission : SavedConversationStateDatum.Value.Missions)
	{
		CurrentMissionsAsSharedTupleArray.Emplace(MakeShared<FName>(SelectedMission).ToSharedPtr());
	}

	TArray<TSharedPtr<FConversationProgressionInnerStruct>> PlayerProgressForCurrentActorAsSharedTupleArray;
	for (const TTuple<int, FRTSOConversationMetaProgressionListWrapper>& SelectedPlayerProgression : SavedConversationStateDatum.Value.ProgressionPerPlayer)
	{
		FConversationProgressionInnerStruct SelectedPlayerConversationProgressionDatum{SelectedPlayerProgression.Key, SelectedPlayerProgression.Value};
		PlayerProgressForCurrentActorAsSharedTupleArray.Emplace(MakeShared<FConversationProgressionInnerStruct>(SelectedPlayerConversationProgressionDatum).ToSharedPtr());
	}	
	

	//
	// Callbacks
	// @note THIS WILL ONLY WORK IN EDITOR AS CLASS PICKER IS AN EDITOR TYPE
	// @todo make version that shows simplified class list, investigate how uclasses are represented in shipped builds and write code that appropriately handles representation in different build types  
	FOnClicked OnActorClassClicked;
	OnActorClassClicked.BindLambda(
		[ImmutableThis = this, ConversationActorID = SavedConversationStateDatum.Key]() -> FReply
		{
#if WITH_EDITOR
			const SRTSOSaveEditor_ConversationsData* MutableThis = const_cast<SRTSOSaveEditor_ConversationsData*>(ImmutableThis);
			check(MutableThis != nullptr)
			
			FClassViewerInitializationOptions InitOptions;
			InitOptions.Mode = EClassViewerMode::ClassPicker;
			InitOptions.DisplayMode = EClassViewerDisplayMode::TreeView;

			const TSharedRef<FRTSSaveEd_InteractableClassFilter> SaveEd_InteractableClassFilter = MakeShared<FRTSSaveEd_InteractableClassFilter>();
			SaveEd_InteractableClassFilter->InterfaceThatMustBeImplemented = UPDInteractInterface::StaticClass();
			InitOptions.ClassFilters.Add(SaveEd_InteractableClassFilter);

			UClass* ChosenClass = nullptr;
			SClassPickerDialog::PickClass(FText(), InitOptions, ChosenClass, nullptr);

			// @todo get back to this, need to get around SNew(SClassPickerDialog) not being callable outside of SClassPickerDialog
			// // Create the window to pick the class
			// const TSharedRef<SWindow> PickerWindow = SNew(SWindow)
			// 	.Title(FText())
			// 	.SizingRule( ESizingRule::Autosized )
			// 	.ClientSize( FVector2D( 0.f, 300.f ))
			// 	.SupportsMaximize(false)
			// 	.SupportsMinimize(false);
			//
			// const TSharedRef<SClassPickerDialog> ClassPickerDialog = SNew(SClassPickerDialog)
			// 	.ParentWindow(PickerWindow)
			// 	.Options(InitOptions)
			// 	.AssetType(nullptr);
			//
			// PickerWindow->SetContent(ClassPickerDialog);
			//
			// const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelRegularWindow();
			// if( ParentWindow.IsValid() )
			// {
			// 	FSlateApplication::Get().AddModalWindow(PickerWindow, ParentWindow );
			// }
			//
#endif // WITH_EDITOR
			return FReply::Handled();
		});

	SNumericS1i::FOnValueChanged OnConversationActorIDChanged;
	OnConversationActorIDChanged.BindLambda(
		[ImmutableThis = this, ConversationActorID = SavedConversationStateDatum.Key](int NewActorID)
		{
			SRTSOSaveEditor_ConversationsData* MutableThis = const_cast<SRTSOSaveEditor_ConversationsData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			FRTSSavedConversationActorData DataToMove;
			MutableSaveData->ConversationActorState.RemoveAndCopyValue(ConversationActorID, DataToMove);

			// Swap instead of overwriting if we already have an existing state on the new requested ID 
			FRTSSavedConversationActorData Swap;
			if (MutableSaveData->ConversationActorState.Contains(NewActorID))
			{
				MutableSaveData->ConversationActorState.RemoveAndCopyValue(NewActorID, Swap);
				MutableSaveData->ConversationActorState.Emplace(ConversationActorID, Swap);
			}

			MutableSaveData->ConversationActorState.Emplace(NewActorID, DataToMove);
		});

	SNumericV3d::FOnVectorValueChanged OnConversationActorLocationChanged;
	OnConversationActorLocationChanged.BindLambda(
		[ImmutableThis = this, ConversationActorID = SavedConversationStateDatum.Key](const FVector& UpdatedVector)
		{
			SRTSOSaveEditor_ConversationsData* MutableThis = const_cast<SRTSOSaveEditor_ConversationsData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			MutableSaveData->ConversationActorState.Find(ConversationActorID)->Location = UpdatedVector;
		});	
	
	SNumericS1d::FOnValueChanged OnConversationActorHealthChanged;
	OnConversationActorHealthChanged.BindLambda(
		[ImmutableThis = this, ConversationActorID = SavedConversationStateDatum.Key](double NewActorHealth)
		{
			SRTSOSaveEditor_ConversationsData* MutableThis = const_cast<SRTSOSaveEditor_ConversationsData*>(ImmutableThis);
			check(MutableThis != nullptr)
			FRTSSaveData* MutableSaveData = MutableThis->LinkedSaveDataCopy;

			MutableSaveData->ConversationActorState.Find(ConversationActorID)->Health = NewActorHealth;
		});

	
	const SListView<TSharedPtr<FName>>::FOnGenerateRow OnGenerateMissionRowWidget =
		SListView<TSharedPtr<FName>>::FOnGenerateRow::CreateLambda(
			[] (TSharedPtr<FName> InItem, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
	{
		// @todo Write impl
		return SNew( STableRow< TSharedPtr<FName> >, OwnerTable );
	});

	
	const SListView<TSharedPtr<FName>>::FOnSelectionChanged OnSelectMissionComponent =
	SListView<TSharedPtr<FName>>::FOnSelectionChanged::CreateLambda(
	[](TSharedPtr<FName> InItem, ESelectInfo::Type InSelectInfo)
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
	

	const SListView<TSharedPtr<FConversationProgressionInnerStruct>>::FOnGenerateRow OnGeneratePlayerMissionProgressRowWidget =
		SListView<TSharedPtr<FConversationProgressionInnerStruct>>::FOnGenerateRow::CreateLambda(
			[] (TSharedPtr<FConversationProgressionInnerStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
	{
		// @todo
		return SNew( STableRow< TSharedPtr<FConversationProgressionInnerStruct> >, OwnerTable );
	});

	
	const SListView<TSharedPtr<FConversationProgressionInnerStruct>>::FOnSelectionChanged OnSelectPlayerMissionProgressComponent =
	SListView<TSharedPtr<FConversationProgressionInnerStruct>>::FOnSelectionChanged::CreateLambda(
	[](TSharedPtr<FConversationProgressionInnerStruct> InItem, ESelectInfo::Type InSelectInfo)
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
		


	//
	// Widget layout
	MutableThis->ConversationStateTable = SNew( STableRow< TSharedPtr<FConversationStateStruct> >, OwnerTable )
		[
			SNew(SVerticalBox)
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(STextBlock)
					.Text(FText::FromString("CONVERSATION"))
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
							.Text(FText::FromString("Conversation Actor ID: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericEntryBox<int32>)
							.Value(SavedConversationStateDatum.Key)
							.OnValueChanged(OnConversationActorIDChanged)							
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
							.Text(FText::FromString("Actor Class Type: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SButton)
							.Text(FText::FromString(SavedConversationStateDatum.Value.ActorClassType.ToString()))
							.OnClicked(OnActorClassClicked)
					]			
				]
			]
			+ VERTICAL_SEPARATOR(5.0f)
			
			
			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("STATE DATA"))
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
						.Vector(SavedConversationStateDatum.Value.Location)
						.OnVectorChanged(OnConversationActorLocationChanged)
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
							.Value(SavedConversationStateDatum.Value.Health)
							.OnValueChanged(OnConversationActorHealthChanged)	 
					]			
				]					
			]
			+ VERTICAL_SEPARATOR(5.0f) 
			

			
			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("MISSION DATA"))
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
							.Text(FText::FromString("Mission List: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SListView<TSharedPtr<FName>>)
							.ListItemsSource(&CurrentMissionsAsSharedTupleArray)
							.OnGenerateRow(OnGenerateMissionRowWidget)
							.OnSelectionChanged(OnSelectMissionComponent)
					]
				]
				+ HORIZONTAL_SEPARATOR(5.0f)

				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SHorizontalBox)
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Progression Per Player: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SListView<TSharedPtr<FConversationProgressionInnerStruct>>)
							.ListItemsSource(&PlayerProgressForCurrentActorAsSharedTupleArray)
							.OnGenerateRow(OnGeneratePlayerMissionProgressRowWidget)
							.OnSelectionChanged(OnSelectPlayerMissionProgressComponent)
					]
				]
			]
		];

	return MutableThis->ConversationStateTable.ToSharedRef();	
}

void SRTSOSaveEditor_ConversationsData::OnComponentSelected_ConversationStateData(TSharedPtr<FConversationStateStruct> InItem, ESelectInfo::Type InSelectInfo)
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
	
	if(OnConversationStateDataChosen.IsBound())
	{
		OnConversationStateDataChosen.Execute(*InItem.Get());
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

