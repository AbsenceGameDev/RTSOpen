/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassStateTreeTypes.h"
#include "PDRTSCommon.h"
#include "AI/Mass/PDMassFragments.h"
#include "PDMassTasks.generated.h"

class UNavigationPath;
struct FPDMFragment_EntityAnimation;
struct FTransformFragment;
struct FMassMoveTargetFragment;
struct FMassMovementParameters;

struct FStateTreeExecutionContext;
class UMassEntitySubsystem;
class UMassSignalSubsystem;

/* Macro to remove some of the boilerplate in declaring a new task */
#define DECLARE_TASK_BODY(TaskName)\
	using FInstanceDataType = FPDMTaskData_##TaskName ; \
	virtual bool Link(FStateTreeLinker& Linker) override; \
	virtual const UStruct* GetInstanceDataType() const override { return FPDMTaskData_##TaskName ::StaticStruct(); } \
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override; \
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override; \
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;


/** @brief Instance data for entity running FPDMTask_MoveToTarget */
USTRUCT(Blueprintable)
struct FPDMStuckMovementConditions
{
	GENERATED_BODY()
	
	EStateTreeRunStatus ShouldContinueMovement(float DeltaTime)
	{
		TimeDeltaAccumulator += DeltaTime;
		if (TimeDeltaAccumulator >= ConsiderationInterval)
		{
			if (MinimumAverageSpeedPerConsiderationInterval > (DistanceDeltaAccumulator / TimeDeltaAccumulator))
			{
				return EStateTreeRunStatus::Failed;
			}
		
			TimeDeltaAccumulator = 0;
		}		
		return EStateTreeRunStatus::Running;
	}
	
	
	/** @brief Minimum Average speed, polled every 10 seconds to make sure the entity is not stuck */
	UPROPERTY(EditAnywhere, Category = "Data")
	float MinimumAverageSpeedPerConsiderationInterval = 20.0f;

	/** @brief How large interval between each consideration to check if we are stuck or not */
	UPROPERTY(EditAnywhere, Category = "Data")
	float ConsiderationInterval = 10.f;

	/* Used to ensure the entity is not stuck, @todo might need to move to the movement processor */
	UPROPERTY(VisibleAnywhere, Category = "Data")
	float LastDistance = 0.0;		
	/* Used to ensure the entity is not stuck, @todo might need to move to the movement processor */
	UPROPERTY(VisibleAnywhere, Category = "Data")
	float DistanceDeltaAccumulator = 0.0;
	/* Used to ensure the entity is not stuck, @todo might need to move to the movement processor */
	UPROPERTY(VisibleAnywhere, Category = "Data")
	float TimeDeltaAccumulator = 0.0;	
};


//
/// MOVETOHANDLE


/** @brief Instance data for entity running FPDMTask_MoveToTarget */
USTRUCT()
struct FPDMTaskData_MoveToHandle
{
	GENERATED_BODY()

	/** @brief Result of the candidates search request (Input) */
	UPROPERTY(EditAnywhere, Category = Input)
	FPDTargetCompound OptTargets;	

	/** @brief Result of the candidates search request (Input) */
	UPROPERTY(VisibleAnywhere, Category = "Parameter")
	TArray<FVector> NavPath;
	
	/** @brief Result of the candidates search request (Input) */
	UPROPERTY(VisibleAnywhere, Category = "Data")
	int16 CurrentNavPathIndex;
	
	/** @brief Settings to control our parameters when we should abort a movement */
	UPROPERTY(EditAnywhere, Category = "Data")
	FPDMStuckMovementConditions StuckMovementRules{};
};

/** @brief Reference-bound parameter structure related to an entities navpath */
struct FPDMPathParameters
{
	FPDMPathParameters(const FPDMPathParameters& Other)
		: InstanceData(Other.InstanceData)
		, MoveTarget(Other.MoveTarget)
		, TransformFragment(Other.TransformFragment)
		, EntitySubsystem(Other.EntitySubsystem)
		, bIsEntityValid(Other.bIsEntityValid)
		, Target(Other.Target)
		, NavPath(Other.NavPath)
	{
	} ;
	
	FPDMPathParameters(
		FPDMTaskData_MoveToHandle& InputInstanceData,
		FMassMoveTargetFragment& MoveTargetInput,
		const FTransformFragment& InputTransformFragment,
		UMassEntitySubsystem& InputEntitySubsystem,
		bool bInputIsEntityValid,
		FPDTargetCompound& InputTarget,
		const UNavigationPath* InputNavPath = nullptr
	)
		:
			InstanceData(InputInstanceData),
			MoveTarget(MoveTargetInput),
			TransformFragment(InputTransformFragment),
			EntitySubsystem(InputEntitySubsystem),
			bIsEntityValid(bInputIsEntityValid),
			Target(InputTarget),
			NavPath(InputNavPath)
	{};

	/** @brief Copy/assignment operator */
	FPDMPathParameters& operator=(const FPDMPathParameters& Other);

	/** @brief First it tries to resolve the target entity's location via it's transform fragment
	 * if it was not valid then it tries to resolve the target actor,
	 * if the target actor was not valid then it lastly resolves to the static location target (hope to god it's set to something sensible haha)*/
	FVector ResolveLocation() const;

	/** @brief InstanceData for the MoveTo task*/
	FPDMTaskData_MoveToHandle& InstanceData;
	/** @brief Move target fragment */
	FMassMoveTargetFragment& MoveTarget; 
	/** @brief Transform fragment of the entity this task is being run on */
	const FTransformFragment& TransformFragment;
	/** @brief Entity subsystem */
	UMassEntitySubsystem& EntitySubsystem;
	/** @brief Is target entity valid */
	bool bIsEntityValid;
	/** @brief Target Compound */
	FPDTargetCompound& Target;
	/** @brief The navpath for the MoveTo task */
	const UNavigationPath* NavPath = nullptr;
};

/**
 * @brief Task to move to a given target, either another entity or a world actor.
 * @note Works by generating a path for the movement processor to read from
 */
USTRUCT()
struct PDRTSBASE_API FPDMTask_MoveToTarget : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	/* Macro helper to declare the required task functions */
	DECLARE_TASK_BODY(MoveToHandle)

	/** @brief Resolves the navpath at current path index for priority pathing, @bug Navpath generates invalid points, commented out for the moment, will resolve issue within a couple of commits  */
	static void ProcessNewPriorityPath(const FPDMPathParameters& Params);
	/** @brief Resolves the navpath at current path index for shared pathing, @bug Navpath generates invalid points, commented out for the moment, will resolve issue within a couple of commits */
	static void ProcessNewSharedPath(const FPDMPathParameters& Params);
	virtual void OnPathSelected(FPDMFragment_RTSEntityBase& RTSData, bool bShouldUseSharedNavigation, const FVector& LastPoint) const;

	/* Links/handles */
	TStateTreeExternalDataHandle<UMassEntitySubsystem> EntitySubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MoveParametersHandle;
	TStateTreeExternalDataHandle<FPDMFragment_RTSEntityBase> RTSDataHandle;
};


//
// Random wander point

/** @brief Instance data for entity running FPDMTask_RandomWander */
USTRUCT()
struct FPDMTaskData_RandomWander
{
	GENERATED_BODY()

	/** @brief Limit for the random search radius, value can be set from state-tree editor */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float StartRadiusRandomLimit = 100.f;

	/** @brief Max remaining distance to goal until it reads as success, value can be set from state-tree editor */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float SuccessRadius = 100.f;

	/** @brief Settings to control our parameters when we should abort a movement */
	UPROPERTY(EditAnywhere, Category = "Data")
	FPDMStuckMovementConditions StuckMovementRules{};	
};

/**
 * @brief Task to move to a random target, either another entity or a world actor
 * @note Works by generating a random point for the movement processor to read from
 */
USTRUCT()
struct PDRTSBASE_API FPDMTask_RandomWander : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	/* Macro helper to declare the required task functions */
	DECLARE_TASK_BODY(RandomWander)

	/* Links/handles */
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};

//
// Wait a given amount of time
/** @brief Instance data for entity running FPDMTask_Wait */
USTRUCT()
struct PDRTSBASE_API FPDMTaskData_Wait
{
	GENERATED_BODY()
 
	/** @brief THe wait duration */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float Duration = 0.f;

	/** @brief Accumulating passed time */
	float TimePassed = 0.f;
};

/**
 * @brief Task to wait, just ticks the time and succeeds when the accumulated time has exceeded the given duration
 */
USTRUCT()
struct PDRTSBASE_API FPDMTask_Wait : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	/* Macro helper to declare the required task functions */
	DECLARE_TASK_BODY(Wait)

protected:
	/* Links/handles */
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};


//
// Animation Player

/** @brief Instance data for entity running FPDMTask_PlayAnimation */
USTRUCT()
struct PDRTSBASE_API FPDMTaskData_PlayAnimation
{
	GENERATED_BODY()

	/** @brief Delay before the task ends. Default (0 or any negative) will run indefinitely so it requires a transition in the state tree to stop it. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float Duration = 0.f;

	/** @brief Accumulated time used if duration is set */
	UPROPERTY()
	float Time = 0.f;
	
	/** @brief Selected animation index for the A2T (vertex animation) */
	UPROPERTY(EditAnywhere, Category = Parameter)
	EPDVertexAnimSelector AnimationIndex = EPDVertexAnimSelector::VertexIdle;
};
 
/**
 * @brief Task to play animation
 * @note Works by setting a duration and vertex animation index.
 * @note Assumes the vertex animation data has been set in the fragment 'FPDMFragment_EntityAnimation' in the animation processor
 */
USTRUCT()
struct PDRTSBASE_API FPDMTask_PlayAnimation : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	/* Macro helper to declare the required task functions */
	DECLARE_TASK_BODY(PlayAnimation)

protected:
	/* Links/handles */
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FPDMFragment_EntityAnimation> AnimationHandle;
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