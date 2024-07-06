﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RTSOpenCommon.h"
#include "ClassViewerFilter.h"
#include "Subsystems/EngineSubsystem.h"
#include "SRTSOSaveEditor.generated.h"

DECLARE_LOG_CATEGORY_CLASS(PDLog_SaveEditor, Log, All);

USTRUCT()
struct FPlayerLocationStruct 
{ 
	GENERATED_BODY() 
	int32 /*UserID*/ Key; 
	FVector /*User Location*/ Value;
};
USTRUCT()
struct FUserInventoriesStruct 
{ 
	GENERATED_BODY() 
	int32 /*UserID*/ Key; 
	FRTSSavedItems /*User Inventory Wrapper*/ Value;
};
USTRUCT()
struct FConversationStateStruct 
{ 
	GENERATED_BODY() 
	int32 /*ActorID*/ Key; 
	FRTSSavedConversationActorData Value;
};
USTRUCT()
struct FConversationProgressionInnerStruct 
{ 
	GENERATED_BODY() 
	int32 /*UserID*/ Key; 
	FRTSOConversationMetaProgressionListWrapper /*ProgressionLevel*/ Value;
};
USTRUCT()
struct FUserMissionTagsStruct 
{ 
	GENERATED_BODY() 
	int32 Key /*PlayerID*/; 
	FGameplayTagContainer /*AccumulatedMissionTags*/ Value;
};
USTRUCT()
struct FStacksStruct 
{ 
	GENERATED_BODY() 
	int32 Key /*StackIndex*/; 
	int32 /*ItemCount*/ Value;
};

/**
 * @brief  Loads custom tags that may have been added by a player/user
*/
class RTSOPEN_API SRTSOSaveEditor : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_OneParam( FOnPlayerDataChosen, const FPlayerLocationStruct&);
	DECLARE_DELEGATE_OneParam( FOnInteractableDataChosen, const FRTSSavedInteractable&);
	DECLARE_DELEGATE_OneParam( FOnEntityDataChosen, const FRTSSavedWorldUnits&);
	DECLARE_DELEGATE_OneParam( FOnInventoryOverviewDataChosen, const FUserInventoriesStruct&);
	DECLARE_DELEGATE_OneParam( FOnItemDataChosen, const FPDItemNetDatum&);
	DECLARE_DELEGATE_OneParam( FOnConverstionStateDataChosen, const FConversationStateStruct&);
	DECLARE_DELEGATE_OneParam( FOnMissionTagsDataChosen, const FUserMissionTagsStruct&);
	DECLARE_DELEGATE_OneParam( FOnInteractableClassPicked, const TOptional<EMouseCursor::Type>&);

	
	SLATE_BEGIN_ARGS(SRTSOSaveEditor) { }

	
 		/** @todo Called when the save editor window is scrolled. */
 		SLATE_EVENT(FOnUserScrolled, OnUserScrolled)

 		/** @todo Called when an element is clicked. */
 		SLATE_EVENT(FOnClicked, OnUserClicked)
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void HandleValueChange(FGameplayTag Submenu, int32 MenuItem, int32 NewValue) const;

	void CopyData(URTSOpenSaveGame* InSaveGame);
	void OnCompletedCopyData();
	void OnFailedCopyData();

	void OnSeedValueChanged(int32 NewSeed);
	void OnGameTimeValueChanged(float NewGameTime);

	void ResetFieldData(EPDSaveDataThreadSelector SaveDataGroupSelector);

	TSharedRef<ITableRow> MakeListViewWidget_PlayerData(TSharedPtr<FPlayerLocationStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_PlayerData(TSharedPtr<FPlayerLocationStruct> InItem, ESelectInfo::Type InSelectInfo);

	void OnInteractableUsabilityChanged(int32 ActorID, float NewUsability) const;
	
	TSharedRef<ITableRow> MakeListViewWidget_InteractableData(TSharedPtr<FRTSSavedInteractable> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_InteractableData(TSharedPtr<FRTSSavedInteractable> InItem, ESelectInfo::Type InSelectInfo);

	TSharedRef<ITableRow> MakeListViewWidget_EntityData(TSharedPtr<FRTSSavedWorldUnits> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_EntityData(TSharedPtr<FRTSSavedWorldUnits> InItem, ESelectInfo::Type InSelectInfo);

	TSharedRef<ITableRow> MakeListViewWidget_InventoryOverviewData(TSharedPtr<FUserInventoriesStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_InventoryOverviewData(TSharedPtr<FUserInventoriesStruct> InItem, ESelectInfo::Type InSelectInfo);

	TSharedRef<ITableRow> MakeListViewWidget_ItemData(TSharedPtr<FPDItemNetDatum> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_ItemData(TSharedPtr<FPDItemNetDatum> InItem, ESelectInfo::Type InSelectInfo) const;

	TSharedRef<ITableRow> MakeListViewWidget_ConversationStateData(TSharedPtr<FConversationStateStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_ConversationStateData(TSharedPtr<FConversationStateStruct> InItem, ESelectInfo::Type InSelectInfo);
	
	TSharedRef<ITableRow> MakeListViewWidget_UserMissionTags(TSharedPtr<FUserMissionTagsStruct> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnComponentSelected_UserMissionTags(TSharedPtr<FUserMissionTagsStruct> InItem, ESelectInfo::Type InSelectInfo);	
	
	
	URTSOpenSaveGame* SaveGamePtr = nullptr;
	FRTSSaveData CopiedSaveData;

	// Views
	TArray<TSharedPtr<FPlayerLocationStruct>>    LocationsAsSharedTupleArray;
	TArray<TSharedPtr<FRTSSavedInteractable>>   InteractableAsSharedArray;
	TArray<TSharedPtr<FRTSSavedWorldUnits>>     EntitiesAsSharedArray;
	TArray<TSharedPtr<FUserInventoriesStruct>>   AllUserInventoriesAsSharedTupleArray;
	TArray<TSharedPtr<FConversationStateStruct>> ConversationStatesAsSharedArray;
	TArray<TSharedPtr<FUserMissionTagsStruct>>   UserMissionTagsAsSharedArray;
	

	// Callbacks
	FOnPlayerDataChosen OnPlayerDataChosen{};
	FOnInteractableDataChosen OnInteractableDataChosen{};
	FOnEntityDataChosen OnEntityDataChosen{};
	FOnInventoryOverviewDataChosen OnInventoryOverviewDataChosen{};
	FOnItemDataChosen OnItemDataChosen{};
	FOnConverstionStateDataChosen OnConversationStateDataChosen{};
	FOnMissionTagsDataChosen OnOnMissionTagsDataChosen{};

	UClass* SelectedClass = nullptr;

	// View Tables
	TSharedPtr<STableRow< TSharedPtr<FRTSSavedInteractable>>> InteractableTable;
	TSharedPtr<STableRow< TSharedPtr<FRTSSavedWorldUnits>>> EntityTable;
	TSharedPtr<STableRow< TSharedPtr<FUserInventoriesStruct>>> InventoryTable;
	TSharedPtr<STableRow< TSharedPtr<FConversationStateStruct>>> ConversationStateTable;
	TSharedPtr<STableRow< TSharedPtr<FUserMissionTagsStruct>>> MissionTagsTable;
};

class SRTSOSaveEditorSubmenu : public SCompoundWidget
{
public:
	DECLARE_DELEGATE(FOnValueChange);
	SLATE_BEGIN_ARGS(SRTSOSaveEditorSubmenu)
		: _SubmenuTag(FGameplayTag())
	{}
	SLATE_ARGUMENT(FGameplayTag, SubmenuTag)

	SLATE_EVENT(FOnValueChange, OnValueChanged)
SLATE_END_ARGS()

	/**  */
	SRTSOSaveEditorSubmenu() 
		: SubmenuTag(FGameplayTag::EmptyTag) {}

	/** 
	 * Builds the slate UI for SRTSOSaveEditorSubmenu (as well as caches off any
	 * important data that was supplied as part of the FArguments struct).
	 *
	 * @param  Args	 The set of slate arguments that are used to customize this panel.
	 */
	void Construct(const FArguments& Args);

private:
	TSharedRef<SWidget> MakeSubmenu();
	FText  GetSubmenuTextValue() const;
	FReply OnUseSelectedItemClick();
	
	FGameplayTag SubmenuTag;
	FOnValueChange OnValueChanged;
};


class FRTSSaveEd_InteractableClassFilter : public IClassViewerFilter
{
public:
	/** The meta class for the property that classes must be a child-of. */
	const UClass* ClassPropertyMetaClass = nullptr;

	/** The interface that must be implemented. */
	const UClass* InterfaceThatMustBeImplemented = nullptr;

	/** Whether or not abstract classes are allowed. */
	bool bAllowAbstract = false;

	/** Classes that can be picked */
	TArray<const UClass*> AllowedClassFilters;

	/** Classes that can't be picked */
	TArray<const UClass*> DisallowedClassFilters;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		return IsClassAllowedHelper(InClass);
	}
	
	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InBlueprint, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return IsClassAllowedHelper(InBlueprint);
	}

private:

	template <typename TClass>
	bool IsClassAllowedHelper(TClass InClass)
	{
		bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated) &&
			(bAllowAbstract || !InClass->HasAnyClassFlags(CLASS_Abstract));

		if (bMatchesFlags && InClass->IsChildOf(ClassPropertyMetaClass)
			&& (!InterfaceThatMustBeImplemented || InClass->ImplementsInterface(InterfaceThatMustBeImplemented)))
		{
			auto PredicateFn = [InClass](const UClass* Class)
			{
				return InClass->IsChildOf(Class);
			};

			if (DisallowedClassFilters.FindByPredicate(PredicateFn) == nullptr &&
				(AllowedClassFilters.Num() == 0 || AllowedClassFilters.FindByPredicate(PredicateFn) != nullptr))
			{
				return true;
			}
		}

		return false;
	}
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
