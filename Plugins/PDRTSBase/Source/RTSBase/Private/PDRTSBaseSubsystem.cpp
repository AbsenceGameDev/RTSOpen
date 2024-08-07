﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "PDRTSBaseSubsystem.h"
#include "PDRTSCommon.h"

#include "DelayAction.h"
#include "MassCommonFragments.h"
#include "MassCrowdRepresentationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassRepresentationTypes.h"
#include "MassVisualizationComponent.h"
#include "MassVisualizer.h"
#include "NavigationSystem.h"
#include "PDBuilderSubsystem.h"
#include "Interfaces/PDRTSBuildableGhostInterface.h"
#include "Pawns/PDRTSBaseUnit.h"

struct FTransformFragment;
class UMassCrowdRepresentationSubsystem;
class AMassVisualizer;

void UPDRTSBaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAndProcessTables();
}

void UPDRTSBaseSubsystem::LoadAndProcessTables()
{
	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDRTSSubsystemSettings>()->WorkTables)
	{
		UDataTable* ResolvedTable = TablePath.LoadSynchronous();
		WorkTables.Emplace(ResolvedTable);

		ResolvedTable->OnDataTableChanged().AddLambda([&]() { ProcessTables(); });
	}

	GetMutableDefault<UPDRTSSubsystemSettings>()->OnSettingChanged().AddLambda(
		[&](UObject* SettingsToChange, FPropertyChangedEvent& PropertyEvent)
		{
			OnDeveloperSettingsChanged(SettingsToChange,PropertyEvent);
		});

	ProcessTables();
}

void UPDRTSBaseSubsystem::DispatchOctreeGeneration()
{
	const FLatentActionInfo DelayInfo{0,0, TEXT("SetupOctree"), this};
	FLatentActionManager& LatentActionManager = GetWorld()->GetLatentActionManager();
	if (LatentActionManager.FindExistingAction<FDelayAction>(DelayInfo.CallbackTarget, DelayInfo.UUID) == nullptr)
	{
		LatentActionManager.AddNewAction(DelayInfo.CallbackTarget, DelayInfo.UUID, new FDelayAction(10.0f, DelayInfo));
	}
}

void UPDRTSBaseSubsystem::ProcessTables()
{
	ProcessFailCounter++;
	if (WorkTables.IsEmpty())
	{
		const FString BuildString = "UPDRTSBaseSubsystem::ProcessTables -- "
		+ FString::Printf(TEXT("\n 'WorkTables' array is empty. Is not able to process data"));
		UE_LOG(PDLog_RTSBase, Error, TEXT("%s"), *BuildString);
		
		return;
	}
	
	for (const UDataTable* Table : WorkTables)
	{
		if (Table == nullptr
			|| Table->IsValidLowLevelFast() == false
			|| Table->RowStruct != FPDWorkUnitDatum::StaticStruct())
		{
			continue;
		}

		TArray<FPDWorkUnitDatum*> Rows;
		Table->GetAllRows("", Rows);
		
		TArray<FName> RowNames = Table->GetRowNames();
		for (const FName& Name : RowNames)
		{
			const FPDWorkUnitDatum* DefaultDatum = Table->FindRow<FPDWorkUnitDatum>(Name,"");
			check(DefaultDatum != nullptr) // This should never be nullptr

			const FGameplayTag& JobTag = DefaultDatum->JobTag;

			if (JobTag.IsValid() == false)
			{
				const FString BuildString = "UPDRTSBaseSubsystem::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add work/job on row (%s) Which does not have a valid gameplay tag. Skipping processing entry"), *Name.ToString());
				UE_LOG(PDLog_RTSBase, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			
			// @note If duplicates, ignore duplicate and output errors to screen and to log
			if (TagToJobMap.Contains(JobTag))
			{
				const UDataTable* RetrievedTable = TagToTable.FindRef(JobTag);
				
				const FString BuildString = "UPDInventorySubsystem::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add item(%s) which has already been added by previous table(%s)."),
						*JobTag.GetTagName().ToString(), RetrievedTable != nullptr ? *RetrievedTable->GetName() : *FString("INVALID TABLE"));
				UE_LOG(PDLog_RTSBase, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			
			TagToJobMap.Emplace(DefaultDatum->JobTag) = DefaultDatum;
			NameToTagMap.Emplace(Name) = JobTag;
			TagToNameMap.Emplace(JobTag) = Name;
			TagToTable.Emplace(JobTag) = Table;
		}
	}
	ProcessFailCounter = 0;
	bHasProcessedTables = true;
}

void UPDRTSBaseSubsystem::RequestNavpathGenerationForSelectionGroup(
	const int32 OwnerID,
	const int32 SelectionGroup,
	const FVector& SelectionCenter,
	const FPDTargetCompound& TargetCompound)
{
	if (SelectionGroup != INDEX_NONE)
	{
		const FVector TargetLocation =
			TargetCompound.ActionTargetAsActor != nullptr ? TargetCompound.ActionTargetAsActor->GetActorLocation()
			: EntityManager->IsEntityValid(TargetCompound.ActionTargetAsEntity) ? EntityManager->GetFragmentDataPtr<FTransformFragment>(TargetCompound.ActionTargetAsEntity)->GetTransform().GetLocation()
			: TargetCompound.ActionTargetAsLocation.Get();

		const UNavigationPath* Navpath = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), SelectionCenter, TargetLocation);
		SelectionGroupNavData.FindOrAdd(OwnerID).SelectionGroupNavData.FindOrAdd(SelectionGroup) = Navpath;
		DirtySharedData.Emplace(OwnerID, SelectionGroup);
		// bGroupPathsDirtied = true;
	}	
}

const FPDWorkUnitDatum* UPDRTSBaseSubsystem::GetWorkEntry(const FGameplayTag& JobTag)
{
	if (bHasProcessedTables == false && ProcessFailCounter < 2) { ProcessTables(); }
	
	return TagToJobMap.Contains(JobTag) ? TagToJobMap.FindRef(JobTag) : nullptr;
}

const FPDWorkUnitDatum* UPDRTSBaseSubsystem::GetWorkEntry(const FName& JobRowName)
{
	if (bHasProcessedTables == false && ProcessFailCounter < 2) { ProcessTables(); }
	
	const FGameplayTag& JobTag = NameToTagMap.Contains(JobRowName) ? NameToTagMap.FindRef(JobRowName) : FGameplayTag::EmptyTag;
	return GetWorkEntry(JobTag);
}

TArray<FMassEntityHandle> UPDRTSBaseSubsystem::FindIdleEntitiesOfType(TArray<FGameplayTag> EligibleEntityTypes, const AActor* ActorToBuild, int32 OwnerID)
{
	TArray<FMassEntityHandle> RetArray{};

	if (ensure(ActorToBuild != nullptr) == false)
	{
		return RetArray;
	}

	const UPDRTSBaseSubsystem* RTSBaseSubsystem = Get();
	const UPDBuilderSubsystem* BuilderSubsystem = UPDBuilderSubsystem::Get();
	const bool bIsBuildableGhost = ActorToBuild->GetClass()->ImplementsInterface(UPDRTSBuildableGhostInterface::StaticClass());
	if (bIsBuildableGhost == false || RTSBaseSubsystem->WorldToEntityHandler.Contains(ActorToBuild->GetWorld()) == false)
	{
		return RetArray;
	}

	// 1. Get cell of related octree
	const FPDActorOctreeCell& Cell = BuilderSubsystem->WorldBuildActorOctree.GetElementById(*BuilderSubsystem->ActorsToCells.FindRef(ActorToBuild->GetUniqueID()).Get());
	// 2. Iterate that cells entities, pick max 50 that are idle and eligible
	TDeque<FMassEntityHandle> HandlesCopy = Cell.IdleUnits;
	for (const FMassEntityHandle& EntityHandle : HandlesCopy)
	{
		const UWorld* World = ActorToBuild->GetWorld();
		if (RTSBaseSubsystem->EntityManager->IsEntityValid(EntityHandle) == false
			|| RTSBaseSubsystem->WorldToEntityHandler.Contains(World) == false)
		{
			continue;
		}

		for (const FGameplayTag& EligibleType : EligibleEntityTypes)
		{
			// 3. Only select those of requested type(s)
			const FPDMFragment_RTSEntityBase* EntityBase = RTSBaseSubsystem->EntityManager->GetFragmentDataPtr<FPDMFragment_RTSEntityBase>(EntityHandle);
			if (EntityBase->EntityType != EligibleType) { continue; }
			
			RetArray.Emplace(EntityHandle);
			break;
		}
	}
	return RetArray;
}

UPDRTSBaseSubsystem* UPDRTSBaseSubsystem::Get()
{
	return GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
}

void UPDRTSBaseSubsystem::AssociateArchetypeWithConfigAsset(const FMassArchetypeHandle& Archetype, const TSoftObjectPtr<UMassEntityConfigAsset>& EntityConfig)
{
	ConfigAssociations.FindOrAdd(Archetype) = EntityConfig;
}

TSoftObjectPtr<UMassEntityConfigAsset> UPDRTSBaseSubsystem::GetConfigAssetForArchetype(const FMassArchetypeHandle& Archetype)
{
	return ConfigAssociations.Contains(Archetype) ? *ConfigAssociations.Find(Archetype) : TSoftObjectPtr<UMassEntityConfigAsset>{nullptr};
}

void UPDRTSBaseSubsystem::WorldInit(const UWorld* World)
{
	check(World)
	EntityManager = &World->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();	
}

void UPDRTSBaseSubsystem::OnDeveloperSettingsChanged(UObject* SettingsToChange, FPropertyChangedEvent& PropertyEvent)
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(PropertyEvent.Property);
	if (ArrayProperty == nullptr) { return; }

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(ArrayProperty->Inner);
	if(ObjectProperty == nullptr) { return; }
	
	if(ObjectProperty->PropertyClass != UDataTable::StaticClass()) { return; }

	WorkTables.Empty(); // clear array and refill with edited properties.
	for(const TSoftObjectPtr<UDataTable>& TablePath : Cast<UPDRTSSubsystemSettings>(SettingsToChange)->WorkTables)
	{
		UDataTable* ResolvedTable = TablePath.LoadSynchronous();
		WorkTables.Emplace(ResolvedTable);
		
		ResolvedTable->OnDataTableChanged().AddLambda([&]() { ProcessTables(); });
	}

	ProcessTables();
}


// Unseemly matters, avert your gaze
using MassVisRepType = TAccessorTypeHandler<UMassRepresentationSubsystem, TObjectPtr<AMassVisualizer>>; 
template struct TTagPrivateMember<MassVisRepType, &UMassCrowdRepresentationSubsystem::Visualizer>;

using VisualInfoTag = TAccessorTypeHandler<FMassInstancedStaticMeshInfoArrayView, TArrayView<FMassInstancedStaticMeshInfo>>; 
template struct TTagPrivateMember<VisualInfoTag, &FMassInstancedStaticMeshInfoArrayView::InstancedStaticMeshInfos>;

using ISMTag = TAccessorTypeHandler<FMassInstancedStaticMeshInfo, TArray<TObjectPtr<UInstancedStaticMeshComponent>>>; 
template struct TTagPrivateMember<ISMTag, &FMassInstancedStaticMeshInfo::InstancedStaticMeshComponents>;

const TArray<TObjectPtr<UInstancedStaticMeshComponent>> FailDummy{};
const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& UPDRTSBaseSubsystem::GetMassISMs(const UWorld* InWorld)
{
	const UMassCrowdRepresentationSubsystem* RepresentationSubsystem = UWorld::GetSubsystem<UMassCrowdRepresentationSubsystem>(InWorld);
	if (RepresentationSubsystem == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("GetMassISMs Fail - No RepSubsystem"))
		return FailDummy;
	}

	const AMassVisualizer* MassVisualizer = (*RepresentationSubsystem).*TPrivateAccessor<MassVisRepType>::TypeValue;
	if (MassVisualizer == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("GetMassISMs Fail - No MassVisualizer"))
		return FailDummy;
	}
	
	UMassVisualizationComponent& MassVisualization = MassVisualizer->GetVisualizationComponent();
	const FMassInstancedStaticMeshInfoArrayView VisualInfoView = MassVisualization.GetMutableVisualInfos();
	const TArrayView<FMassInstancedStaticMeshInfo>& InstancedStaticMeshInfos = VisualInfoView.*TPrivateAccessor<VisualInfoTag>::TypeValue;
	for (FMassInstancedStaticMeshInfo& MeshInfo : InstancedStaticMeshInfos)
	{
		return MeshInfo.*TPrivateAccessor<ISMTag>::TypeValue;
	}
	return FailDummy;
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
