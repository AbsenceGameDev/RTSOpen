/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "PDRTSSharedOctree.h"
#include "AI/Mass/PDMassFragments.h"

#include "GameplayTagContainer.h"
#include "PDBuildCommon.h"
#include "Containers/Deque.h"

#include "Subsystems/EngineSubsystem.h"
#include "PDBuilderSubsystem.generated.h"

/** @brief Subsystem to handle octree size changes and to act as a manager for the entity workers */
UCLASS()
class PDRTSBASE_API UPDBuilderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	/** @brief Shorthand to get the subsystem,
	 * @note as the engine will instantiate these subsystem earlier than anything will reasonably call Get()  */
	static UPDBuilderSubsystem* Get();

	void ProcessBuildContextTable(const TSoftObjectPtr<UDataTable>& TablePath);
	/** @brief Loads the worktables when the subsystem initializes during engine startup, far earlier than any world exists.
	 * It uses developer settings (UPDRTSSubsystemSettings) to read and write selected worktable paths via config, and or project settings window  
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	
	/** @brief Bound to the given developer setting. Resolved the paths into actual tables which we cache to via hash maps */
	void OnDeveloperSettingsChanged(UObject* SettingsToChange, FPropertyChangedEvent& PropertyEvent);

	/** @brief Returns the default build context data via it's BuildContext-tag*/
	const FPDBuildContext* GetBuildContextEntry(const FGameplayTag& BuildContextTag);
	/** @brief Returns the default Buildable data via it's Buildable-tag*/
	const FPDBuildableData* GetBuildableData(const FGameplayTag& BuildableTag);
	const FGameplayTag& GetBuildableTagFromData(const FPDBuildableData* BuildableData);
	
	
	/** @brief Queue a removal from the buildable octree */
	void QueueRemoveFromWorldBuildTree(int32 UID);
	/** @brief Pass data between buffers when access to the first buffer is complete */
	void PassOverDataFromQueue();
	
	/** @brief  Processes the data-asset for a given ghost stage */
	static void ProcessGhostStageDataAsset(const AActor* GhostActor, bool bIsStartOfStage, const FPDRTSGhostStageData& SelectedStageData);

	/** @brief  Processes a ghost stage: i.e. has it passed checks to increase stage or finish build? */
	UFUNCTION(BlueprintCallable, Category = "Actor|Ghost")
	static void ProcessGhostStage(AActor* GhostActor, const FGameplayTag& BuildableTag, FPDRTSGhostBuildState& MutableGhostDatum, bool bIsStartOfStage);

	/** @brief Get the max configured duration for a ghost stage to finish if running automatic stage progression without resources */
	UFUNCTION(BlueprintCallable, Category = "Actor|Ghost")
	static double GetMaxDurationGhostStage(const FGameplayTag& BuildableTag, FPDRTSGhostBuildState& MutableGhostDatum);	

	/** @brief Is the stage past final index (final stage) */
	UFUNCTION(BlueprintCallable, Category = "Actor|Ghost")
	static bool IsPastFinalIndex(const FGameplayTag& BuildableTag, FPDRTSGhostBuildState& MutableGhostDatum);	
	
	/** @brief Reserved */
	void WorldInit(const UWorld* World);


	/** @brief Tell the subsystem we won't allow direct changes to WorldBuildActorArrays (Lists of users and their buildable actor arrays),
	 * and that we might be processing select removals of actual buildables  */
	TArray<FPDActorCompound>& BlockMutationOfBuildableTrackingData();
	
	/** @brief Tell the subsystem we may resume making direct changes to WorldBuildActorArrays (Lists of users and their buildable actor arrays),
	 * and that we might be processing select removals of actual buildables.
	 * Calls PassOverDataFromQueue & ProcessQueuedAdditionsAndRemovals_BuildablesArrayTracker to process any changes requested while it was paused*/
	void ResumeMutationOfBuildableTrackingData();

	/** @brief Processes any added users and pointers to their buildable arrays */
	void ProcessQueuedAdditionsAndRemovals_BuildablesArrayTracker();
	
	/** @brief Request to add a user and a pointer to their buildable array */
	void AddBuildActorArray(int32 OwnerID, TArray<AActor*>* OwnerArrayPtr);

	/** @brief Request to add a user and a pointer to their buildable array, via using an FPDActorCompound struct */
	void AddBuildActorArray(FPDActorCompound ActorCompound);

	/** @brief Request to remove a user adn their pointer to their buildable array */
	void RemoveBuildActorArray(int32 OwnerID);
	
public:	

	/** @brief Build context tables held by subsystem for others to fetch */
	UPROPERTY()
	TArray<UDataTable*> BuildContextTables{};

	/** @brief Build queues per player */
	UPROPERTY()
	TMap<int32 /*Player ID*/, FPDBuildQueue_WNoStageCosts> BuildQueues{};	

	/** @brief Mapped for fast access. Mapped upon subsystem loading the developer settings 'UPDRTSSubsystemSettings' */
	TMap<FGameplayTag, const FPDBuildWorker*> GrantedBuildContexts_WorkerTag{};		
	/** @brief Mapped for fast access. Mapped upon subsystem loading the developer settings 'UPDRTSSubsystemSettings' */
	TMap<FGameplayTag, const FPDBuildContext*> BuildContexts_WTag{};	
	/** @brief Mapped for fast access. Mapped upon subsystem loading the developer settings 'UPDRTSSubsystemSettings' */
	TMap<FGameplayTag, const FPDBuildableData*> BuildableData_WTag{};	
	/** @brief Mapped for fast access. Mapped upon subsystem loading the developer settings 'UPDRTSSubsystemSettings' */
	TMap<FPDBuildableData*, FGameplayTag>       BuildableData_WTagReverse{};	

	/** @brief The actual octree our buildable actors will make use of*/
	PD::Mass::Actor::Octree WorldBuildActorOctree;
	TMap<int32 /*UID*/, TSharedPtr<FOctreeElementId2>> ActorsToCells;

	bool bIsProcessingBuildableRemovalQueue = false;
	TDeque<int32 /*UID*/> RemoveBuildableQueue_FirstBuffer{};  // First  buffer, tells our processor twhihc UIDS to flush and remove their cells
	TDeque<int32 /*UID*/> RemoveBuildableQueue_SecondBuffer{}; // Second buffer, while processing first buffer, use this so we don't get race conditions

	// @todo move into developer settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitPingLimitBuilding = 100; // max 100 eligible units may be pinged
	UWorld* TemporaryWorldCache;
	
	/** @brief When false any calls to AddBuildActorArray or RemoveBuildActorArray will resolve into queuing the calls
	 * @note Sets to false while our UPDOctreeProcessor::Execute is running our auxiliary async task  */
	bool bCanEditActorArray = true;
	/** @brief  Actors to build */
	TArray<FPDActorCompound> WorldBuildActorArrays{};
	/** @brief  Reserved */
	TArray<FVector> WorldBuildableLocationList{};
	/** @brief  Mapping Array index (of 'WorldBuildActorArrays') to owner ID for use in our queued additions & removals  */
	TMap<int32, int32> IndexToID{};
	/** @brief  Backmapping OwnerID to arraty index, for use in our queued additions & removals  */
	TMap<int32, int32> IDToIndex{};
	/** @brief  Queued removals of user actor arrays, removals requested while our auxiliary asynctask in 'UPDOctreeProcessor::Execute' is iterating 'WorldBuildActorArrays' */
	TDeque<int32> QueuedRemovals_BuildablesArrays{};
	/** @brief  Queued additions of user actor arrays, additions requested while our auxiliary asynctask in 'UPDOctreeProcessor::Execute' is iterating 'WorldBuildActorArrays'  */
	TDeque<TTuple<int32, void*>> QueuedAdditions_BuildablesArrayPointers{};
	
	/** @brief Tracking worlds that has been setup with this WorldOctree.  */
	TMap<void*, bool> WorldsWithOctrees{};

	/** @brief  (Build system) Query Shape structure. Reserved. @todo @refactor assess if needed */
	FPDOctreeUserQuery OctreeBuildSystemEntityQuery{}; 
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