/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "PDInteractCommon.h"
#include "Net/PDItemNetDatum.h"

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SaveGame.h"
#include "CommonActivatableWidget.h"
#include "CommonButtonBase.h"
#include "PDItemCommon.h"
#include "AI/Mass/PDMassFragments.h"
#include "MassEntityConfigAsset.h"
#include "RTSOpenCommon.generated.h"

DECLARE_LOG_CATEGORY_CLASS(PDLog_RTSOConversation, Log, All);
DECLARE_LOG_CATEGORY_CLASS(PDLog_RTSOInteract, Log, All);
DECLARE_LOG_CATEGORY_CLASS(PDLog_RTSO, Log, All);

class APDInteractActor;
class UPDRTSBaseUnit;
class ARTSOInteractableConversationActor;

UENUM()
enum class EPDSaveDataThreadSelector : int8
{
	// Things we want to handle on different threads
	EPlayers = 0,
	EInteractables,
	EEntities,
	EInventories,
	EConversationActors,
	EPlayerConversationProgress,
	EEnd,

	// Needed for other work that does not involve threads, felt unnecessary to keep in another enum when this enum ha all the other fields we are looking for
	EWorldBaseData = -1
};



/**
 * @todo @note: If I have time then write a small mission system, but avoid putting to much time into it
 * @todo @cont: as I am working on a separate repo as-well with a full fledged mission system,
 * @todo @cont: so not worth making two fully fledged ones,
 * @todo @cont: this one well have to be made with some shortcuts in design and implementation
 */
UENUM()
enum ERTSOConversationState
{
	CurrentStateActive    UMETA(DisplayName="Conversation: Current State Active"),
	CurrentStateInactive  UMETA(DisplayName="Conversation: Current State Inactive"),
	CurrentStateCompleted UMETA(DisplayName="Conversation: Current State Completed"),
	Invalid               UMETA(DisplayName="Conversation: Current State INVALID"),
	Valid                 UMETA(DisplayName="Conversation: Current State VALID"),
};

/** @brief Conversation rules, used for mission-progression */
USTRUCT(Blueprintable)
struct FRTSOConversationRules
{
	GENERATED_BODY()
	
	/** @brief Tag for entry data that these rules pertain */
	UPROPERTY(EditAnywhere)
	FGameplayTag EntryTag;
	
	/** @brief StartingState and ActiveState for this conversation instance */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ERTSOConversationState> State = ERTSOConversationState::CurrentStateInactive;

	/** @brief Tags required for this instance to be loaded */
	UPROPERTY(EditAnywhere)
	TArray<FGameplayTag> RequiredTags;

	/** @brief Flag to tell if this conversation is repeatable*/
	UPROPERTY(EditAnywhere)
	bool bCanRepeatConversation = true;

	bool operator==(const FRTSOConversationRules& Other) const
	{
		// we really only care about the entry tag and required tags in comparisons 
		return
			this->EntryTag == Other.EntryTag
			&& this->RequiredTags == Other.RequiredTags;
	}

	bool operator!=(const FRTSOConversationRules& Other) const
	{
		return (*this == Other) == false;
	}
};

/** @brief Conversation base Settings.  */
USTRUCT(Blueprintable)
struct FRTSOConversationMetaProgressionDatum : public FTableRowBase
{
	GENERATED_BODY()

	/**< @brief Starting progression */
	UPROPERTY(EditAnywhere)
	FGameplayTag MissionTag = FGameplayTag{};	
	
	/**< @brief Starting progression */
	UPROPERTY(EditAnywhere)
	int32 BaseProgression = 0;
	
	/** @brief Required tags to progress through the mission phases */
	UPROPERTY(EditAnywhere)
	TArray<FRTSOConversationRules> PhaseRequiredTags;

	bool operator==(const FRTSOConversationMetaProgressionDatum& Other) const
	{
		return 
			this->MissionTag == Other.MissionTag
			&& this->BaseProgression == Other.BaseProgression
			&& this->PhaseRequiredTags == Other.PhaseRequiredTags;
	}

	bool operator!=(const FRTSOConversationMetaProgressionDatum& Other) const
	{
		return (*this == Other) == false;
	}
};


/** @brief Conversation meta progression wrapper, @note this is nasty @todo actually plan design for this after having the initial prototypes finished. */
USTRUCT(Blueprintable)
struct FRTSOConversationMetaProgressionListWrapper
{
	GENERATED_BODY()

	/** @brief Map of progression datum per mission tag */
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, FRTSOConversationMetaProgressionDatum> ProgressionDataMap;
	
	/** @brief Equality comparison, slow */
	bool operator==(const FRTSOConversationMetaProgressionListWrapper& Other) const
	{
		TArray<FGameplayTag> ThisMetaProgressionKeyArray;
		TArray<FGameplayTag> OtherMetaProgressionKeyArray;
		this->ProgressionDataMap.GenerateKeyArray(ThisMetaProgressionKeyArray);
		Other.ProgressionDataMap.GenerateKeyArray(OtherMetaProgressionKeyArray);

		TArray<FRTSOConversationMetaProgressionDatum> ThisMetaProgressionValueArray;
		TArray<FRTSOConversationMetaProgressionDatum> OtherMetaProgressionValueArray;
		this->ProgressionDataMap.GenerateValueArray(ThisMetaProgressionValueArray);
		Other.ProgressionDataMap.GenerateValueArray(OtherMetaProgressionValueArray);	
		
		return 
			ThisMetaProgressionKeyArray == OtherMetaProgressionKeyArray
			&& ThisMetaProgressionValueArray == OtherMetaProgressionValueArray;		
	}

	/** @brief Inequality comparison, slow */
	bool operator!=(const FRTSOConversationMetaProgressionListWrapper& Other) const
	{
		return (*this == Other) == false;
	}
};


/** @brief Save data for inventory items */
USTRUCT(BlueprintType, Blueprintable)
struct FRTSSavedItems
{
	GENERATED_BODY()

	/** @brief Saved items, @todo need to sort this by user/ownerID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPDItemNetDatum> Items;
};

/** @brief Save data for conversation progression actors and players conversation progressions  */
USTRUCT(BlueprintType, Blueprintable)
struct FRTSSavedConversationActorData
{
	GENERATED_BODY()
	
	/** @brief Missions to load from table*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> Missions{};

	/** @brief Location of conversation progression actor it belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	FVector Location{}; 

	/** @brief Actor class of the interactable conversation actor*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	TSoftClassPtr<ARTSOInteractableConversationActor> ActorClassType{};

	/** @brief Progression map, keyed by user (ID) and valued by progression level for the conversation.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32/*ActorID*/, FRTSOConversationMetaProgressionListWrapper/*ProgressionLevel*/> ProgressionPerPlayer; 

	/** @brief Unused. assume it will always be 1.0 for now	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	double Health = 1.0;

	UPROPERTY()
	UClass* _HiddenInstantiatedClass = nullptr;

	// Copy selected class (_HiddenInstantiatedClass) to softobject if not already copied, clear _HiddenInstantiatedClass no matter if it was copied or not 
	void CopySelectedToSoftClass(UClass* PotentialOverrideHiddenClass = nullptr)
	{
		if (PotentialOverrideHiddenClass != nullptr || _HiddenInstantiatedClass == nullptr)
		{
			_HiddenInstantiatedClass = PotentialOverrideHiddenClass;
		}
		
		if (_HiddenInstantiatedClass != nullptr)
		{
			if (TSoftClassPtr<ARTSOInteractableConversationActor>(_HiddenInstantiatedClass) != ActorClassType)
			{
				ActorClassType = TSoftClassPtr<ARTSOInteractableConversationActor>(_HiddenInstantiatedClass);
			}
			_HiddenInstantiatedClass = nullptr;
		}		
	}
	
	/** @brief Equality comparison, slow */
	bool operator==(const FRTSSavedConversationActorData& Other) const
	{
		TArray<int32> ThisProgressionKeyArray;
		TArray<int32> OtherProgressionKeyArray;
		this->ProgressionPerPlayer.GenerateKeyArray(ThisProgressionKeyArray);
		Other.ProgressionPerPlayer.GenerateKeyArray(OtherProgressionKeyArray);

		TArray<FRTSOConversationMetaProgressionListWrapper> ThisProgressionValueArray;
		TArray<FRTSOConversationMetaProgressionListWrapper> OtherProgressionValueArray;
		this->ProgressionPerPlayer.GenerateValueArray(ThisProgressionValueArray);
		Other.ProgressionPerPlayer.GenerateValueArray(OtherProgressionValueArray);		

		// Give a fair amount of slack on the floating points
		return 
			this->ActorClassType == Other.ActorClassType
			&& ThisProgressionKeyArray == OtherProgressionKeyArray
			&& ThisProgressionValueArray == OtherProgressionValueArray
			&& this->Missions.Array() == Other.Missions.Array()
			&& FMath::IsNearlyEqual(this->Health, Other.Health, 0.5f) 
			&& (this->Location - Other.Location).IsNearlyZero(5.f);
	}

	/** @brief Inequality comparison, slow */
	bool operator!=(const FRTSSavedConversationActorData& Other) const
	{
		return (*this == Other) == false;
	}	
	
};

/** @brief Save data for world units/entities*/
USTRUCT(BlueprintType, Blueprintable)
struct FRTSSavedWorldUnits
{
	GENERATED_BODY()

	/** @brief Saved current action for the unit this relates to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	FPDMFragment_Action CurrentAction{};

	/** @brief Saved current location for the unit this relates to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	FVector Location{};

	/** @brief Saved current health for the unit this relates to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	double Health = 0.0;

	/** @brief Saved OwnerID for the unit this relates to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	int32 OwnerID = INDEX_NONE;

	/** @brief Saved selection index the unit this relates to is contained within */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	int32 SelectionIndex = INDEX_NONE;

	/** @brief Tag related to this entity, @done use to load entity config from table  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|SaveGame|Unit")
	FGameplayTag EntityUnitTag; //

	/** @brief Instance ID is used to compare old vs new data */
	UPROPERTY(EditAnywhere, Category = "GameInstance|Widgets")
	FMassEntityHandle InstanceIndex = {INDEX_NONE, INDEX_NONE};

	/** @brief Equality comparison, has certain slack in the health and location comparisons */
	bool operator==(const FRTSSavedWorldUnits& Other) const
	{
		return 
			this->CurrentAction.Reward == Other.CurrentAction.Reward
			&& this->CurrentAction.RewardAmount == Other.CurrentAction.RewardAmount
			&& this->CurrentAction.ActionTag == Other.CurrentAction.ActionTag
			&& this->CurrentAction.OptTargets.IsValidCompound() == Other.CurrentAction.OptTargets.IsValidCompound()
			&& this->InstanceIndex == Other.InstanceIndex
			&& this->EntityUnitTag == Other.EntityUnitTag
			&& FMath::IsNearlyEqual(this->Health, Other.Health, 0.5f) 
			&& (this->Location - Other.Location).IsNearlyZero(5.f);
	}

	/** @brief Inequality comparison, negates the result of the equality comparison */
	bool operator!=(const FRTSSavedWorldUnits& Other) const
	{
		return (*this == Other) == false;
	}	
};

USTRUCT(BlueprintType, Blueprintable)
struct RTSOPEN_API FRTSSaveData
{
	GENERATED_BODY()
	/** @brief Random stream */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	FRandomStream Seeder{0};
	
	/** @brief Total elapsed game-time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	double GameTime = 0.0;

	/** @brief Players locations, keyed by players persistent IDs*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TMap<int32 /*Player Persistent ID*/, FVector> PlayerLocations{}; 

	/** @brief World interactables and their parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TArray<FRTSSavedInteractable> Interactables{};

	/** @brief World entities and their parameters, such as ownerID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TArray<FRTSSavedWorldUnits> EntityUnits{};
	
	/** @brief userid tied to some account id?, in singleplayer keep only one entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TMap<int32, FRTSSavedItems> Inventories{};

	/** @brief Save data for the existing conversation actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TMap<int32 /*ConversationActorID*/, FRTSSavedConversationActorData> ConversationActorState{};

	/** @brief userid tied to some account id?, in singleplayer keep only one entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32 /*PlayerID*/, FGameplayTagContainer> PlayersAndConversationTags{};

	/** @brief Timer Handle, save is dispatched on a timer to throttle any saves */
	FTimerHandle SaveThrottleHandle;
	
};


/** @brief Main save data structure, @todo associate account ID to the persistent ID */
UCLASS(BlueprintType, Blueprintable)
class RTSOPEN_API URTSOpenSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	/** @brief Data struct */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	FRTSSaveData Data;
};

/** @brief @todo move to new file in UI folder */
UCLASS(BlueprintType, Blueprintable)
class UPDTransitionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief Starts a transition animation */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartTransition();
	/** @brief Ends the transition animation */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndTransition();
	
public:
	/** @brief The given transition animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget|BaseTransition", Meta = (BindWidget))
	UWidgetAnimation* TransitionAnimation = nullptr;
};

/** @brief @todo move to new file in UI folder */
UCLASS(BlueprintType, Blueprintable)
class URTSHudWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
};

/** @brief @todo remove, redundant as there are already widgets in the interaction plugin afaik */
UCLASS(BlueprintType, Blueprintable)
class URTSInteractWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/** @brief @todo see note above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget|Base", Meta = (BindWidget))
	UCommonButtonBase* InteractButton = nullptr;
};

/** @brief Data-asset that holds any uasset we want to default to a given godhand settings entry */
UCLASS(BlueprintType, Blueprintable)
class URTSGodhandDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief Curve to source magnification value from. is none exist it reverts to a expression based logarithmic 'curve' */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	UCurveFloat* MagnificationCurve = nullptr;
};

/** @brief Settings table row for the godhand pawn */
USTRUCT(Blueprintable, BlueprintType)
struct FRTSGodhandSettings : public FTableRowBase
{
	GENERATED_BODY()

	/** @brief Dataasset that holds uassets we want to default to this godhand settings entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Godhand|Settings")
	URTSGodhandDataAsset* AssetData = nullptr;
	/** @brief Padding to offset a scalar range the is fed to a sine. This is used when the cursor hovers an actor or entity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Godhand|Settings")
	double TargetPadding = 40.0;
	/** @brief Scalar phase speed, how fast does it ride the sine-wave? This is used when the cursor hovers an actor or entity  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Godhand|Settings")
	double CursorSelectedScalePhaseSpeed = 4.0;	
	/** @brief  Sine intensity. This is used when the cursor hovers an actor or entity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Godhand|Settings")
	double CursorSelectedSinScaleIntensity = 0.3;
	/** @brief This controls how fast the cursor rescales when snapping to a hovered actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Godhand|Settings")
	double SelectionRescaleSpeed = 8.0;
	/** @brief This controls how fast the godhand rotates when using the rotate keys */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Godhand|Settings")
	double RotationRateModifier = 75.0;
	/** @brief This controls how fast the godhand moves to a new buildable camera target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Godhand|Settings")
	double CameraTargetInterpSpeed = 3.0;	
};

/** @brief State struct for the godhand pawn */
USTRUCT(BlueprintType, Blueprintable)
struct FRTSGodhandState
{
	GENERATED_BODY()
	
	/** @brief The currently hovered actor, it looks for objects that inherit from FPDInteractInterface */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Godhand|State")
	AActor* HoveredActor = nullptr;

	/** @brief The worlds agent handler gets assigned to this. This ISM component has logic to handle spawning mass entities, dispatching jobs and such  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Godhand|State")
	UPDRTSBaseUnit* ISMAgentComponent = nullptr;	

	/** @brief This value is calculated based on the current MagnificationStrength */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Godhand|State")
	double MagnificationValue = 0.0;

	/** @brief This value is assigned from the inputvalue of the 'ActionMagnify' input action */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Godhand|State")
	double MagnificationStrength = 0.01;	

	/** @brief This value is assigned from the inputvalue of the 'ActionMagnify' input action, in case we are rotating a held object */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Godhand|State")
	double ScrollAccumulator = 0.0;	

	/* State(s) - Cursor */	
	/** @brief Accumulates the current time elapsed and feeds a sinewave with it */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Godhand|State")
	double AccumulatedPlayTime = 0.0;
	/** @brief The calculated target transform for the godhand,  be it a hovered entity/actor or the mouse projection location */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Godhand|State")
	FTransform TargetTransform{};
	
	/* State(s) - Target */	
	/** @brief Virtually unused @todo Remove within a commit or two, this has lost it's purpose at this point  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Godhand|State")
	FVector WorkUnitTargetLocation{};

	/** @brief Copies the pointer from 'HoveredActor' at the point of triggering the 'ActionWorkerUnit' input action  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Godhand|State")
	AActor* WorkerUnitActionTarget;
	
	/** @brief Currently selected, singular, mass entity. In case we have multiple selected this is often set to an invalid handle as that is handled via selection groups instead */
	UPROPERTY()
	FMassEntityHandle SelectedWorkerUnitHandle{0, 0};
	
	/* State(s) - Pathing */	
	/** @brief This flag is set when we start drawing our niagara path, and is unset when we stop. Used to know if it's safe to overwrite certain properties or not */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Godhand|State")
	bool bUpdatePathOnTick = false;
	
	/** @brief The generated path points for the niagara effect */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Godhand|State")
	TArray<FVector> PathPoints;
	
	/** @brief Rotation direction deque, it increases the head is the current ratation operation,
	 * @note rotate ccw if Element == 1
	 * @note rotate cw  if Element == -1
	 * 
	 * @note A. Get if positive or negative,
	 * @note B. Add Direction to queue
	 * @note C. In tick: rotate +-90 degree in yaw with interpolation
	 * @note D. If rotating direction Positive, then pressing positive again then negative, , final two should cancel eachother out
	 */
	TDeque<TPair<int8, double /*ActualRotationTarget*/>> RotationDeque{};

	/** @brief Amount degrees left of rotation,
	 * @note this has a tendency to 'slide' into an offset when tick is not reliable,
	 * @note fixed it by snapping it to cardinal directions so it never accumulates this offset */
	double CurrentRotationLeft = 0.0;
	
	/** @brief Flag used to avoid overwriting the rotation states for a active rotation */
	bool bIsInRotation = false;
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