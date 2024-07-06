/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
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

#define LOCTEXT_NAMESPACE "SRTSOSaveEditor"

typedef SNumericVectorInputBox<int32, UE::Math::TVector<int32>, 3> SNumericV3i;
typedef SNumericVectorInputBox<double, FVector, 3> SNumericV3d;
typedef SNumericEntryBox<int32> SNumericS1i;
typedef SNumericEntryBox<double> SNumericS1d;

#define VERTICAL_SEPARATOR(thickness) \
SVerticalBox::Slot() \
[ \
	SNew(SSeparator) \
	.Thickness(thickness) \
	.Orientation(Orient_Vertical) \
]

#define HORIZONTAL_SEPARATOR(thickness) \
SHorizontalBox::Slot() \
[ \
	SNew(SSeparator) \
	.Thickness(thickness) \
	.Orientation(Orient_Horizontal) \
]

#define INSET_VERTICAL_SLOT(VerticalPadding) \
SVerticalBox::Slot() \
.Padding(FMargin{0, VerticalPadding}) \
.AutoHeight()

#define INSET_HORIZONTAL_SLOT(HorizontalPadding) \
SHorizontalBox::Slot() \
.Padding(FMargin{HorizontalPadding, 0}) \
.AutoWidth()


/** @brief SRTSOSaveEditorSubmenu */

//
// SAVE EDITOR MAIN
void SRTSOSaveEditor::Construct(const FArguments& InArgs)
{
}

void SRTSOSaveEditor::HandleValueChange(FGameplayTag Submenu, int32 MenuItem, int32 NewValue) const
{
	// @todo Write impl
}

void SRTSOSaveEditor::CopyData(URTSOpenSaveGame* InSaveGame)
{
	AsyncTask(ENamedThreads::AnyThread,
		[this, InSaveGame]()
		{
			if (InSaveGame == nullptr)
			{
				OnFailedCopyData();
				return;
			}
			
			SaveGamePtr = InSaveGame;
			CopiedSaveData = SaveGamePtr->Data;
			OnCompletedCopyData();
		});
	
}

void SRTSOSaveEditor::OnCompletedCopyData()
{
	// CopiedSaveData.Seeder;
	// CopiedSaveData.GameTime;
	// CopiedSaveData.PlayerLocations;
	// CopiedSaveData.Interactables;
	// CopiedSaveData.EntityUnits;
	// CopiedSaveData.Inventories;
	// CopiedSaveData.ConversationActorState;
	// CopiedSaveData.PlayersAndConversationTags;


	// @todo  Rethink design here, want to avoid having to iterate the maps here,
	// ^@todo some of them are large enough it will be a performance penalty if I keep it like this
	LocationsAsSharedTupleArray.Empty();
	for (const TTuple<int, UE::Math::TVector<double>>& PlayerLocationTuple : CopiedSaveData.PlayerLocations)
	{
		FPlayerLocationStruct PlayerDatum{PlayerLocationTuple.Key, PlayerLocationTuple.Value};
		LocationsAsSharedTupleArray.Emplace(MakeShared<FPlayerLocationStruct>(PlayerDatum).ToSharedPtr());
	}

	InteractableAsSharedArray.Empty();
	for (const FRTSSavedInteractable& Interactable : CopiedSaveData.Interactables)
	{
		InteractableAsSharedArray.Emplace(MakeShared<FRTSSavedInteractable>(Interactable).ToSharedPtr());
	}

	EntitiesAsSharedArray.Empty();
	for (const FRTSSavedWorldUnits& EntityUnit : CopiedSaveData.EntityUnits)
	{
		EntitiesAsSharedArray.Emplace(MakeShared<FRTSSavedWorldUnits>(EntityUnit).ToSharedPtr());
	}

	AllUserInventoriesAsSharedTupleArray.Empty();
	for (const TTuple<int32, FRTSSavedItems>& UserInventoryTuple : CopiedSaveData.Inventories)
	{
		FUserInventoriesStruct InvDatum{UserInventoryTuple.Key, UserInventoryTuple.Value};
		AllUserInventoriesAsSharedTupleArray.Emplace(MakeShared<FUserInventoriesStruct>(InvDatum).ToSharedPtr());
	}

	ConversationStatesAsSharedArray.Empty();
	for (const TTuple<int32, FRTSSavedConversationActorData>& ConversationStateTuple : CopiedSaveData.ConversationActorState)
	{
		FConversationStateStruct ConversationStateDatum{ConversationStateTuple.Key, ConversationStateTuple.Value};
		ConversationStatesAsSharedArray.Emplace(MakeShared<FConversationStateStruct>(ConversationStateDatum).ToSharedPtr());
	}		
	
	ChildSlot
	[
		SNew(SSplitter)
		+ SSplitter::Slot()
		[
			SNew(SNumericEntryBox<int32>)
				.Value(CopiedSaveData.Seeder.GetCurrentSeed())
				.OnValueChanged(this, &SRTSOSaveEditor::OnSeedValueChanged)
		] 
		+ SSplitter::Slot()
		[
			SNew(SNumericEntryBox<float>)
				.Value(CopiedSaveData.GameTime)
				.OnValueChanged(this, &SRTSOSaveEditor::OnGameTimeValueChanged)
		]
		+ SSplitter::Slot() // @todo make expandable and scrollable
		[
			SNew(SListView<TSharedPtr<FPlayerLocationStruct>>)
				.ListItemsSource( &LocationsAsSharedTupleArray)
				.OnGenerateRow( this, &SRTSOSaveEditor::MakeListViewWidget_PlayerData )
				.OnSelectionChanged( this, &SRTSOSaveEditor::OnComponentSelected_PlayerData )
		]
		+ SSplitter::Slot() // @todo make expandable and scrollable
		[
			SNew(SListView<TSharedPtr<FRTSSavedInteractable>>)
				.ListItemsSource( &InteractableAsSharedArray)
				.OnGenerateRow( this, &SRTSOSaveEditor::MakeListViewWidget_InteractableData )
				.OnSelectionChanged( this, &SRTSOSaveEditor::OnComponentSelected_InteractableData )
		]
		+ SSplitter::Slot() // @todo make expandable and scrollable
		[
			SNew(SListView<TSharedPtr<FRTSSavedWorldUnits>>)
				.ListItemsSource( &EntitiesAsSharedArray)
				.OnGenerateRow( this, &SRTSOSaveEditor::MakeListViewWidget_EntityData )
				.OnSelectionChanged( this, &SRTSOSaveEditor::OnComponentSelected_EntityData )
		]
		+ SSplitter::Slot() // @todo make expandable and scrollable
		[
			SNew(SListView<TSharedPtr<FUserInventoriesStruct>>)
				.ListItemsSource( &AllUserInventoriesAsSharedTupleArray)
				.OnGenerateRow( this, &SRTSOSaveEditor::MakeListViewWidget_InventoryOverviewData )
				.OnSelectionChanged( this, &SRTSOSaveEditor::OnComponentSelected_InventoryOverviewData )
		]
		+ SSplitter::Slot() // @todo make expandable and scrollable
		[
			SNew(SListView<TSharedPtr<FConversationStateStruct>>)
				.ListItemsSource( &ConversationStatesAsSharedArray)
				.OnGenerateRow( this, &SRTSOSaveEditor::MakeListViewWidget_ConversationStateData )
				.OnSelectionChanged( this, &SRTSOSaveEditor::OnComponentSelected_ConversationStateData )
		]
		+ SSplitter::Slot() // @todo make expandable and scrollable
		[
			SNew(SListView<TSharedPtr<FUserMissionTagsStruct>>)
				.ListItemsSource( &UserMissionTagsAsSharedArray)
				.OnGenerateRow( this, &SRTSOSaveEditor::MakeListViewWidget_UserMissionTags )
				.OnSelectionChanged( this, &SRTSOSaveEditor::OnComponentSelected_UserMissionTags )
		]	
		
	];	
}

TSharedRef<ITableRow> SRTSOSaveEditor::MakeListViewWidget_PlayerData(TSharedPtr<FPlayerLocationStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	const FPlayerLocationStruct& PlayerLocationTuple = *InItem.Get();
	
	SNumericV3d::FOnVectorValueChanged OnVectorValueChanged;
	OnVectorValueChanged.BindLambda(
		[ThisPtr = this, UserID = PlayerLocationTuple.Key](const FVector& UpdatedVector)
		{
			check(ThisPtr != nullptr) 
			if (ThisPtr->CopiedSaveData.PlayerLocations.Contains(UserID))
			{
				// @todo this might be problematic
				FVector& MutableLocation = *const_cast<FVector*>(ThisPtr->CopiedSaveData.PlayerLocations.Find(UserID));
				MutableLocation = UpdatedVector;
			}
		});
	

	return SNew( STableRow< TSharedPtr<FPlayerLocationStruct> >, OwnerTable )
		[
			SNew(SHorizontalBox)
			+ INSET_HORIZONTAL_SLOT(0)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock).Text(FText::FromString("User ID: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock).Text(FText::FromString(FString::FromInt(PlayerLocationTuple.Key)))
				]	
			
			]
			+ INSET_HORIZONTAL_SLOT(0)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock).Text(FText::FromString("User Location: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SNumericV3d)
						.Vector(PlayerLocationTuple.Value)
						.OnVectorChanged(OnVectorValueChanged)
				]			
			]
		];
}

void SRTSOSaveEditor::OnComponentSelected_PlayerData(TSharedPtr<FPlayerLocationStruct> InItem, ESelectInfo::Type InSelectInfo)
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
	
	if(OnPlayerDataChosen.IsBound())
	{
		OnPlayerDataChosen.Execute(*InItem.Get());
	}
}

void SRTSOSaveEditor::OnInteractableUsabilityChanged(int32 ActorID, float NewUsability) const
{
	// @todo
}

TSharedRef<ITableRow> SRTSOSaveEditor::MakeListViewWidget_InteractableData(TSharedPtr<FRTSSavedInteractable> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(this);
	check(MutableThis != nullptr)
	
	const FRTSSavedInteractable& SavedInteractableDatum = *InItem.Get();

	// @done SavedInteractableDatum.Location;
	// @done SavedInteractableDatum.Usability;
	// @done SavedInteractableDatum.ActorClass;
	// @done SavedInteractableDatum.InstanceIndex;
	
	SNumericV3d::FOnVectorValueChanged OnPlayerLocationChanged;
	OnPlayerLocationChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedInteractableDatum.InstanceIndex](const FVector& UpdatedVector)
		{
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedInteractable& CurrentInteractable : MutableThis->CopiedSaveData.Interactables)
			{
				if (CurrentInteractable.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentInteractable.Location = UpdatedVector;
				break;
			}
		});

	SNumericS1d::FOnValueChanged OnUsabilityChanged;
	OnUsabilityChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedInteractableDatum.InstanceIndex](double UpdatedValue)
		{
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedInteractable& CurrentInteractable : MutableThis->CopiedSaveData.Interactables)
			{
				if (CurrentInteractable.InstanceIndex != InstanceIndex) { continue;}
				
				CurrentInteractable.Usability = UpdatedValue;
				break;
			}
		});	


	// @note THIS WILL ONLY WORK IN EDITOR AS CLASS PICKER IS AN EDITOR TYPE
	// @todo make version that shows simplified class list, investigate how uclasses are represented in shipped builds and write code that appropriately handles representation in different build types  
	FOnClicked OnActorClassClicked;
	OnActorClassClicked.BindLambda(
		[ImmutableThis = this]() -> FReply
		{
#if WITH_EDITOR
			const SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
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
			//
			// PickerWindow->SetContent(ClassPickerDialog);
			//
			// const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelRegularWindow();
			// if( ParentWindow.IsValid() )
			// {
			// 	FSlateApplication::Get().AddModalWindow(PickerWindow, ParentWindow );
			// }
			
#endif // WITH_EDITOR
			return FReply::Handled();
		});
			
	MutableThis->InteractableTable = SNew( STableRow< TSharedPtr<FRTSSavedInteractable>>, OwnerTable )
		[
			SNew(SVerticalBox)
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SHorizontalBox)
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("ID: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString(FString::FromInt(SavedInteractableDatum.InstanceIndex)))
					]
				]
			]
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SHorizontalBox)
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Class: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SButton)
							.OnClicked(OnActorClassClicked)
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SButton)
							.Text(FText::FromString(SavedInteractableDatum.ActorClass.ToString()))
							.OnClicked(OnActorClassClicked)
					]			
				]
				+ INSET_HORIZONTAL_SLOT(0)
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
							.Vector(SavedInteractableDatum.Location)
							.OnVectorChanged(OnPlayerLocationChanged)
					]			
				]
			]
			+ INSET_VERTICAL_SLOT(0)
			[

				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SHorizontalBox)
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Usability: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericEntryBox<double>)
								.Value(SavedInteractableDatum.Usability)
								.OnValueChanged(OnUsabilityChanged)	 
					]			
				]				
				
			]
		];


	/*
	 */
	
	return MutableThis->InteractableTable.ToSharedRef();
}

void SRTSOSaveEditor::OnComponentSelected_InteractableData(TSharedPtr<FRTSSavedInteractable> InItem, ESelectInfo::Type InSelectInfo)
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
	
	if(OnInteractableDataChosen.IsBound())
	{
		OnInteractableDataChosen.Execute(*InItem.Get());
	}	
}

// @todo implement SLightGameplayTagPicker then change return type
static TSharedRef<SWindow> CreatePickerDialog(TSharedRef<SWindow>& PickerWindow)
{
	FClassViewerInitializationOptions InitOptions;
	InitOptions.Mode = EClassViewerMode::ClassPicker;
	InitOptions.DisplayMode = EClassViewerDisplayMode::TreeView;

	const TSharedRef<FRTSSaveEd_InteractableClassFilter> SaveEd_InteractableClassFilter = MakeShared<FRTSSaveEd_InteractableClassFilter>();
	SaveEd_InteractableClassFilter->InterfaceThatMustBeImplemented = UPDInteractInterface::StaticClass();
	InitOptions.ClassFilters.Add(SaveEd_InteractableClassFilter);
	
	// @todo implement SLightGameplayTagPicker then uncomment below
	// return TSharedRef<SLightGameplayTagPicker> GameplayTagPickerDialog = SNew(SLightGameplayTagPicker)
	// 	.ParentWindow(PickerWindow)
	// 	.Options(InitOptions)
	// 	.AssetType(nullptr);
	//

	return SNew(SWindow);	
}

static TSharedRef<SWindow> CreatePickerWindow()
{
	// Create the window to pick the class
	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(FText())
		.SizingRule( ESizingRule::Autosized )
		.ClientSize( FVector2D( 0.f, 300.f ))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	// @todo implement SLightGameplayTagPicker then uncomment below
	// TSharedRef<SLightGameplayTagPicker> GameplayTagPickerDialog = CreatePickerDialog();
	// PickerWindow->SetContent(GameplayTagPickerDialog);

	const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelRegularWindow();
	if( ParentWindow.IsValid() )
	{
		FSlateApplication::Get().AddModalWindow(PickerWindow, ParentWindow );
	}

	return PickerWindow;
}

TSharedRef<ITableRow> SRTSOSaveEditor::MakeListViewWidget_EntityData(TSharedPtr<FRTSSavedWorldUnits> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(this);
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
				// @todo associate with picker window, to assign result in below member
				SavedEntityDatum.EntityUnitTag;
				
				// Create the window to pick the class
				TSharedRef<SWindow> PickerWindow = CreatePickerWindow();
				return FReply::Handled();
			});

	const FOnClicked OnEntityActionTagChanged =
		FOnClicked::CreateLambda(
			[&]() -> FReply
			{
				// @todo associate with picker window, to assign result in below member
				SavedEntityDatum.CurrentAction.ActionTag;
				
				// Create the window to pick the class
				TSharedRef<SWindow> PickerWindow = CreatePickerWindow();
				return FReply::Handled();
			});

	const FOnClicked OnOptRewardItemTypeChanged =
		FOnClicked::CreateLambda(
			[&]() -> FReply
			{
				// @todo associate with picker window, to assign result in below member
				SavedEntityDatum.CurrentAction.Reward;
				
				// Create the window to pick the class
				TSharedRef<SWindow> PickerWindow = CreatePickerWindow();
				return FReply::Handled();
			});		
	

	SNumericV3d::FOnVectorValueChanged OnEntityLocationChanged;
	OnEntityLocationChanged.BindLambda(
		[ImmutableThis = this, InstanceIndex = SavedEntityDatum.InstanceIndex](const FVector& UpdatedVector)
		{
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableThis->CopiedSaveData.EntityUnits)
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableThis->CopiedSaveData.EntityUnits)
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableThis->CopiedSaveData.EntityUnits)
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableThis->CopiedSaveData.EntityUnits)
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentInteractable : MutableThis->CopiedSaveData.EntityUnits)
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableThis->CopiedSaveData.EntityUnits)
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableThis->CopiedSaveData.EntityUnits)
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			// Slow, @todo come back to this
			for (FRTSSavedWorldUnits& CurrentUnit : MutableThis->CopiedSaveData.EntityUnits)
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

void SRTSOSaveEditor::OnComponentSelected_EntityData(TSharedPtr<FRTSSavedWorldUnits> InItem, ESelectInfo::Type InSelectInfo)
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

TSharedRef<ITableRow> SRTSOSaveEditor::MakeListViewWidget_InventoryOverviewData(TSharedPtr<FUserInventoriesStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(this);
	check(MutableThis != nullptr)

	FUserInventoriesStruct& SavedInventoryDatum = *InItem.Get();

	const int32 UserID = SavedInventoryDatum.Key; // 
	const TArray<FPDItemNetDatum>& UserItems = SavedInventoryDatum.Value.Items; //

	const SNumericS1i::FOnValueChanged OnInventoryOwningUserIDChanged =
		SNumericS1i::FOnValueChanged::CreateLambda(
			[&](int32 NewUserID)
			{
				// Slow, @todo come back to this
				for (TTuple<int32, FRTSSavedItems>& CurrentInventory : MutableThis->CopiedSaveData.Inventories)
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
						.OnGenerateRow( this, &SRTSOSaveEditor::MakeListViewWidget_ItemData )
						.OnSelectionChanged( this, &SRTSOSaveEditor::OnComponentSelected_ItemData )
				]
				+ HORIZONTAL_SEPARATOR(5.0f)
			]
			+ VERTICAL_SEPARATOR(5.0f) 
			
		];

	return MutableThis->InventoryTable.ToSharedRef();	
}

void SRTSOSaveEditor::OnComponentSelected_InventoryOverviewData(TSharedPtr<FUserInventoriesStruct> InItem, ESelectInfo::Type InSelectInfo)
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

TSharedRef<ITableRow> SRTSOSaveEditor::MakeListViewWidget_ItemData(TSharedPtr<FPDItemNetDatum> InItem,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	const SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(this);
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
				// @todo associate with picker window, to assign result in below member
				SavedItemDatum.ItemTag;
				
				// Create the window to pick the class
				TSharedRef<SWindow> PickerWindow = CreatePickerWindow();
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

void SRTSOSaveEditor::OnComponentSelected_ItemData(TSharedPtr<FPDItemNetDatum> InItem, ESelectInfo::Type InSelectInfo) const
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

TSharedRef<ITableRow> SRTSOSaveEditor::MakeListViewWidget_ConversationStateData(TSharedPtr<FConversationStateStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(this);
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
			const SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
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
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			FRTSSavedConversationActorData DataToMove;
			MutableThis->CopiedSaveData.ConversationActorState.RemoveAndCopyValue(ConversationActorID, DataToMove);

			// Swap instead of overwriting if we already have an existing state on the new requested ID 
			FRTSSavedConversationActorData Swap;
			if (MutableThis->CopiedSaveData.ConversationActorState.Contains(NewActorID))
			{
				MutableThis->CopiedSaveData.ConversationActorState.RemoveAndCopyValue(NewActorID, Swap);
				MutableThis->CopiedSaveData.ConversationActorState.Emplace(ConversationActorID, Swap);
			}

			MutableThis->CopiedSaveData.ConversationActorState.Emplace(NewActorID, DataToMove);
		});

	SNumericV3d::FOnVectorValueChanged OnConversationActorLocationChanged;
	OnConversationActorLocationChanged.BindLambda(
		[ImmutableThis = this, ConversationActorID = SavedConversationStateDatum.Key](const FVector& UpdatedVector)
		{
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			MutableThis->CopiedSaveData.ConversationActorState.Find(ConversationActorID)->Location = UpdatedVector;
		});	
	
	SNumericS1d::FOnValueChanged OnConversationActorHealthChanged;
	OnConversationActorHealthChanged.BindLambda(
		[ImmutableThis = this, ConversationActorID = SavedConversationStateDatum.Key](double NewActorHealth)
		{
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			MutableThis->CopiedSaveData.ConversationActorState.Find(ConversationActorID)->Health = NewActorHealth;
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

void SRTSOSaveEditor::OnComponentSelected_ConversationStateData(TSharedPtr<FConversationStateStruct> InItem, ESelectInfo::Type InSelectInfo)
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






TSharedRef<ITableRow> SRTSOSaveEditor::MakeListViewWidget_UserMissionTags(TSharedPtr<FUserMissionTagsStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(this);
	check(MutableThis != nullptr)

	const FUserMissionTagsStruct& SavedMissionTagsDatum = *InItem.Get();

	//
	// Data
	int32 UserID = SavedMissionTagsDatum.Key;
	const FGameplayTagContainer& InnerConversationDatum = SavedMissionTagsDatum.Value;
	
	//
	// Gather shared ptrs
	TArray<TSharedPtr<FGameplayTag>> CurrentMissionTagsAsSharedTupleArray;
	for (const FGameplayTag& SelectedMissionTag : InnerConversationDatum.GetGameplayTagArray())
	{
		CurrentMissionTagsAsSharedTupleArray.Emplace(MakeShared<FGameplayTag>(SelectedMissionTag).ToSharedPtr());
	}
	
	//
	// Callbacks
	SNumericS1i::FOnValueChanged OnUserIDChanged;
	OnUserIDChanged.BindLambda(
		[ImmutableThis = this, OldUserID = UserID](int NewUserID)
		{
			SRTSOSaveEditor* MutableThis = const_cast<SRTSOSaveEditor*>(ImmutableThis);
			check(MutableThis != nullptr)

			FGameplayTagContainer DataToMove;
			MutableThis->CopiedSaveData.PlayersAndConversationTags.RemoveAndCopyValue(OldUserID, DataToMove);

			// Swap instead of overwriting if we already have existing data on the new requested ID 
			FGameplayTagContainer Swap;
			if (MutableThis->CopiedSaveData.PlayersAndConversationTags.Contains(NewUserID))
			{
				MutableThis->CopiedSaveData.PlayersAndConversationTags.RemoveAndCopyValue(NewUserID, Swap);
				MutableThis->CopiedSaveData.PlayersAndConversationTags.Emplace(OldUserID, Swap);
			}

			MutableThis->CopiedSaveData.PlayersAndConversationTags.Emplace(NewUserID, DataToMove);
		});

	const SListView<TSharedPtr<FGameplayTag>>::FOnGenerateRow OnGenerateMissionTagRowWidget =
		SListView<TSharedPtr<FGameplayTag>>::FOnGenerateRow::CreateLambda(
			[] (TSharedPtr<FGameplayTag> InItem, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
	{
		// @todo
		return SNew( STableRow< TSharedPtr<FName> >, OwnerTable );
	});

	
	const SListView<TSharedPtr<FGameplayTag>>::FOnSelectionChanged OnSelectMissionTagComponent =
	SListView<TSharedPtr<FGameplayTag>>::FOnSelectionChanged::CreateLambda(
	[](TSharedPtr<FGameplayTag> InItem, ESelectInfo::Type InSelectInfo)
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
	MutableThis->MissionTagsTable = SNew( STableRow< TSharedPtr<FUserMissionTagsStruct> >, OwnerTable )
		[
			SNew(SVerticalBox)
			+ INSET_VERTICAL_SLOT(0)
			[
				SNew(STextBlock)
					.Text(FText::FromString("MISSION PROGRESS"))
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
							.Text(FText::FromString("User ID: "))
					]
					+ INSET_HORIZONTAL_SLOT(0)
					[
						SNew(SNumericEntryBox<int32>)
							.Value(UserID)
							.OnValueChanged(OnUserIDChanged)							
					]
				]
			]
			+ VERTICAL_SEPARATOR(5.0f)
			
			
			+ INSET_VERTICAL_SLOT(20)
			[
				SNew(STextBlock)
					.Text(FText::FromString("PROGRESS DATA"))
			]				
			+ INSET_VERTICAL_SLOT(40)
			[
				SNew(SHorizontalBox)
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Tags: "))
				]
				+ INSET_HORIZONTAL_SLOT(0)
				[
					SNew(SListView<TSharedPtr<FGameplayTag>>)
						.ListItemsSource(&CurrentMissionTagsAsSharedTupleArray)
						.OnGenerateRow(OnGenerateMissionTagRowWidget)
						.OnSelectionChanged(OnSelectMissionTagComponent)
				]
			]
		];

	return MutableThis->MissionTagsTable.ToSharedRef();	
}

void SRTSOSaveEditor::OnComponentSelected_UserMissionTags(TSharedPtr<FUserMissionTagsStruct> InItem, ESelectInfo::Type InSelectInfo)
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
	
	if(OnOnMissionTagsDataChosen.IsBound())
	{
		OnOnMissionTagsDataChosen.Execute(*InItem.Get());
	}			
}

void SRTSOSaveEditor::OnFailedCopyData()
{
}

void SRTSOSaveEditor::OnSeedValueChanged(int32 NewSeed)
{
	CopiedSaveData.Seeder = FRandomStream(NewSeed); 
}

void SRTSOSaveEditor::OnGameTimeValueChanged(float NewGameTime)
{
	CopiedSaveData.GameTime = NewGameTime;
}

void SRTSOSaveEditor::ResetFieldData(EPDSaveDataThreadSelector SaveDataGroupSelector)
{
	if (SaveGamePtr == nullptr)
	{
		UE_LOG(PDLog_SaveEditor, Error, TEXT("SRTSOSaveEditor::ResetFieldData -- 'SaveGamePtr' has gone stale"))
		return;
	}
	
	switch (SaveDataGroupSelector)
	{
	case EPDSaveDataThreadSelector::ESeeder:
		CopiedSaveData.Seeder = SaveGamePtr->Data.Seeder;
		break;
	case EPDSaveDataThreadSelector::EGameTime:
		CopiedSaveData.GameTime = SaveGamePtr->Data.GameTime;
		break;
	case EPDSaveDataThreadSelector::EPlayers:
		CopiedSaveData.PlayerLocations = SaveGamePtr->Data.PlayerLocations;
		break;
	case EPDSaveDataThreadSelector::EInteractables:
		CopiedSaveData.Interactables = SaveGamePtr->Data.Interactables;
		break;
	case EPDSaveDataThreadSelector::EEntities:
		CopiedSaveData.EntityUnits = SaveGamePtr->Data.EntityUnits;
		break;
	case EPDSaveDataThreadSelector::EInventories:
		CopiedSaveData.Inventories = SaveGamePtr->Data.Inventories;
		break;
	case EPDSaveDataThreadSelector::EConversationActors:
		CopiedSaveData.ConversationActorState = SaveGamePtr->Data.ConversationActorState;
		break;
	case EPDSaveDataThreadSelector::EPlayerConversationProgress:
		CopiedSaveData.PlayersAndConversationTags = SaveGamePtr->Data.PlayersAndConversationTags;
		break;
	case EPDSaveDataThreadSelector::EEnd:
		CopiedSaveData.Seeder = SaveGamePtr->Data.Seeder;
		CopiedSaveData.GameTime = SaveGamePtr->Data.GameTime;
		CopiedSaveData.PlayerLocations = SaveGamePtr->Data.PlayerLocations;
		CopiedSaveData.Interactables = SaveGamePtr->Data.Interactables;
		CopiedSaveData.EntityUnits = SaveGamePtr->Data.EntityUnits;
		CopiedSaveData.Inventories = SaveGamePtr->Data.Inventories;
		CopiedSaveData.ConversationActorState = SaveGamePtr->Data.ConversationActorState;
		CopiedSaveData.PlayersAndConversationTags = SaveGamePtr->Data.PlayersAndConversationTags;
		break;
	}
}


//
// SAVE EDITOR SUB-MENU
void SRTSOSaveEditorSubmenu::Construct(const FArguments& InArgs)
{
	SubmenuTag        = InArgs._SubmenuTag;
	OnValueChanged    = InArgs._OnValueChanged;

	ChildSlot
	[
		SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
					.HAlign(HAlign_Left)
					.AutoHeight()
					.Padding(0, 2)
				[
					MakeSubmenu()
				]
			]
		]
	];
}

TSharedRef<SWidget> SRTSOSaveEditorSubmenu::MakeSubmenu()
{
	TSharedPtr<SButton> EditButton;
	SAssignNew(EditButton, SButton)
		.ButtonStyle(FAppStyle::Get(), "NoBorder")
		// .OnClicked(FOnClicked::CreateSP(this, &SRTSOSaveEditorSubmenu::))
		.ContentPadding(0)
		.ToolTipText(NSLOCTEXT("SaveEditor", "SaveEditor_ModifyElem", "Modify the selected item"))
	[
		SNew(SImage)
			.Image(FAppStyle::GetBrush(TEXT("Icons.Search")))
	];

	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1, 0)
		.VAlign(VAlign_Center)
	[
		SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "NoBorder")
			.OnClicked(FOnClicked::CreateSP(this, &SRTSOSaveEditorSubmenu::OnUseSelectedItemClick))
			.ContentPadding(1.f)
			.ToolTipText(NSLOCTEXT("SaveEditor", "SaveEditor_SelectItem", "Selected an item in the editor"))
		[
			SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("Icons.CircleArrowLeft")))
		]
	]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1, 0)
		.VAlign(VAlign_Center)
	[
		// @todo Write actual impl
		EditButton.ToSharedRef()
	];
}

FText SRTSOSaveEditorSubmenu::GetSubmenuTextValue() const
{
	FText SubmenuTitle = LOCTEXT("SelectSubmenu", "Select a Submenu...");
	// @todo Write impl

	return SubmenuTitle;
}

FReply SRTSOSaveEditorSubmenu::OnUseSelectedItemClick()
{
	// @todo Write impl

	return FReply::Handled();
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

