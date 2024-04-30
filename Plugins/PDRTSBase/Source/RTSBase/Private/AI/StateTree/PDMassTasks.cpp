/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "AI/StateTree/PDMassTasks.h"
#include "AI/Mass/PDMassFragments.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "StateTreeLinker.h"


//
// MOVETOTARGET
bool FPDMTask_MoveToTarget::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveParametersHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	return true;
}

EStateTreeRunStatus FPDMTask_MoveToTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassMovementParameters& MoveParameters = Context.GetExternalData(MoveParametersHandle);
	const FTransformFragment& TransformFragment = Context.GetExternalData(TransformHandle);
	const FMassEntityHandle& ItemHandle = InstanceData.EntityHandle;
	const AActor* PotentialTargetActor = InstanceData.ActorTarget;
	const UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);

	const bool bIsEntityValid = EntitySubsystem.GetEntityManager().IsEntityValid(ItemHandle);
	if ((bIsEntityValid == false) && (PotentialTargetActor == nullptr || PotentialTargetActor->IsValidLowLevelFast() == false))
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector& StartLocation = TransformFragment.GetTransform().GetLocation();
	const FVector& TargetLocation = bIsEntityValid
		? EntitySubsystem.GetEntityManager().GetFragmentDataChecked<FTransformFragment>(ItemHandle).GetTransform().GetLocation()
		: PotentialTargetActor->GetActorLocation();

	UNavigationPath* Navpath = UNavigationSystemV1::FindPathToLocationSynchronously(EntitySubsystem.GetWorld(), StartLocation, TargetLocation);
	if (Navpath != nullptr && Navpath->PathPoints.IsEmpty() == false)
	{
		TArray<FVector>& PathPoints = Navpath->PathPoints;
		PathPoints[0] = StartLocation;
		PathPoints.Last() = TargetLocation;

		InstanceData.NavPath = std::move(PathPoints);
		InstanceData.CurrentNavPathIndex = 0;
		MoveTarget.Center = InstanceData.NavPath[InstanceData.CurrentNavPathIndex];
	}
	else
	{
		MoveTarget.Center = TargetLocation;
	}

	
	MoveTarget.SlackRadius = 100.f;
	MoveTarget.DesiredSpeed.Set(MoveParameters.DefaultDesiredSpeed);
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPDMTask_MoveToTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// When entity reaches target, mark as complete
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);

	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		// Get new center if we are not at last index yet 
		if (InstanceData.CurrentNavPathIndex < (InstanceData.NavPath.Num() - 1) )
		{
			InstanceData.CurrentNavPathIndex++;
			MoveTarget.Center = InstanceData.NavPath[InstanceData.CurrentNavPathIndex];
			return EStateTreeRunStatus::Running;
		}
		
		MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Running;
}
void FPDMTask_MoveToTarget::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	Super::ExitState(Context, Transition);
}

//
// Random wander point
bool FPDMTask_RandomWander::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveTargetHandle);
	return true;
}

EStateTreeRunStatus FPDMTask_RandomWander::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMassMoveTargetFragment& MoveTargetFragment = Context.GetExternalData(MoveTargetHandle);
	const FTransform& Transform = Context.GetExternalData(TransformHandle).GetTransform();
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	const float& Radius = InstanceData.StartRadiusRandomLimit;
	FVector RandomLocation = FVector(FMath::RandRange(-Radius, Radius), FMath::RandRange(-Radius, Radius), 0.f);
	RandomLocation += Transform.GetLocation();

	MoveTargetFragment.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTargetFragment.Center = RandomLocation;

	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	Context.GetExternalData(MassSignalSubsystemHandle).DelaySignalEntity(UE::Mass::Signals::StateTreeActivate, MassContext.GetEntity(), 0.1f);
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPDMTask_RandomWander::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (MoveTarget.DistanceToGoal <= InstanceData.SuccessRadius)
	{
		MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}

	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	Context.GetExternalData(MassSignalSubsystemHandle).DelaySignalEntity(UE::Mass::Signals::StateTreeActivate, MassContext.GetEntity(), DeltaTime == 0.0 ? 0.1 : DeltaTime );	
	
	return EStateTreeRunStatus::Running;
}
void FPDMTask_RandomWander::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	Super::ExitState(Context, Transition);
}

//
// Wait a given amount of time
bool FPDMTask_Wait::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

EStateTreeRunStatus FPDMTask_Wait::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	auto& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::StateTreeActivate, MassContext.GetEntity(), InstanceData.Duration);
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPDMTask_Wait::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.TimePassed += DeltaTime;

	if (InstanceData.TimePassed >= InstanceData.Duration)
	{
		return EStateTreeRunStatus::Succeeded;
	}
	
	const FMassStateTreeExecutionContext& StateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	auto& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::StateTreeActivate, StateTreeContext.GetEntity(), InstanceData.Duration - InstanceData.TimePassed);
	
	return EStateTreeRunStatus::Running;
}

void FPDMTask_Wait::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMassStateTreeTaskBase::ExitState(Context, Transition);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.TimePassed = 0.f;
}


//
// Animation player task
bool FPDMTask_PlayAnimation::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(AnimationHandle);
	
	return true;
}

EStateTreeRunStatus FPDMTask_PlayAnimation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	FPDMFragment_EntityAnimation& AnimationFragment = Context.GetExternalData(AnimationHandle);

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Time = 0; // Reset
	
	AnimationFragment.bOveriddenAnimation = true;
	AnimationFragment.AnimPosition = 0;
	AnimationFragment.AnimationStateIndex = InstanceData.AnimationIndex;
	MoveTarget.CreateNewAction(EMassMovementAction::Animate, *Context.GetWorld());

	const float Duration = InstanceData.Duration;
	if (Duration > 0.0f)
	{
		UMassSignalSubsystem& MassSignalSubsystem = MassContext.GetExternalData(MassSignalSubsystemHandle);
		MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::LookAtFinished, MassContext.GetEntity(), Duration);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPDMTask_PlayAnimation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FPDMFragment_EntityAnimation& AnimationFragment = Context.GetExternalData(AnimationHandle);

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const float Duration = InstanceData.Duration;
	float& Time = InstanceData.Time;
	Time += DeltaTime;

	if (Time >= Duration)
	{
		AnimationFragment.bOveriddenAnimation = false;
		return EStateTreeRunStatus::Succeeded;
	}

	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& MassSignalSubsystem = MassContext.GetExternalData(MassSignalSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::LookAtFinished, MassContext.GetEntity(), Duration);
	
	return Duration <= 0.0f ? EStateTreeRunStatus::Failed : EStateTreeRunStatus::Running;

}
void FPDMTask_PlayAnimation::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	Super::ExitState(Context, Transition);
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