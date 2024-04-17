// Fill out your copyright notice in the Description page of Project Settings.


#include "Mass/Tasks/PDAITask_GoTo.h"
#include "MassStateTreeExecutionContext.h"
#include "ZoneGraphTypes.h"
#include "ZoneGraphQuery.h"
#include "ZoneGraphSubsystem.h"
#include "ZoneGraphAnnotationSubsystem.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassCrowdSubsystem.h"
#include "MassCrowdSettings.h"

#include "ZoneGraphAStar.h"

EStateTreeRunStatus UPDAITask_GoTo::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	return Super::Tick(Context, DeltaTime);
}

EStateTreeRunStatus UPDAITask_GoTo::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	return Super::EnterState(Context, Transition);
}

void UPDAITask_GoTo::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::ExitState(Context, Transition);
}
