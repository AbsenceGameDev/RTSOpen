/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "PDBuilderSubsystem.h"
#include "PDRTSCommon.h"

#include "NiagaraComponent.h"
#include "PDBuildCommon.h"
#include "Interfaces/PDRTSBuildableGhostInterface.h"

void UPDBuilderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDBuilderSubsystemSettings>()->BuildContextTables)
	{
		ProcessBuildContextTable(TablePath);
	}

	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDBuilderSubsystemSettings>()->BuildWorkerTables)
	{
		ProcessBuildContextTable(TablePath);
	}

	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDBuilderSubsystemSettings>()->BuildActionContextTables)
	{
		ProcessBuildContextTable(TablePath);
	}

	GetMutableDefault<UPDBuilderSubsystemSettings>()->OnSettingChanged().AddLambda(
		[&](UObject* SettingsToChange, FPropertyChangedEvent& PropertyEvent)
		{
			OnDeveloperSettingsChanged(SettingsToChange,PropertyEvent);
		});
}

const TCHAR* StrCtxt_ProcessBuildData = *FString("UPDDBuilderSubsystem::ProcessBuildContextTable");

void UPDBuilderSubsystem::ProcessBuildContextTable(const TSoftObjectPtr<UDataTable>& TablePath)
{
	UDataTable* BuildContextTable = TablePath.LoadSynchronous();
	if (BuildContextTable == nullptr) { return; }
		
	BuildContextTables.Emplace(BuildContextTable);
	if (BuildContextTable->RowStruct == FPDBuildContext::StaticStruct())
	{
		TArray<FPDBuildContext*> BuildContexts;
		BuildContextTable->GetAllRows<FPDBuildContext>(StrCtxt_ProcessBuildData, BuildContexts);
		for (const FPDBuildContext* BuildContext : BuildContexts)
		{
			for (const FDataTableRowHandle& BuildableDatum : BuildContext->BuildablesData)
			{
				FPDBuildable* Buildable = BuildableDatum.GetRow<FPDBuildable>("");
				BuildableData_WTag.Emplace(Buildable->BuildableTag, &Buildable->BuildableData);
				BuildableData_WTagReverse.Emplace(&Buildable->BuildableData, Buildable->BuildableTag);

				Buildable_WClass.Emplace(Buildable->BuildableData.ActorToSpawn, Buildable);
			}
			BuildContexts_WTag.Emplace(BuildContext->ContextTag, BuildContext);
		}
	}
	
	if (BuildContextTable->RowStruct == FPDBuildWorker::StaticStruct())
	{
		TArray<FPDBuildWorker*> WorkerGrantedContexts;
		BuildContextTable->GetAllRows<FPDBuildWorker>(StrCtxt_ProcessBuildData, WorkerGrantedContexts);
		for (const FPDBuildWorker* GrantedContext : WorkerGrantedContexts)
		{
			GrantedBuildContexts_WorkerTag.Emplace(GrantedContext->WorkerType, GrantedContext);
		}
	}

	if (BuildContextTable->RowStruct == FPDBuildActionContext::StaticStruct())
	{
		TArray<FPDBuildActionContext*> GrantedContexts;
		BuildContextTable->GetAllRows<FPDBuildActionContext>(StrCtxt_ProcessBuildData, GrantedContexts);
		for (const FPDBuildActionContext* GrantedContext : GrantedContexts)
		{
			for (const FDataTableRowHandle& ActionDatum : GrantedContext->ActionData)
			{
				FPDBuildAction* Action = ActionDatum.GetRow<FPDBuildAction>("");
				ActionData_WTag.Emplace(Action->ActionTag, Action);
			}			
			
			GrantedActionContexts_KeyedByBuildableTag.Emplace(GrantedContext->ContextTag, GrantedContext);
		}
	}	
}

const FPDBuildContext* UPDBuilderSubsystem::GetBuildContextEntry(const FGameplayTag& BuildContextTag)
{
	return BuildContexts_WTag.Contains(BuildContextTag) ? *BuildContexts_WTag.Find(BuildContextTag) : nullptr;
}

const FPDBuildable* UPDBuilderSubsystem::GetBuildableFromClass(const TSubclassOf<AActor> Class)
{
	return Buildable_WClass.Contains(Class) ? *Buildable_WClass.Find(Class) : nullptr;
}

const FPDBuildable* UPDBuilderSubsystem::GetBuildableFromClassStatic(TSubclassOf<AActor> Class)
{
	UPDBuilderSubsystem* BuilderSubsystem = UPDBuilderSubsystem::Get();
	return BuilderSubsystem->Buildable_WClass.Contains(Class) ? *BuilderSubsystem->Buildable_WClass.Find(Class) : nullptr;
}

const FPDBuildableData* UPDBuilderSubsystem::GetBuildableData(const FGameplayTag& BuildableTag)
{
	return BuildableData_WTag.Contains(BuildableTag) ? *BuildableData_WTag.Find(BuildableTag) : nullptr;
}

const FGameplayTag& UPDBuilderSubsystem::GetBuildableTagFromData(const FPDBuildableData* BuildableData)
{
	return BuildableData_WTagReverse.Contains(BuildableData) ? *BuildableData_WTagReverse.Find(BuildableData) : FGameplayTag::EmptyTag;
}

void UPDBuilderSubsystem::QueueRemoveFromWorldBuildTree(int32 UID)
{
	// FILO
	if (bIsProcessingBuildableRemovalQueue == false)
	{
		RemoveBuildableQueue_FirstBuffer.EmplaceFirst(UID);
		return;
	}

	RemoveBuildableQueue_SecondBuffer.EmplaceFirst(UID);
}

void UPDBuilderSubsystem::PassOverDataFromQueue()
{
	RemoveBuildableQueue_FirstBuffer = RemoveBuildableQueue_SecondBuffer;
	RemoveBuildableQueue_SecondBuffer.Empty();
}

UPDBuilderSubsystem* UPDBuilderSubsystem::Get()
{
	return GEngine->GetEngineSubsystem<UPDBuilderSubsystem>();
}

void UPDBuilderSubsystem::ProcessGhostStageDataAsset(const AActor* GhostActor, const bool bIsStartOfStage, const FPDRTSGhostStageData& SelectedStageData)
{
	if (GhostActor == nullptr || GhostActor->IsValidLowLevelFast() == false)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("UPDDBuilderSubsystem::ProcessSingleGhostStage() -- GhostActor is invalid"));
		return;
	}

	// @todo use interface and allow actor to decide which mesh component they should update, they may have several 
	// FVector ActorLocation = GhostActor->GetActorLocation();
	UStaticMeshComponent* ActorMeshComp = GhostActor->GetComponentByClass<UStaticMeshComponent>(); 
	UNiagaraComponent* NiagaraComp = GhostActor->GetComponentByClass<UNiagaraComponent>();

	if (ActorMeshComp == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("UPDDBuilderSubsystem::ProcessSingleGhostStage(Actor: %s, Data: %s) -- Found no static mesh component on actor. \n ^ @todo set up interface and call that instead to allow the user to select the mesh component"), *GhostActor->GetName(), SelectedStageData.StageDA != nullptr ? *SelectedStageData.StageDA->GetName() : *FString("N/A"));
	}
	if (NiagaraComp == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("UPDDBuilderSubsystem::ProcessSingleGhostStage(Actor: %s, Data: %s) -- Found no Niagara component on actor. \n ^ @todo set up interface and call that instead to allow the user to select the mesh component "), *GhostActor->GetName(), SelectedStageData.StageDA != nullptr ? *SelectedStageData.StageDA->GetName() : *FString("N/A"));
	}
	
	
	bool bCanApplyMeshUpdate = false;
	switch (SelectedStageData.StageMesh_ApplyBehaviour)
	{
	case EPDRTSGhostStageBehaviour::EOnStart:
		bCanApplyMeshUpdate = bIsStartOfStage;
		break;
	case EPDRTSGhostStageBehaviour::EOnEnd:
		bCanApplyMeshUpdate = bIsStartOfStage == false;
		break;
	}
	
	bool bCanApplyVFX = false;
	switch (SelectedStageData.StageVFX_ApplyBehaviour)
	{
	case EPDRTSGhostStageBehaviour::EOnStart:
		bCanApplyVFX = bIsStartOfStage;
		break;
	case EPDRTSGhostStageBehaviour::EOnEnd:
		bCanApplyVFX = bIsStartOfStage == false;
		break;
	}
	
	bCanApplyMeshUpdate &= ActorMeshComp != nullptr;
	if (bCanApplyMeshUpdate)
	{
		// @todo hook some event here where users can control mesh transition by other means 
		ActorMeshComp->SetStaticMesh(SelectedStageData.StageDA->StageGhostMesh);
	}

	bCanApplyVFX &= NiagaraComp != nullptr;
	if (bCanApplyVFX)
	{
		// @todo hook some event here where users can control vfx transition by other means 
		NiagaraComp->SetAsset(SelectedStageData.StageDA->StageGhostNiagaraSystem);
		NiagaraComp->ActivateSystem(true);
	}
}

void UPDBuilderSubsystem::ProcessGhostStage(
	AActor*                     GhostActor,
	const FGameplayTag&         BuildableTag,
	FPDRTSGhostBuildState& MutableGhostDatum,
	bool                        bIsStartOfStage)
{
	const UPDBuilderSubsystem* NonStaticSelf = UPDBuilderSubsystem::Get();
	
	if (NonStaticSelf->BuildableData_WTag.Contains(BuildableTag)) { return; }
	
	const FPDBuildableData* BuildableData = NonStaticSelf->BuildableData_WTag.FindRef(BuildableTag);
	const FPDRTSGhostDatum& GhostData = BuildableData->GhostData;
	if (GhostData.StageAssets.IsValidIndex(MutableGhostDatum.CurrentStageIdx))
	{
		ProcessGhostStageDataAsset(GhostActor, bIsStartOfStage, GhostData.StageAssets[MutableGhostDatum.CurrentStageIdx]);
		if (bIsStartOfStage == false) { MutableGhostDatum.CurrentStageIdx++; }
		
		if (MutableGhostDatum.CurrentStageIdx > GhostData.StageAssets.Num())
		{
			IPDRTSBuildableGhostInterface::Execute_OnSpawnedAsMain(GhostActor, BuildableTag);
		}			
	}
}

double UPDBuilderSubsystem::GetMaxDurationGhostStage(
	const FGameplayTag&         BuildableTag,
	FPDRTSGhostBuildState& MutableGhostDatum)
{
	const UPDBuilderSubsystem* NonStaticSelf = UPDBuilderSubsystem::Get();
	
	if (NonStaticSelf->BuildableData_WTag.Contains(BuildableTag)) { return -1.0; }
	
	const FPDBuildableData* BuildableData = NonStaticSelf->BuildableData_WTag.FindRef(BuildableTag);
	const FPDRTSGhostDatum& GhostData = BuildableData->GhostData;
	
	if (GhostData.StageAssets.IsValidIndex(MutableGhostDatum.CurrentStageIdx))
	{
		return GhostData.StageAssets[MutableGhostDatum.CurrentStageIdx].MaxDurationForStage;
	}

	return -1.0;
}

bool UPDBuilderSubsystem::IsPastFinalIndex(const FGameplayTag& BuildableTag, FPDRTSGhostBuildState& MutableGhostDatum)
{
	const UPDBuilderSubsystem* NonStaticSelf = UPDBuilderSubsystem::Get();
	if (NonStaticSelf->BuildableData_WTag.Contains(BuildableTag)) { return false; }
	
	const FPDBuildableData* BuildableData = NonStaticSelf->BuildableData_WTag.FindRef(BuildableTag);
	const FPDRTSGhostDatum& GhostData = BuildableData->GhostData;
	if (MutableGhostDatum.CurrentStageIdx >= GhostData.StageAssets.Num())
	{
		return true;
	}

	return false;	
}

void UPDBuilderSubsystem::WorldInit(const UWorld* World)
{
	check(World);
	WorldBuildActorArrays.Empty();
	WorldBuildableLocationList.Empty();
}

void UPDBuilderSubsystem::OnDeveloperSettingsChanged(UObject* SettingsToChange, FPropertyChangedEvent& PropertyEvent)
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(PropertyEvent.Property);
	if (ArrayProperty == nullptr) { return; }

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(ArrayProperty->Inner);
	if(ObjectProperty == nullptr) { return; }
	
	if(ObjectProperty->PropertyClass != UDataTable::StaticClass()) { return; }
	
	BuildContextTables.Empty();
	for(const TSoftObjectPtr<UDataTable>& TablePath : Cast<UPDBuilderSubsystemSettings>(SettingsToChange)->BuildContextTables)
	{
		ProcessBuildContextTable(TablePath);
	}

	GrantedBuildContexts_WorkerTag.Empty();
	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDBuilderSubsystemSettings>()->BuildWorkerTables)
	{
		ProcessBuildContextTable(TablePath);
	}		
	
}

TArray<FPDActorCompound>& UPDBuilderSubsystem::BlockMutationOfBuildableTrackingData()
{
	bCanEditActorArray = false;
	bIsProcessingBuildableRemovalQueue = true;
	return WorldBuildActorArrays;
}

void UPDBuilderSubsystem::ResumeMutationOfBuildableTrackingData()
{
	bCanEditActorArray = true;
	
	RemoveBuildableQueue_FirstBuffer.Empty();
	bIsProcessingBuildableRemovalQueue = false;
	PassOverDataFromQueue();
	
	// Processing FILO deque
	ProcessQueuedAdditionsAndRemovals_BuildablesArrayTracker();
	return;	
}

void UPDBuilderSubsystem::ProcessQueuedAdditionsAndRemovals_BuildablesArrayTracker()
{
	for (int32 Step = QueuedAdditions_BuildablesArrayPointers.Num(); Step > 0;)
	{
		const TTuple<int32, void*>& Top = QueuedAdditions_BuildablesArrayPointers.Last();
		const int32 QueuedOwnerID = Top.Get<0>();
		TArray<AActor*>* QueuedArrayPtr = static_cast<TArray<AActor*>*>(Top.Get<1>());

		AddBuildActorArray(QueuedOwnerID,  QueuedArrayPtr);
		QueuedAdditions_BuildablesArrayPointers.PopLast();
		Step--;
	}

	for (int32 Step = QueuedRemovals_BuildablesArrays.Num(); Step > 0;)
	{
		const int32 QueuedOwnerID = QueuedRemovals_BuildablesArrays.Last();
		RemoveBuildActorArray(QueuedOwnerID);
		QueuedRemovals_BuildablesArrays.PopLast();
		Step--;
	}		
}

void UPDBuilderSubsystem::AddBuildActorArray(int32 OwnerID, TArray<AActor*>* OwnerArrayPtr)
{
	if (bCanEditActorArray == false)
	{
		QueuedAdditions_BuildablesArrayPointers.EmplaceFirst(TTuple<int32, void*>{OwnerID, OwnerArrayPtr});
		return;
	}
		
	int32 Idx = WorldBuildActorArrays.AddUnique(FPDActorCompound{OwnerID, OwnerArrayPtr});
	IndexToID.Emplace(Idx, OwnerID);
	IDToIndex.Emplace(OwnerID, Idx);
	
	// // Iterate WorldBuildActorArrays and put it's entries into WorldBuildableLocationList
	// WorldBuildableLocationList.EmplaceAt(Idx);
}

void UPDBuilderSubsystem::AddBuildActorArray(FPDActorCompound ActorCompound)
{
	if (bCanEditActorArray == false)
	{
		QueuedAdditions_BuildablesArrayPointers.EmplaceFirst(TTuple<int32, void*>(ActorCompound.OwnerID, ActorCompound.WorldActorsPtr));
		return;
	}

	int32 Idx = WorldBuildActorArrays.AddUnique(ActorCompound);
	IndexToID.Emplace(Idx, ActorCompound.OwnerID);
	IDToIndex.Emplace(ActorCompound.OwnerID, Idx);

	// // Iterate WorldBuildActorArrays and put it's entries into WorldBuildableLocationList
	// WorldBuildableLocationList.EmplaceAt(Idx);
}

void UPDBuilderSubsystem::RemoveBuildActorArray(int32 OwnerID)
{
	if (bCanEditActorArray == false)
	{
		QueuedRemovals_BuildablesArrays.EmplaceFirst(OwnerID);
		return;
	}
		
	if (IDToIndex.Contains(OwnerID) == false) { return; } 
		
	WorldBuildActorArrays.RemoveAt(IDToIndex.FindRef(OwnerID));

	IndexToID.Remove(IDToIndex.FindRef(OwnerID));
	IDToIndex.Remove(OwnerID);
}

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
