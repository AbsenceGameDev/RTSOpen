/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "PDRTSBaseSubsystem.h"

#include "DelayAction.h"
#include "MassCommonFragments.h"
#include "MassCrowdRepresentationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassRepresentationTypes.h"
#include "MassVisualizationComponent.h"
#include "MassVisualizer.h"
#include "NavigationSystem.h"
#include "NiagaraComponent.h"
#include "Interfaces/PDRTSBuildableGhostInterface.h"
#include "Interfaces/PDRTSBuilderInterface.h"
#include "Pawns/PDRTSBaseUnit.h"
#include "RTSBase/Classes/PDRTSCommon.h"

struct FTransformFragment;
class UMassCrowdRepresentationSubsystem;
class AMassVisualizer;

void UPDRTSBaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDRTSSubsystemSettings>()->WorkTables)
	{
		WorkTables.Emplace(TablePath.LoadSynchronous());
	}

	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDRTSSubsystemSettings>()->BuildContextTables)
	{
		ProcessBuildContextTable(TablePath);
	}

	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UPDRTSSubsystemSettings>()->BuildWorkerTables)
	{
		ProcessBuildContextTable(TablePath);
	}		

	GetMutableDefault<UPDRTSSubsystemSettings>()->OnSettingChanged().AddLambda(
		[&](UObject* SettingsToChange, FPropertyChangedEvent& PropertyEvent)
		{
			OnDeveloperSettingsChanged(SettingsToChange,PropertyEvent);
		});
}

const TCHAR* StrCtxt_ProcessBuildData = *FString("UPDRTSBaseSubsystem::ProcessBuildContextTable");

FPDEntityPingDatum::FPDEntityPingDatum(uint32 InInstanceID)
{
	InstanceID = InInstanceID;
}

FPDEntityPingDatum::FPDEntityPingDatum(AActor* InWorldActor, const FGameplayTag& InJobTag)
	: WorldActor(InWorldActor)
	, JobTag(InJobTag)
{
	// World actor or job-tag invalid
	if ((ensure(WorldActor != nullptr) && ensure(JobTag.IsValid())) == false) { return; }

	// No Owner ID supplied in this ctor, call another ctor which supplies this to avoid this check
	check(WorldActor->GetOwner<APawn>() == nullptr && WorldActor->GetOwner<AController>() == nullptr);

	InstanceID = WorldActor->GetUniqueID();
	Unsafe_SetOwnerIDFromOwner();
}

FPDEntityPingDatum::FPDEntityPingDatum(AActor* InWorldActor, const FGameplayTag& InJobTag, int32 InAltID, double InInterval, int32 InMaxCountIntervals)
	: WorldActor(InWorldActor)
	, JobTag(InJobTag)
	, OwnerID(InAltID)
	, Interval(InInterval)
	, MaxCountIntervals(InMaxCountIntervals)
{
	// World actor or job-tag invalid
	if ((ensure(WorldActor != nullptr) && ensure(JobTag.IsValid())) == false) { return; }
	
	InstanceID = WorldActor->GetUniqueID();
	if (WorldActor->GetOwner<APawn>() == nullptr && WorldActor->GetOwner<AController>() == nullptr)
	{
		// Log error message
		return;
	}

	if (OwnerID != INDEX_NONE) { return; }

	Unsafe_SetOwnerIDFromOwner();	
}

FPDEntityPingDatum::~FPDEntityPingDatum()
{
	WorldActor = nullptr;
}

#define MASK_BYTE(Value, ByteIdx) (uint8)(((Value) >> (ByteIdx*8)) & 0xffff)
#define RESTORE_BYTE(Bytes, ByteIdx) ((((uint32)Bytes[ByteIdx]) << (ByteIdx*8)))

TArray<uint8> FPDEntityPingDatum::ToBytes() const
{
	const uint32 Bytes = GetTypeHash(*this);
	TArray<uint8> PingHashBytes = {MASK_BYTE(Bytes, 0), MASK_BYTE(Bytes, 1), MASK_BYTE(Bytes, 2), MASK_BYTE(Bytes, 3)};
	return PingHashBytes;
}

void FPDEntityPingDatum::FromBytes(const TArray<uint8>&& Bytes)
{
	InstanceID = RESTORE_BYTE(Bytes, 0) | RESTORE_BYTE(Bytes, 1) | RESTORE_BYTE(Bytes, 2) | RESTORE_BYTE(Bytes, 3);
}

void FPDEntityPingDatum::Unsafe_SetOwnerIDFromOwner()
{
	APawn* OwnerPawn = WorldActor->GetOwner<APawn>();
	AController* OwnerPC = OwnerPawn != nullptr ? OwnerPawn->GetController() : WorldActor->GetOwner<AController>();

	if (OwnerPawn->GetClass()->ImplementsInterface(UPDRTSBuilderInterface::StaticClass()))
	{
		OwnerID = IPDRTSBuilderInterface::Execute_GetBuilderID(OwnerPawn);
	}
	else if (OwnerPC->GetClass()->ImplementsInterface(UPDRTSBuilderInterface::StaticClass()))
	{
		OwnerID = IPDRTSBuilderInterface::Execute_GetBuilderID(OwnerPC);
	}
}

UPDEntityPinger::UPDEntityPinger(const FPDEntityPingDatum& PingDatum)
{
	PingDataAndHandles.Emplace(PingDatum);
}

TArray<uint8>  UPDEntityPinger::AddPingDatum(const FPDEntityPingDatum& PingDatum)
{
	PingDataAndHandles.Emplace(PingDatum );
	return PingDatum.ToBytes();
}

void UPDEntityPinger::RemovePingDatum(const FPDEntityPingDatum& PingDatum)
{
	PingDataAndHandles.Remove(PingDatum);
}

void UPDEntityPinger::RemovePingDatumWithHash(const FPDEntityPingDatum& PingDatum)
{
	PingDataAndHandles.RemoveByHash(GetTypeHash(PingDatum), PingDatum);
}

FTimerHandle UPDEntityPinger::GetPingHandleCopy(const TArray<uint8>& PingHashBytes)
{
	FPDEntityPingDatum BuildDatum{0};
	BuildDatum.FromBytes(std::move(PingHashBytes));

	return GetPingHandleCopy(BuildDatum.InstanceID);
}

FTimerHandle UPDEntityPinger::GetPingHandleCopy(uint32 PingHash)
{
	const FPDEntityPingDatum CompDatum{PingHash};

	if (PingDataAndHandles.ContainsByHash(PingHash, CompDatum))
	{
		return FTimerHandle{};
	}
	
	return  *PingDataAndHandles.FindByHash(PingHash, CompDatum);
}


TArray<uint8> UPDEntityPinger::EnablePing(const FPDEntityPingDatum& PingDatum)
{
	UWorld* World = PingDatum.WorldActor ? PingDatum.WorldActor->GetWorld() : nullptr;
	if (World == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("FPDEntityPinger::EnablePing -- World it null or not initialized yet"));
		return TArray<uint8>{};
	}
	if (World->bIsWorldInitialized == false)
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("FPDEntityPinger::EnablePing -- World is not initialized yet"));
		return TArray<uint8>{};
	}
	
	FTimerHandle& PingHandleRef = PingDataAndHandles.FindOrAdd(PingDatum);
	const auto InnerPing =
		[&]()
		{
			if (World == nullptr || World->IsValidLowLevelFast() == false)
			{
				return;
			}
				
			Ping(World, PingDatum);
		};

	const FTimerDelegate OnPingDlgt = FTimerDelegate::CreateLambda(InnerPing);

	World->GetTimerManager().SetTimer(PingHandleRef, OnPingDlgt, PingDatum.Interval, true);
	return PingDatum.ToBytes();
}

TArray<uint8> UPDEntityPinger::EnablePingStatic(const FPDEntityPingDatum& PingDatum)
{
	return GEngine->GetEngineSubsystem<UPDEntityPinger>()->EnablePing(PingDatum);
}

void UPDEntityPinger::Ping_Implementation(UWorld* World, const FPDEntityPingDatum& PingDatum)
{
	UPDRTSBaseSubsystem* RTSSubsystem = UPDRTSBaseSubsystem::Get();
	AsyncTask(ENamedThreads::GameThread,
		[ConstPingDatum = PingDatum, InWorld = World, RTSSubsystem]()
		{
			const FGameplayTag EntityTag = TAG_AI_Type_BuilderUnit_Novice ; // @todo, pass into here from somewhere else 
			TArray<FMassEntityHandle> Handles =
				UPDRTSBaseSubsystem::FindIdleEntitiesOfType({EntityTag}, ConstPingDatum.WorldActor, ConstPingDatum.OwnerID);

			ParallelFor(Handles.Num(),
				[InHandles = Handles, InRTSSubsystem = RTSSubsystem, ConstPingDatum, InWorld](const int32 Idx)
				{
					const FMassEntityHandle& EntityHandle = InHandles[Idx];
					if (InRTSSubsystem->EntityManager->IsEntityValid(EntityHandle) == false
						|| InRTSSubsystem->WorldToEntityHandler.Contains(InWorld) == false)
					{
						return;
					}

					const FPDTargetCompound OptTarget = {InvalidHandle, ConstPingDatum.WorldActor->GetActorLocation(), ConstPingDatum.WorldActor};
					InRTSSubsystem->WorldToEntityHandler.FindRef(InWorld)->RequestAction(ConstPingDatum.OwnerID, OptTarget, ConstPingDatum.JobTag, EntityHandle);
				},
				EParallelForFlags::BackgroundPriority);
		});
}

void UPDEntityPinger::DisablePing(const FPDEntityPingDatum& PingDatum)
{
	const UWorld* World = PingDatum.WorldActor ? PingDatum.WorldActor->GetWorld() : nullptr;
	if (World == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("FPDEntityPinger::EnablePing -- World it null or not initialized yet"));
		return;
	}
	if (World->bIsWorldInitialized == false)
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("FPDEntityPinger::EnablePing -- World is not initialized yet"));
		return;
	}
	
	FTimerHandle& PingHandleRef = PingDataAndHandles.FindOrAdd(PingDatum);
	World->GetTimerManager().ClearTimer(PingHandleRef);
}

void UPDEntityPinger::DisablePingStatic(const FPDEntityPingDatum& PingDatum)
{
	GEngine->GetEngineSubsystem<UPDEntityPinger>()->DisablePing(PingDatum);
}

void UPDRTSBaseSubsystem::ProcessBuildContextTable(const TSoftObjectPtr<UDataTable>& TablePath)
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

void UPDRTSBaseSubsystem::SetupOctree()
{
	SetupOctreeWithNewWorld(TemporaryWorldCache);
}

void UPDRTSBaseSubsystem::SetupOctreeWithNewWorld(UWorld* NewWorld)
{
	if (WorldsWithOctrees.Contains(NewWorld))
	{
		return;
	}
		
	static const FString BuildString = "UPDRTSBaseSubsystem::SetupOctreeWithNewWorld";
	UE_LOG(PDLog_RTSBase, Log, TEXT("%s"), *BuildString);
	
	if (NewWorld == nullptr || NewWorld->IsInitialized() == false)
	{
		if (NewWorld != nullptr
			&& NewWorld->IsValidLowLevelFast()
			&& NewWorld->bIsTearingDown == false
			&& NewWorld != TemporaryWorldCache)
		{
			WorldsWithOctrees.FindOrAdd(NewWorld, false);
			TemporaryWorldCache = NewWorld;
		} // Cache as it is being initialized
		
		DispatchOctreeGeneration();
		return;
	}

	// RefreshEntityManager
	EntityManager = &NewWorld->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	
	const float UniformBounds = GetDefault<UPDRTSSubsystemSettings>()->OctreeUniformBounds;
	WorldEntityOctree = PD::Mass::Entity::FPDEntityOctree(FVector::ZeroVector, UniformBounds);
	WorldBuildActorOctree = PD::Mass::Actor::FPDActorOctree(FVector::ZeroVector, UniformBounds);
	WorldsWithOctrees.FindOrAdd(TemporaryWorldCache, true);
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

const FPDBuildContext* UPDRTSBaseSubsystem::GetBuildContextEntry(const FGameplayTag& BuildContextTag)
{
	return BuildContexts_WTag.Contains(BuildContextTag) ? *BuildContexts_WTag.Find(BuildContextTag) : nullptr;
}

const FPDBuildableData* UPDRTSBaseSubsystem::GetBuildableData(const FGameplayTag& BuildableTag)
{
	return BuildableData_WTag.Contains(BuildableTag) ? *BuildableData_WTag.Find(BuildableTag) : nullptr;
}

const FGameplayTag& UPDRTSBaseSubsystem::GetBuildableTagFromData(const FPDBuildableData* BuildableData)
{
	return BuildableData_WTagReverse.Contains(BuildableData) ? *BuildableData_WTagReverse.Find(BuildableData) : FGameplayTag::EmptyTag;
}

void UPDRTSBaseSubsystem::QueueRemoveFromWorldBuildTree(int32 UID)
{
	// FILO
	if (bIsProcessingBuildableRemovalQueue == false)
	{
		RemoveBuildableQueue_FirstBuffer.EmplaceFirst(UID);
		return;
	}

	RemoveBuildableQueue_SecondBuffer.EmplaceFirst(UID);
}

void UPDRTSBaseSubsystem::PassOverDataFromQueue()
{
	RemoveBuildableQueue_FirstBuffer = RemoveBuildableQueue_SecondBuffer;
	RemoveBuildableQueue_SecondBuffer.Empty();
}

TArray<FMassEntityHandle> UPDRTSBaseSubsystem::FindIdleEntitiesOfType(TArray<FGameplayTag> EligibleEntityTypes, const AActor* ActorToBuild, int32 OwnerID)
{
	TArray<FMassEntityHandle> RetArray{};

	if (ensure(ActorToBuild != nullptr) == false)
	{
		return RetArray;
	}

	const UPDRTSBaseSubsystem* RTSBaseSubsystem = Get();
	const bool bIsBuildableGhost = ActorToBuild->GetClass()->ImplementsInterface(UPDRTSBuildableGhostInterface::StaticClass());
	if (bIsBuildableGhost == false || RTSBaseSubsystem->WorldToEntityHandler.Contains(ActorToBuild->GetWorld()) == false)
	{
		return RetArray;
	}

	// 1. Get cell of related octree
	const FPDActorOctreeCell& Cell = RTSBaseSubsystem->WorldBuildActorOctree.GetElementById(*RTSBaseSubsystem->ActorsToCells.FindRef(ActorToBuild->GetUniqueID()).Get());
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

void UPDRTSBaseSubsystem::ProcessGhostStageDataAsset(const AActor* GhostActor, const bool bIsStartOfStage, const FPDRTSGhostStageData& SelectedStageData)
{
	if (GhostActor == nullptr || GhostActor->IsValidLowLevelFast() == false)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("UPDRTSBaseSubsystem::ProcessSingleGhostStage() -- GhostActor is invalid"));
		return;
	}

	// @todo use interface and allow actor to decide which mesh component they should update, they may have several 
	// FVector ActorLocation = GhostActor->GetActorLocation();
	UStaticMeshComponent* ActorMeshComp = GhostActor->GetComponentByClass<UStaticMeshComponent>(); 
	UNiagaraComponent* NiagaraComp = GhostActor->GetComponentByClass<UNiagaraComponent>();

	if (ActorMeshComp == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("UPDRTSBaseSubsystem::ProcessSingleGhostStage(Actor: %s, Data: %s) -- Found no static mesh component on actor. \n ^ @todo set up interface and call that instead to allow the user to select the mesh component"), *GhostActor->GetName(), SelectedStageData.StageDA != nullptr ? *SelectedStageData.StageDA->GetName() : *FString("N/A"));
	}
	if (NiagaraComp == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Error, TEXT("UPDRTSBaseSubsystem::ProcessSingleGhostStage(Actor: %s, Data: %s) -- Found no Niagara component on actor. \n ^ @todo set up interface and call that instead to allow the user to select the mesh component "), *GhostActor->GetName(), SelectedStageData.StageDA != nullptr ? *SelectedStageData.StageDA->GetName() : *FString("N/A"));
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

void UPDRTSBaseSubsystem::ProcessGhostStage(
	AActor*                     GhostActor,
	const FGameplayTag&         BuildableTag,
	FPDRTSGhostBuildState& MutableGhostDatum,
	bool                        bIsStartOfStage)
{
	const UPDRTSBaseSubsystem* NonStaticSelf = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	
	if (NonStaticSelf->BuildableData_WTag.Contains(BuildableTag)) { return; }
	
	const FPDBuildableData* BuildableData = NonStaticSelf->BuildableData_WTag.FindRef(BuildableTag);
	const FPDRTSGhostDatum& GhostData = BuildableData->GhostData;
	switch (GhostData.TransitionStageType)
	{
	case EPDRTSGhostTransition::ESingleStage:
		{
			ProcessGhostStageDataAsset(GhostActor, bIsStartOfStage, GhostData.SingleStageAsset);
			if (bIsStartOfStage == false) { MutableGhostDatum.CurrentStageIdx++; }

			if (MutableGhostDatum.CurrentStageIdx > 0)
			{
				IPDRTSBuildableGhostInterface::Execute_OnSpawnedAsMain(GhostActor, BuildableTag);
			}
		}
		break;
	case EPDRTSGhostTransition::EMultipleStages:
		if (GhostData.MultiStageStageAssets.IsValidIndex(MutableGhostDatum.CurrentStageIdx))
		{
			ProcessGhostStageDataAsset(GhostActor, bIsStartOfStage, GhostData.MultiStageStageAssets[MutableGhostDatum.CurrentStageIdx]);
			if (bIsStartOfStage == false) { MutableGhostDatum.CurrentStageIdx++; }

			if (MutableGhostDatum.CurrentStageIdx > GhostData.MultiStageStageAssets.Num())
			{
				IPDRTSBuildableGhostInterface::Execute_OnSpawnedAsMain(GhostActor, BuildableTag);
			}			
		}
		break;
	default: ;
	}

}

double UPDRTSBaseSubsystem::GetMaxDurationGhostStage(
	const FGameplayTag&         BuildableTag,
	FPDRTSGhostBuildState& MutableGhostDatum)
{
	const UPDRTSBaseSubsystem* NonStaticSelf = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	
	if (NonStaticSelf->BuildableData_WTag.Contains(BuildableTag)) { return -1.0; }
	
	const FPDBuildableData* BuildableData = NonStaticSelf->BuildableData_WTag.FindRef(BuildableTag);
	const FPDRTSGhostDatum& GhostData = BuildableData->GhostData;
	switch (GhostData.TransitionStageType)
	{
	case EPDRTSGhostTransition::ESingleStage:
		if (MutableGhostDatum.CurrentStageIdx == 0)
		{
			return GhostData.SingleStageAsset.MaxDurationForStage;
		}
		break;
	case EPDRTSGhostTransition::EMultipleStages:
		if (GhostData.MultiStageStageAssets.IsValidIndex(MutableGhostDatum.CurrentStageIdx))
		{
			return GhostData.MultiStageStageAssets[MutableGhostDatum.CurrentStageIdx].MaxDurationForStage;
		}
		break;
	default: ;
	}

	return -1.0;
}

bool UPDRTSBaseSubsystem::IsPastFinalIndex(const FGameplayTag& BuildableTag, FPDRTSGhostBuildState& MutableGhostDatum)
{
	const UPDRTSBaseSubsystem* NonStaticSelf = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	if (NonStaticSelf->BuildableData_WTag.Contains(BuildableTag)) { return false; }
	
	const FPDBuildableData* BuildableData = NonStaticSelf->BuildableData_WTag.FindRef(BuildableTag);
	const FPDRTSGhostDatum& GhostData = BuildableData->GhostData;
	switch (GhostData.TransitionStageType)
	{
	case EPDRTSGhostTransition::ESingleStage:
		if (MutableGhostDatum.CurrentStageIdx > 0)
		{
			return true;
		}
		break;
	case EPDRTSGhostTransition::EMultipleStages:
		if (MutableGhostDatum.CurrentStageIdx >= GhostData.MultiStageStageAssets.Num())
		{
			return true;
		}
		break;
	default: ;
	}

	return false;	
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
		WorkTables.Emplace(TablePath.LoadSynchronous());
	}

	BuildContextTables.Empty();
	for(const TSoftObjectPtr<UDataTable>& TablePath : Cast<UPDRTSSubsystemSettings>(SettingsToChange)->BuildContextTables)
	{
		ProcessBuildContextTable(TablePath);
	}		
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
