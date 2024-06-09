/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

// Permadev 
#include "AI/Mass/PDMassProcessors.h"
#include "AI/Mass/PDMassFragments.h"
#include "PDRTSBaseSubsystem.h"

// Engine
#include "Engine/World.h"

// Mass (Shared)
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassCommonUtils.h"

// Mass (Fragments)
#include "MassCrowdFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSimulationLOD.h"

// Mass (Subsystems)
#include "MassSignalSubsystem.h"
#include "MassRepresentationSubsystem.h"

// A2T
#include "AnimToTextureDataAsset.h"
#include "AnimToTextureInstancePlaybackHelpers.h"

// Nav1
#include "NavigationSystem.h"

// Chaos debug
#include "Chaos/DebugDrawQueue.h"
#include "ChaosLog.h"
#include "MassCrowdRepresentationSubsystem.h"

TAutoConsoleVariable<bool> UPDOctreeProcessor::CVarDrawCells(
	TEXT("Octree.DebugCells"), false,
	TEXT("Enables debug drawing for the active worlds octree."));


#define CONSTVIEW(Context, FFragment) Context.GetFragmentView<FFragment>()
#define MUTVIEW(Context, FFragment) Context.GetMutableFragmentView<FFragment>()
#define SHAREDVIEW(Context, FFragment) Context.GetConstSharedFragment<FFragment>()
#define MUTSHAREDVIEW(Context, FFragment) Context.GetMutableSharedFragment<FFragment>()

UPDMProcessor_InitializeEntities::UPDMProcessor_InitializeEntities()
{
	ObservedType = FPDMFragment_RTSEntityBase::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UPDMProcessor_InitializeEntities::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void UPDMProcessor_InitializeEntities::ConfigureQueries()
{
	EntityQuery.AddRequirement<FPDMFragment_RTSEntityBase>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FPDMFragment_SharedAnimData>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FPDMTag_RTSEntity>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FPDMFragment_EntityAnimation>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UPDMProcessor_InitializeEntities::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([this](FMassExecutionContext& Context)
	{
		
		TSharedFragment<FPDMFragment_SharedAnimData> RTSEntityParameters = SHAREDVIEW(Context, FPDMFragment_SharedAnimData);
		TMutFragment<FPDMFragment_EntityAnimation> AnimationFragments = MUTVIEW(Context, FPDMFragment_EntityAnimation);

		// Iterate the entities animation fragments and refresh their vertex anim data-asset
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FPDMFragment_EntityAnimation& AnimationFragment = AnimationFragments[EntityIndex];
			UAnimToTextureDataAsset* Anim = RTSEntityParameters.AnimData.Get();
			if (Anim != nullptr) { Anim = RTSEntityParameters.AnimData.LoadSynchronous(); }
			
			AnimationFragment.A2TData = Anim;
		}
	}));
}

UPDMProcessor_EntityCosmetics::UPDMProcessor_EntityCosmetics()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
}

void UPDMProcessor_EntityCosmetics::Initialize(UObject& Owner)
{
	// @todo replace with crowd subsystem
	Super::Initialize(Owner);
	RepresentationSubsystem = UWorld::GetSubsystem<UMassCrowdRepresentationSubsystem>(Owner.GetWorld());
}

void UPDMProcessor_EntityCosmetics::ConfigureQueries()
{
	// PDM requirements
	EntityQuery.AddTagRequirement<FPDMTag_RTSEntity>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FPDMFragment_RTSEntityBase>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FPDMFragment_EntityAnimation>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FPDMFragment_SharedAnimData>(EMassFragmentPresence::All);
	
	// Base MASS requirements
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	EntityQuery.RegisterWithProcessor(*this);
}

bool UPDMProcessor_EntityCosmetics::ProcessVertexAnimation(
	int32 EntityIdx,
	TConstFragment<FMassRepresentationLODFragment>& RepresentationLODFragments,
	const FMassRepresentationFragment& Rep,
	FPDMFragment_RTSEntityBase* RTSEntityFragment,
	FPDMFragment_EntityAnimation* AnimationData,
	const FMassVelocityFragment& Velocity,
	const FMassInstancedStaticMeshInfoArrayView& MeshInfo,
	const TArrayView<FMassInstancedStaticMeshInfo>& MeshInfoInnerArray,
	const UPDMProcessor_EntityCosmetics* Self)
{
	if (RTSEntityFragment == nullptr || AnimationData == nullptr) { return false; }
	
	if (RTSEntityFragment->MeshSkinIdx == -1) { RTSEntityFragment->MeshSkinIdx = FMath::RandRange(0.f,3.f); }

	// todo, find a way to iterate and update animations based on current action state. can also be used to update other ISM things
	if (Rep.CurrentRepresentation != EMassRepresentationType::StaticMeshInstance) { return false; }
	
	// 0-4 is anim data
	const float PrevPlayRate = AnimationData->PlayRate;
	const float GlobalTime = Self->GetWorld()->GetTimeSeconds();
		
	const float SpeedSq = Velocity.Value.SizeSquared();
	constexpr float IdleLimit     = 25. * 25.;
	constexpr float WalkStart     = 250. * 250.;
	constexpr float JogStart      = 500. * 500.;
	constexpr float SprintStart   = 700. * 700.; 
	if (LIKELY(AnimationData->bOverriddenAnimation == false))
	{
		EPDVertexAnimSelector AnimSelectionIndex = EPDVertexAnimSelector::VertexIdle;
		if (RTSEntityFragment->bAction)
		{
			AnimSelectionIndex = EPDVertexAnimSelector::VertexAction;
		}				
		else if (SpeedSq <= IdleLimit)
		{
			AnimSelectionIndex = EPDVertexAnimSelector::VertexIdle;
			
			AnimationData->PlayRate = FMath::Clamp(FMath::Sqrt(SpeedSq), 0.6f, 2.0f);
		}
		else if (SpeedSq <= WalkStart)
		{
			AnimSelectionIndex = EPDVertexAnimSelector::VertexSlowWalk;
			
			AnimationData->PlayRate = FMath::Clamp(FMath::Sqrt(SpeedSq), 0.6f, 2.0f);
		}
		else if (SpeedSq <= JogStart)
		{
			AnimSelectionIndex = EPDVertexAnimSelector::VertexWalk;
			
			AnimationData->PlayRate = FMath::Clamp(FMath::Sqrt(SpeedSq), 0.6f, 2.0f);
		}
		else if (SpeedSq <= SprintStart)
		{
			AnimSelectionIndex = EPDVertexAnimSelector::VertexJog;
			
			AnimationData->PlayRate = FMath::Clamp(FMath::Sqrt(SprintStart / SpeedSq), 0.6f, 2.0f);
		} else if (SpeedSq > SprintStart)
		{
			AnimSelectionIndex = EPDVertexAnimSelector::VertexSprint;
			
			AnimationData->PlayRate = FMath::Clamp(FMath::Sqrt(SpeedSq / SprintStart), 0.6f, 2.0f);
		}
			
		AnimationData->AnimationStateIndex = AnimSelectionIndex;
		AnimationData->InWorldStartTime = GlobalTime - PrevPlayRate * (GlobalTime - AnimationData->InWorldStartTime) / AnimationData->PlayRate;
	}
	else
	{
		// Animation processing taken over briefly by task
		AnimationData->PlayRate = 1.f;
		AnimationData->InWorldStartTime = GlobalTime - PrevPlayRate * (GlobalTime - AnimationData->InWorldStartTime) / AnimationData->PlayRate;
	}

	// ensuring that MeshInfo has valid data, problematically the actual array being accessed by the [] operator is in private access
	// and thus we can't actually bounds check it beforehand directly, at-least that would have been the case if we didn't have this in the standard https://eel.is/c++draft/temp.friend 
	if (MeshInfoInnerArray.IsValidIndex(Rep.StaticMeshDescIndex) == false) { return false; }
	const FMassRepresentationLODFragment& RepLOD = RepresentationLODFragments.IsValidIndex(EntityIdx) ? RepresentationLODFragments[EntityIdx] : FMassRepresentationLODFragment();
			
	FMassInstancedStaticMeshInfo& ISMInfo = MeshInfo[Rep.StaticMeshDescIndex];
	Self->UpdateISMVertexAnimation(ISMInfo, *AnimationData, RepLOD.LODSignificance, Rep.PrevLODSignificance, 0);
	ISMInfo.AddBatchedCustomData<float>(RTSEntityFragment->MeshSkinIdx, RepLOD.LODSignificance, Rep.PrevLODSignificance, 4);

	return true;
}

bool UPDMProcessor_EntityCosmetics::ProcessMaterialInstanceData(
	const FMassEntityHandle& EntityHandle,
	const FMassRepresentationLODFragment& RepLOD,
	const FMassRepresentationFragment& Rep,
	FMassInstancedStaticMeshInfo& ISMInfo,
	FPDMFragment_RTSEntityBase* RTSEntityFragment)
{
	const FMassLODSignificanceRange* Range = ISMInfo.GetLODSignificanceRange(Rep.PrevLODSignificance);
	if (Range == nullptr)
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("UPDMProcessor_EntityCosmetics::ProcessMaterialInstanceData - Range INVALID"))
		return false;
	}

	if (Range->ISMCSharedDataPtr == nullptr) 
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("UPDMProcessor_EntityCosmetics::ProcessMaterialInstanceData - RangeData INVALID"))
		return false;
	}

	const int32 EntityHashID = GetTypeHash(EntityHandle);
	FMassISMCSharedData* FirstFoundInstance = nullptr;
	for (const FMassStaticMeshInstanceVisualizationMeshDesc& Mesh : ISMInfo.GetDesc().Meshes)
	{
		const uint32 MeshDescHash = GetTypeHash(Mesh);
		FMassISMCSharedData* Instance = Range->ISMCSharedDataPtr->Find(MeshDescHash);
		if (Instance == nullptr || Instance->GetISMComponent() == nullptr || Instance->GetISMComponent()->IsValidInstance(EntityHashID) == false)
		{
			continue;
		}
		
		FirstFoundInstance = Instance;
		break;
	}

	if (FirstFoundInstance == nullptr)
	{
		return false;
	}
	
	double OpacityModifier;
	double DilationModifier;
	bool bLog = false;
	switch (RTSEntityFragment->SelectionState)
	{
	case EPDEntitySelectionState::ENTITY_SELECTED:
		if (RTSEntityFragment->bHasClearedSelection)
		{
			bLog = true;

			RTSEntityFragment->bHasClearedSelection = false;
			OpacityModifier  = 0.0;
			DilationModifier = 1.0;
			FirstFoundInstance->GetISMComponent()->SetCustomDataValue(EntityHashID,2, OpacityModifier);
			FirstFoundInstance->GetISMComponent()->SetCustomDataValue(EntityHashID,3, DilationModifier);
		}
		break;
	case EPDEntitySelectionState::ENTITY_NOTSELECTED:
		// UE_LOG(PDLog_RTSBase, Warning, TEXT("UPDMProcessor_EntityCosmetics::ProcessMaterialInstanceData - EPDEntitySelectionState::ENTITY_NOTSELECTED"))
		if (RTSEntityFragment->bHasClearedSelection == false)
		{
			bLog = true;

			RTSEntityFragment->bHasClearedSelection = true;
			OpacityModifier  = 1.0;
			DilationModifier = 0.0;
			FirstFoundInstance->GetISMComponent()->SetCustomDataValue(EntityHashID,2, OpacityModifier);
			FirstFoundInstance->GetISMComponent()->SetCustomDataValue(EntityHashID,3, DilationModifier);					
		}
		break;
	case EPDEntitySelectionState::ENTITY_UNSET:
		// UE_LOG(PDLog_RTSBase, Warning, TEXT("UPDMProcessor_EntityCosmetics::ProcessMaterialInstanceData - EPDEntitySelectionState::ENTITY_UNSET"))

		break;
	}
	
	if (bLog && FirstFoundInstance->GetISMComponent()->IsValidInstance(EntityHashID))
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("UPDMProcessor_EntityCosmetics::ProcessMaterialInstanceData - VALID INSTANCE(%i)"), EntityHashID)
	}
	else if (bLog)
	{
		UE_LOG(PDLog_RTSBase, Warning, TEXT("UPDMProcessor_EntityCosmetics::ProcessMaterialInstanceData - INVALID INSTANCE(%i)"), EntityHashID)
	}
	
	return bLog;
}

// Tag private member for safe access, usually avoid this
// BUT: The amount of work fetching the engine source just to make a tiny change in FMassInstancedStaticMeshInfoArrayView would not be worth it,
// this is portable and allowed by ISO so deal with it
using MassISMArrayTagType = TAccessorTypeHandler<FMassInstancedStaticMeshInfoArrayView, TArrayView<FMassInstancedStaticMeshInfo>>; 
template struct TTagPrivateMember<MassISMArrayTagType, &FMassInstancedStaticMeshInfoArrayView::InstancedStaticMeshInfos>;
void UPDMProcessor_EntityCosmetics::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& InContext)
	{
		TConstFragment<FMassVelocityFragment> VelocityFragments = CONSTVIEW(InContext, FMassVelocityFragment);
		TConstFragment<FMassRepresentationLODFragment> RepresentationLODFragments = CONSTVIEW(InContext, FMassRepresentationLODFragment);
		TMutFragment<FMassRepresentationFragment> RepresentationFragments = MUTVIEW(InContext, FMassRepresentationFragment);
		TMutFragment<FPDMFragment_RTSEntityBase> RTSEntityFragments = MUTVIEW(InContext, FPDMFragment_RTSEntityBase);
		TMutFragment<FPDMFragment_EntityAnimation> EntityAnimationData = MUTVIEW(InContext, FPDMFragment_EntityAnimation);
		
		const FMassInstancedStaticMeshInfoArrayView MeshInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();

		// Access private member safely and legally according to ISO: https://eel.is/c++draft/temp.friend
		const TArrayView<FMassInstancedStaticMeshInfo>& MeshInfoInnerArray = MeshInfo.*TPrivateAccessor<MassISMArrayTagType>::TypeValue;
		
		for (int32 EntityIdx = 0; EntityIdx < InContext.GetNumEntities(); ++EntityIdx)
		{
			const FMassVelocityFragment& Velocity          = VelocityFragments.IsValidIndex(EntityIdx)       ? VelocityFragments[EntityIdx]       : FMassVelocityFragment(); ;
			const FMassRepresentationFragment& Rep         = RepresentationFragments.IsValidIndex(EntityIdx) ? RepresentationFragments[EntityIdx] : FMassRepresentationFragment();
			FPDMFragment_RTSEntityBase* RTSEntityFragment  = RTSEntityFragments.IsValidIndex(EntityIdx)      ? &RTSEntityFragments[EntityIdx]     : nullptr;
			FPDMFragment_EntityAnimation* AnimationData    = EntityAnimationData.IsValidIndex(EntityIdx)     ? &EntityAnimationData[EntityIdx]     : nullptr;
			
			const FMassRepresentationLODFragment& RepLOD = RepresentationLODFragments.IsValidIndex(EntityIdx) ? RepresentationLODFragments[EntityIdx] : FMassRepresentationLODFragment();
			ProcessMaterialInstanceData(InContext.GetEntity(EntityIdx), RepLOD, Rep,MeshInfo[Rep.StaticMeshDescIndex], RTSEntityFragment);
			ProcessVertexAnimation(EntityIdx, RepresentationLODFragments, Rep, RTSEntityFragment, AnimationData, Velocity, MeshInfo, MeshInfoInnerArray, this);
		}
	});
}

void UPDMProcessor_EntityCosmetics::UpdateISMVertexAnimation(FMassInstancedStaticMeshInfo& ISMInfo, FPDMFragment_EntityAnimation& AnimationData, const float LODSignificance, const float PrevLODSignificance, const int32 NumFloatsToPad /*= 0*/) const
{
	const float DeltaTimeSinceStart = GetWorld()->GetTimeSeconds() - AnimationData.InWorldStartTime;
	
	FAnimToTextureFrameData InstanceData;
	UAnimToTextureInstancePlaybackLibrary::GetFrameDataFromDataAsset(AnimationData.A2TData.Get(), static_cast<uint8>(AnimationData.AnimationStateIndex),DeltaTimeSinceStart, InstanceData);
	ISMInfo.AddBatchedCustomData<FAnimToTextureFrameData>(InstanceData, LODSignificance, PrevLODSignificance, NumFloatsToPad);
}

//
// move (to) target
UPDProcessor_MoveTarget::UPDProcessor_MoveTarget()
{
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UPDProcessor_MoveTarget::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddSharedRequirement<FPDMFragment_SharedEntity>(EMassFragmentAccess::ReadOnly);
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
	EntityQuery.RegisterWithProcessor(*this);
}


//
// Navpath calculations when? somewhere in here in this execute function
void UPDProcessor_MoveTarget::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesToSignal;
	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([this, &EntitiesToSignal](FMassExecutionContext& Context)
	{
		TMutFragment<FTransformFragment>& TransformsList = MUTVIEW(Context, FTransformFragment);
		TMutFragment<FMassMoveTargetFragment>& MoveTargets = MUTVIEW(Context, FMassMoveTargetFragment);
		FPDMFragment_SharedEntity& SharedFragment = MUTSHAREDVIEW(Context, FPDMFragment_SharedEntity);
		
		//
		// Update Shared navpath fragments 
		UPDRTSBaseSubsystem* RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
		if (RTSSubsystem != nullptr && RTSSubsystem->DirtySharedData.IsEmpty() == false)
		{
			TMap<int32, FPDWrappedSelectionGroupNavData>& NavSelections = RTSSubsystem->SelectionGroupNavData;
			// only copy over the dirty keys, the clear the dirty list
			for (const TTuple<int32 /*OwnerID*/,int32 /*SelectionIdx*/> DirtyKey : RTSSubsystem->DirtySharedData)
			{
				if (NavSelections.Contains(DirtyKey.Key) == false
					|| NavSelections.Find(DirtyKey.Key)->SelectionGroupNavData.Contains(DirtyKey.Value))
				{
					continue;
				}
				
				SharedFragment.SharedNavData.FindOrAdd(DirtyKey.Key).SelectionGroupNavData.FindOrAdd(DirtyKey.Value) =
					NavSelections.FindRef(DirtyKey.Key).SelectionGroupNavData.FindRef(DirtyKey.Value);
			}
			RTSSubsystem->DirtySharedData.Empty();
		}
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargets[EntityIndex];
			if (MoveTarget.GetCurrentAction() != EMassMovementAction::Move) { continue; }
			
			const FTransform& Transform = TransformsList[EntityIndex].GetTransform();
			MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();
			if (MoveTarget.DistanceToGoal > MoveTarget.SlackRadius) { continue; }
			
			EntitiesToSignal.Add(Context.GetEntity(EntityIndex));
			MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
		}
	}));

	if (EntitiesToSignal.IsEmpty()) { return; }
	SignalSubsystem->SignalEntities(UE::Mass::Signals::FollowPointPathDone, EntitiesToSignal);
}

void UPDProcessor_MoveTarget::Initialize(UObject& Owner)
{
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(GetWorld());
}

namespace PD::Mass::Crowd
{
	PDRTSBASE_API int32 GCrowdTurnOffVisualization = 0;
	FAutoConsoleVariableRef CVarCrowdTurnOffVisualization(TEXT("Mass.CrowdTurnOffVisualization"), GCrowdTurnOffVisualization, TEXT("Turn off crowd visualization"));

	int32 bDebugCrowdVisualizationLOD = 0;
	int32 bDebugShowISMUnderSpecifiedRange = 0;

	FAutoConsoleVariableRef ConsoleVariables[] =
	{
		FAutoConsoleVariableRef(TEXT("ai.debug.CrowdVisualizationLOD"), bDebugCrowdVisualizationLOD, TEXT("Debug crowd visualization LOD"), ECVF_Cheat),
		FAutoConsoleVariableRef(TEXT("ai.debug.ShowISMUnderSpecifiedRange"), bDebugShowISMUnderSpecifiedRange, TEXT("Show ISM under a specified range (meters)"), ECVF_Cheat)
	};

} // UE::MassCrowd

UPDMProcessor_LODVisualization::UPDMProcessor_LODVisualization()
{
	bAutoRegisterWithProcessingPhases = true;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);

	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::LODCollector);
}

void UPDMProcessor_LODVisualization::ConfigureQueries()
{
	Super::ConfigureQueries();

	CloseEntityQuery.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);
	CloseEntityAdjustDistanceQuery.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);
	FarEntityQuery.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);
	DebugEntityQuery.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);

	FilterTag = FMassCrowdTag::StaticStruct();
}

void UPDMProcessor_LODVisualization::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ForceOffLOD(static_cast<bool>(PD::Mass::Crowd::GCrowdTurnOffVisualization));

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("CrowdVisualizationLOD"))
	Super::Execute(EntityManager, Context);
}

void UPDMProcessor_LODVisualization::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void UPDMProcessor_LODCollector::ConfigureQueries()
{
	Super::ConfigureQueries();

	EntityQuery_VisibleRangeAndOnLOD.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);
	EntityQuery_VisibleRangeOnly.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);
	EntityQuery_OnLODOnly.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);
	EntityQuery_NotVisibleRangeAndOffLOD.AddTagRequirement<FMassCrowdTag>(EMassFragmentPresence::All);
}

UPDOctreeProcessor::UPDOctreeProcessor()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UPDOctreeProcessor::DebugDrawCells()
{
	const PD::Mass::Entity::FPDSafeOctree& Octree = RTSSubsystem->WorldOctree;
#if CHAOS_DEBUG_DRAW
	if (UPDOctreeProcessor::CVarDrawCells.GetValueOnAnyThread())
	{
		if (bSentChaosCommand == false && GEngine != nullptr)
		{
			bSentChaosCommand = true;
			GEngine->HandleDeferCommand(TEXT("p.Chaos.DebugDraw.Enabled 1"), *GLog); 
		}
		
		QUICK_SCOPE_CYCLE_COUNTER(STAT_OctreeChaosDebug)
		Octree.FindAllElements([&](const FPDEntityOctreeCell& Cell)
		{
			Chaos::FDebugDrawQueue::GetInstance()
				.DrawDebugBox(
					Cell.Bounds.Center,
					Cell.Bounds.Extent,
					FQuat::Identity,
					FColor::White,
					false,
					0,
					0,
					1.0f);
			
			Chaos::FDebugDrawQueue::GetInstance()
				.DrawDebugString(
					Cell.Bounds.Center + FVector(0, 0, (Cell.Bounds.Extent.Z * 2)),
					FString::FromInt(Cell.EntityHandle.Index),
					nullptr,
					FColor::MakeRandomSeededColor(Cell.EntityHandle.Index),
					0,
					true,
					2);
		});
	}
	else if (bSentChaosCommand && GEngine != nullptr)
	{
		bSentChaosCommand = false;
		GEngine->HandleDeferCommand(TEXT("p.Chaos.DebugDraw.Enabled 0"), *GLog); 
	}
#endif
}

void UPDOctreeProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
}

void UPDOctreeProcessor::ConfigureQueries()
{
	UpdateOctreeElementsQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	UpdateOctreeElementsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UpdateOctreeElementsQuery.AddRequirement<FPDOctreeFragment>(EMassFragmentAccess::ReadWrite);
	UpdateOctreeElementsQuery.AddRequirement<FPDMFragment_RTSEntityBase>(EMassFragmentAccess::ReadOnly);
	UpdateOctreeElementsQuery.AddTagRequirement<FPDInOctreeGridTag>(EMassFragmentPresence::All);
	UpdateOctreeElementsQuery.RegisterWithProcessor(*this);
}

void UPDOctreeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Clear previous buffer
	UPDRTSBaseSubsystem* RTSSubSystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	RTSSubSystem->OctreeUserQuery.ClearQueryBuffer(UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_MINIMAP);
	RTSSubSystem->OctreeUserQuery.ClearQueryBuffer(UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_HOVERSELECTION); // Hover Selection Group
	
	UpdateOctreeElementsQuery.ForEachEntityChunk(EntityManager, Context,
		[this, RTSSubSystem](FMassExecutionContext& LambdaContext)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_OctreeCellExecution)
		PD::Mass::Entity::FPDSafeOctree& Octree = RTSSubsystem->WorldOctree;
		const int32 NumEntities = LambdaContext.GetNumEntities();

		TConstFragment<FMassVelocityFragment>& Velocities         = CONSTVIEW(LambdaContext, FMassVelocityFragment);
		TConstFragment<FTransformFragment>& LocationList          = CONSTVIEW(LambdaContext, FTransformFragment);
		TConstFragment<FPDMFragment_RTSEntityBase>& RTSEntityList = CONSTVIEW(LambdaContext, FPDMFragment_RTSEntityBase);
		TMutFragment<FPDOctreeFragment>& OctreeFragments          = MUTVIEW(LambdaContext, FPDOctreeFragment);

		if (Octree.IsLocked()) { return; }
		
		Octree.Lock();
		for (int32 EntityListIdx = 0; EntityListIdx < NumEntities; ++EntityListIdx)
		{
			const FVector& DistanceTraveled = (Velocities[EntityListIdx].Value * LambdaContext.GetDeltaTimeSeconds());
			const FVector& CurrentLocation = LocationList[EntityListIdx].GetTransform().GetLocation();
			const FVector& PrevLocation = CurrentLocation - DistanceTraveled;

			FPDOctreeFragment& OctreeFragment = OctreeFragments[EntityListIdx];
			
			const FMassEntityHandle Entity = LambdaContext.GetEntity(EntityListIdx);			
			RTSSubSystem->OctreeUserQuery.UpdateQueryOverlapBuffer(
				UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_MINIMAP,
				CurrentLocation, Entity, RTSEntityList[EntityListIdx].OwnerID);
			RTSSubSystem->OctreeUserQuery.UpdateQueryOverlapBuffer(
				UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_HOVERSELECTION,
				CurrentLocation, Entity, RTSEntityList[EntityListIdx].OwnerID);

			
			// I expect standing still more than moving around, hence a 'likely' hint here
			if (LIKELY(OctreeFragment.CellID == nullptr
				|| Octree.IsValidElementId(*OctreeFragment.CellID) == false
				|| PrevLocation.Equals(CurrentLocation)))
			{
				continue;
			}

			const FOctreeElementId2* CellID = OctreeFragment.CellID.Get(); 
			FPDEntityOctreeCell CopyCurrentOctreeElement = Octree.GetElementById(*CellID);

			Octree.RemoveElement(*CellID);
			
			CopyCurrentOctreeElement.Bounds.Center = FVector4(CurrentLocation, 0);
			Octree.AddElement(CopyCurrentOctreeElement);
		}
		Octree.Unlock();
	});

	DebugDrawCells();
}

UPDOctreeEntityObserver::UPDOctreeEntityObserver()
{
	ObservedType = FPDOctreeFragment::StaticStruct();
	bRequiresGameThreadExecution = true;
	Operation = EMassObservedOperation::Add;
}

void UPDOctreeEntityObserver::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	ElementCellSize = GetDefault<UPDRTSSubsystemSettings>()->DefaultOctreeCellSize;
}

void UPDOctreeEntityObserver::ConfigureQueries()
{
	EntityQuery.AddRequirement<FPDOctreeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UPDOctreeEntityObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context,
	[this](FMassExecutionContext& LambdaContext)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_OctreeCellTracker)
		TConstFragment<FTransformFragment>& TransformList   = CONSTVIEW(LambdaContext, FTransformFragment); 
		TMutFragment<FPDOctreeFragment>& OctreeList         = MUTVIEW(LambdaContext, FPDOctreeFragment);
		TMutFragment<FAgentRadiusFragment>& RadiusFragments = MUTVIEW(LambdaContext, FAgentRadiusFragment);

		const bool bRadiiValid = RadiusFragments.Num() > 0;
		const int32 EntityCount = LambdaContext.GetNumEntities();
		PD::Mass::Entity::FPDSafeOctree& Octree = RTSSubsystem->WorldOctree;

		if (Octree.IsLocked()) { return; }
		
		Octree.Lock();
		for (int32 Step = 0; Step < EntityCount; ++Step)
		{
			const FTransform& Transform = TransformList[Step].GetTransform();
			const FMassEntityHandle Entity = LambdaContext.GetEntity(Step);
			FPDEntityOctreeCell NewOctreeElement;

			NewOctreeElement.Bounds =
				bRadiiValid ? FBoxCenterAndExtent(Transform.GetLocation(), FVector(RadiusFragments[Step].Radius * 2.0f))
				:             FBoxCenterAndExtent(Transform.GetLocation(), FVector(ElementCellSize));
			
			NewOctreeElement.SharedCellID = MakeShared<FOctreeElementId2, ESPMode::ThreadSafe>();
			NewOctreeElement.EntityHandle = Entity;

			Octree.AddElement(NewOctreeElement);

			OctreeList[Step].CellID = NewOctreeElement.SharedCellID;
			LambdaContext.Defer().AddTag<FPDInOctreeGridTag>(LambdaContext.GetEntity(Step));
		}
		Octree.Unlock();
	});
}

UPDGridCellDeinitObserver::UPDGridCellDeinitObserver()
{
	ObservedType = FPDOctreeFragment::StaticStruct();
	bRequiresGameThreadExecution = true;
	Operation = EMassObservedOperation::Remove;
}

void UPDGridCellDeinitObserver::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
}

void UPDGridCellDeinitObserver::ConfigureQueries()
{
	EntityQuery.AddRequirement<FPDOctreeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UPDGridCellDeinitObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	PD::Mass::Entity::FPDSafeOctree& Octree = RTSSubsystem->WorldOctree;
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& LambdaContext)
	{
		const TArrayView<FPDOctreeFragment> CellFragments = LambdaContext.GetMutableFragmentView<FPDOctreeFragment>();

		const int32 NumEntities = LambdaContext.GetNumEntities();

		if (Octree.IsLocked()) { return; }
		
		Octree.Lock();
		for (int32 EntityListIdx = 0; EntityListIdx < NumEntities; ++EntityListIdx)
		{
			TSharedPtr<FOctreeElementId2> CellID = CellFragments[EntityListIdx].CellID;
			if (CellID.IsValid() == false) { continue; }

			Octree.RemoveElement(*CellID);
		}
		Octree.Unlock();
	});
}

UPDCollisionSignalProcessor::UPDCollisionSignalProcessor()
{
	ExecutionOrder.ExecuteAfter.Add(UPDOctreeProcessor::StaticClass()->GetFName());
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}


void UPDCollisionSignalProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();

	UMassSignalSubsystem* SignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	SubscribeToSignal(*SignalSubsystem, MassSample::Signals::OnEntityHitOther);			
}

void UPDCollisionSignalProcessor::ConfigureQueries()
{
	WorldOctreeEntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	WorldOctreeEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	WorldOctreeEntityQuery.AddTagRequirement<FPDOctreeQueryTag>(EMassFragmentPresence::All);
	WorldOctreeEntityQuery.RegisterWithProcessor(*this);
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UPDCollisionSignalProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// PD::Mass::Entity::FPDSafeOctree& WorldOctree = RTSSubsystem->WorldOctree;

	// Other entities we hit
	TQueue<FMassEntityHandle, EQueueMode::Mpsc> EntityCollisionQueue;
	int32 HitEntityCount = 0;
	
	WorldOctreeEntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& LambdaContext)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_OctreeEntityCollisions);

		TConstFragment<FMassVelocityFragment>& Velocities = LambdaContext.GetFragmentView<FMassVelocityFragment>();
		TConstFragment<FTransformFragment>& Transforms = LambdaContext.GetFragmentView<FTransformFragment>();

		const int32 NumEntities = LambdaContext.GetNumEntities();
		for (int32 EntityListIdx = 0; EntityListIdx < NumEntities; ++EntityListIdx)
		{
			const FVector& Velocity = Velocities[EntityListIdx].Value;
			const FVector& CurrentLocation = Transforms[EntityListIdx].GetTransform().GetLocation();
			const FVector& DistanceTraveled = (Velocity * LambdaContext.GetDeltaTimeSeconds());
			
			const FVector& PrevLocation = CurrentLocation - DistanceTraveled;

			// Halving extents
			FBoxCenterAndExtent QueryBounds = FBoxCenterAndExtent(CurrentLocation - (DistanceTraveled / 2), (DistanceTraveled.GetAbs() / 2));

			// @todo in progress:  replace with WorldOctree.FindNodesWithPredicate() or do a manual search from root, instead of running it over all cells
			// @todo backlog rewrite functions to use parameter sig.: ParentNodeIndex, CurrentNodeIndex, NodeContext.Bounds
			// WorldOctree.FindNodesWithPredicate(
			// [&](const FPDEntityOctreeCell& GridElement)->bool
			// {
			// 	const FVector TraceDir = CurrentLocation - PrevLocation;
			// 	if (
			// 		FMath::LineBoxIntersection(GridElement.Bounds.GetBox(), PrevLocation, CurrentLocation, TraceDir)
			// 		&& GridElement.EntityHandle.Index != LambdaContext.GetEntity(EntityListIdx).Index)
			// 	{
			// 		return false; // Stop traversal, found a viable node
			// 	}
			//
			// 	return true; // Continue traversal, found no suitable node yet
			// },
			//
			// [&](const FPDEntityOctreeCell& GridElement)
			// {
			// 		HitEntityCount++;
			// 		EntityCollisionQueue.Enqueue(GridElement.EntityHandle);
			// });
		}
	});

	if (HitEntityCount > 0)
	{
		TArray<FMassEntityHandle> Entities = UE::Mass::Utils::EntityQueueToArray(EntityCollisionQueue, HitEntityCount);
		Context.GetMutableSubsystem<UMassSignalSubsystem>()->SignalEntities(MassSample::Signals::OnEntityHitOther, Entities);
	}
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